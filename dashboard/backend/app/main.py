import asyncio
import base64
import json
import os
import ssl
from urllib.parse import quote, urlsplit, urlunsplit
from collections import deque
from datetime import datetime, timezone, date, time, timedelta
from zoneinfo import ZoneInfo
from typing import Annotated

import asyncpg
from aiomqtt import Client
from fastapi import Depends, FastAPI, HTTPException, Query, Request, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import StreamingResponse

POSTGRES_DSN = os.getenv("POSTGRES_DSN", "")
POSTGRES_PASSWORD_FILE = os.getenv("POSTGRES_PASSWORD_FILE", "")
MQTT_HOST = os.getenv("MQTT_HOST", "mqtt.wio.me")
MQTT_PORT = int(os.getenv("MQTT_PORT", "8883"))
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "home/ai/living_room/event")
MQTT_USER = os.getenv("MQTT_USER", "")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "")
ABSENCE_TIMEOUT_SEC = int(os.getenv("ABSENCE_TIMEOUT_SEC", "300"))
LOCAL_TZ = ZoneInfo(os.getenv("LOCAL_TIMEZONE", "Asia/Shanghai"))
BASIC_AUTH_USER = os.getenv("BASIC_AUTH_USER", "")
BASIC_AUTH_PASSWORD = os.getenv("BASIC_AUTH_PASSWORD", "")

app = FastAPI(title="Elderly Monitor API")
app.add_middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"])
pool: asyncpg.Pool | None = None
recent = deque(maxlen=100)
subscribers: set[asyncio.Queue] = set()


def authorized(request: Request) -> None:
    if not BASIC_AUTH_USER:
        return
    value = request.headers.get("authorization", "")
    expected = "Basic " + base64.b64encode(f"{BASIC_AUTH_USER}:{BASIC_AUTH_PASSWORD}".encode()).decode()
    if value != expected:
        raise HTTPException(401, "Authentication required", headers={"WWW-Authenticate": "Basic"})


Auth = Annotated[None, Depends(authorized)]


@app.on_event("startup")
async def startup() -> None:
    global pool
    if POSTGRES_DSN:
        dsn = POSTGRES_DSN
        if POSTGRES_PASSWORD_FILE and "@" in dsn and "://" in dsn:
            password = open(POSTGRES_PASSWORD_FILE, encoding="utf-8").read().strip()
            parts = urlsplit(dsn)
            if ":" not in parts.netloc.rsplit("@", 1)[0]:
                user = parts.netloc.rsplit("@", 1)[0]
                host = parts.netloc.rsplit("@", 1)[1]
                dsn = urlunsplit((parts.scheme, f"{user}:{quote(password, safe='')}@{host}", parts.path, parts.query, parts.fragment))
        pool = await asyncpg.create_pool(dsn)
    asyncio.create_task(mqtt_loop())


@app.on_event("shutdown")
async def shutdown() -> None:
    if pool:
        await pool.close()


async def mqtt_loop() -> None:
    tls = ssl.create_default_context()
    while True:
        try:
            async with Client(MQTT_HOST, port=MQTT_PORT, username=MQTT_USER, password=MQTT_PASSWORD, tls_context=tls) as client:
                await client.subscribe(MQTT_TOPIC)
                async for message in client.messages:
                    payload = json.loads(message.payload.decode())
                    item = {"received_at": datetime.now(timezone.utc).isoformat(), "topic": str(message.topic), **payload}
                    recent.appendleft(item)
                    if pool:
                        await pool.execute("INSERT INTO mqtt_messages(topic, payload, received_at) VALUES($1, $2, now())", str(message.topic), json.dumps(payload, ensure_ascii=False))
                    for queue in list(subscribers):
                        await queue.put(item)
        except Exception as exc:
            print(f"MQTT subscriber reconnect: {exc}", flush=True)
            await asyncio.sleep(5)


@app.get("/api/health")
async def health() -> dict:
    return {"status": "ok", "mqtt": MQTT_HOST, "database": pool is not None}


@app.get("/api/status", dependencies=[Depends(authorized)])
async def status(_: Auth) -> dict:
    latest = None
    if pool:
        row = await pool.fetchrow("SELECT topic, payload, received_at FROM mqtt_messages ORDER BY received_at DESC LIMIT 1")
        if row:
            latest = dict(row)
            try:
                payload = json.loads(latest["payload"])
                latest = {"received_at": latest["received_at"].isoformat(), "topic": latest["topic"], **payload}
            except (TypeError, json.JSONDecodeError):
                latest["received_at"] = latest["received_at"].isoformat()
    if latest is None and recent:
        latest = recent[0]
    last_report = latest.get("received_at") if latest else None
    online = False
    if last_report:
        timestamp = datetime.fromisoformat(last_report.replace("Z", "+00:00"))
        online = (datetime.now(timezone.utc) - timestamp).total_seconds() <= ABSENCE_TIMEOUT_SEC
    return {"online": online, "last_report": last_report, "latest": latest, "absence_timeout_sec": ABSENCE_TIMEOUT_SEC}


@app.get("/api/messages", dependencies=[Depends(authorized)])
async def messages(_: Auth, limit: int = Query(100, ge=1, le=100)) -> list:
    if pool:
        rows = await pool.fetch("""
            SELECT topic, payload, received_at
            FROM mqtt_messages
            WHERE COALESCE(payload::jsonb->>'event', '') <> 'regular_report'
            ORDER BY received_at DESC LIMIT $1
        """, limit)
        result = []
        for row in rows:
            item = dict(row)
            try:
                item["payload"] = json.loads(item["payload"])
            except (TypeError, json.JSONDecodeError):
                pass
            result.append(item)
        return result
    return [item for item in recent if item.get("event") != "regular_report"][:limit]


@app.get("/api/history", dependencies=[Depends(authorized)])
async def history(_: Auth, day: date = Query(default=None)) -> dict:
    """Return meaningful events and merged presence intervals for a local day."""
    selected = day or datetime.now(LOCAL_TZ).date()
    start_local = datetime.combine(selected, time.min, LOCAL_TZ)
    end_local = start_local + timedelta(days=1)
    start_utc = start_local.astimezone(timezone.utc)
    end_utc = end_local.astimezone(timezone.utc)
    if not pool:
        return {"date": selected.isoformat(), "events": [], "intervals": [], "present_seconds": 0}

    rows = await pool.fetch("""
        SELECT payload, received_at
        FROM mqtt_messages
        WHERE received_at >= $1 AND received_at < $2
        ORDER BY received_at ASC
    """, start_utc, end_utc)
    events = []
    points = []
    for row in rows:
        try:
            payload = json.loads(row["payload"])
        except (TypeError, json.JSONDecodeError):
            continue
        stamp = row["received_at"].astimezone(LOCAL_TZ)
        present = bool(payload.get("person_present"))
        points.append((stamp, present))
        if payload.get("event") != "regular_report":
            events.append({"received_at": stamp.isoformat(), **payload})

    intervals = []
    current = None
    max_gap = timedelta(seconds=ABSENCE_TIMEOUT_SEC + 60)
    for stamp, present in points:
        if present:
            if current is None:
                current = {"start": stamp, "end": stamp}
            elif stamp - current["end"] <= max_gap:
                current["end"] = stamp
            else:
                intervals.append(current)
                current = {"start": stamp, "end": stamp}
        elif current is not None and stamp - current["end"] > max_gap:
            intervals.append(current)
            current = None
    if current is not None:
        intervals.append(current)

    result_intervals = []
    present_seconds = 0
    for item in intervals:
        end = item["end"]
        if end >= start_local and end < end_local:
            end = min(end + timedelta(seconds=ABSENCE_TIMEOUT_SEC), end_local)
        seconds = max(0, int((end - item["start"]).total_seconds()))
        present_seconds += seconds
        result_intervals.append({"start": item["start"].isoformat(), "end": end.isoformat(), "seconds": seconds})
    return {"date": selected.isoformat(), "events": list(reversed(events)), "intervals": result_intervals, "present_seconds": present_seconds}


@app.get("/api/calendar", dependencies=[Depends(authorized)])
async def calendar(_: Auth, month: str = Query(..., pattern=r"^\d{4}-\d{2}$")) -> dict:
    """Return daily in-home totals for the month shown by the calendar."""
    year, month_number = (int(value) for value in month.split("-"))
    first = date(year, month_number, 1)
    next_month = date(year + (month_number == 12), 1 if month_number == 12 else month_number + 1, 1)
    start = datetime.combine(first, time.min, LOCAL_TZ).astimezone(timezone.utc)
    end = datetime.combine(next_month, time.min, LOCAL_TZ).astimezone(timezone.utc)
    totals = {}
    if pool:
        rows = await pool.fetch("""
            SELECT received_at, payload
            FROM mqtt_messages
            WHERE received_at >= $1 AND received_at < $2
            ORDER BY received_at ASC
        """, start, end)
        points = []
        for row in rows:
            try:
                payload = json.loads(row["payload"])
            except (TypeError, json.JSONDecodeError):
                continue
            points.append((row["received_at"].astimezone(LOCAL_TZ), bool(payload.get("person_present"))))
        open_start = None
        last_present = None
        for stamp, present in points:
            if present:
                if open_start is None:
                    open_start = stamp
                last_present = stamp
            elif open_start is not None and last_present and stamp - last_present > timedelta(seconds=ABSENCE_TIMEOUT_SEC + 60):
                totals[open_start.date().isoformat()] = totals.get(open_start.date().isoformat(), 0) + int((min(last_present + timedelta(seconds=ABSENCE_TIMEOUT_SEC), stamp) - open_start).total_seconds())
                open_start = None
                last_present = None
        if open_start is not None and last_present:
            totals[open_start.date().isoformat()] = totals.get(open_start.date().isoformat(), 0) + int((min(last_present + timedelta(seconds=ABSENCE_TIMEOUT_SEC), end.astimezone(LOCAL_TZ)) - open_start).total_seconds())
    return {"month": month, "days": totals}


@app.get("/api/events", dependencies=[Depends(authorized)])
async def events(_: Auth):
    async def stream():
        queue: asyncio.Queue = asyncio.Queue()
        subscribers.add(queue)
        try:
            while True:
                item = await queue.get()
                yield f"data: {json.dumps(item)}\n\n"
        finally:
            subscribers.discard(queue)
    return StreamingResponse(stream(), media_type="text/event-stream")


@app.websocket("/ws")
async def websocket(websocket: WebSocket):
    await websocket.accept()
    queue: asyncio.Queue = asyncio.Queue()
    subscribers.add(queue)
    try:
        while True:
            await websocket.send_json(await queue.get())
    except WebSocketDisconnect:
        subscribers.discard(queue)
