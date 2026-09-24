import asyncio
import base64
import json
import os
import ssl
from urllib.parse import quote, urlsplit, urlunsplit
from collections import deque
from datetime import datetime, timezone
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
    latest = recent[0] if recent else None
    return {"online": latest is not None, "last_report": latest.get("received_at") if latest else None, "latest": latest, "absence_timeout_sec": ABSENCE_TIMEOUT_SEC}


@app.get("/api/messages", dependencies=[Depends(authorized)])
async def messages(_: Auth, limit: int = Query(100, ge=1, le=100)) -> list:
    if pool:
        rows = await pool.fetch("SELECT topic, payload, received_at FROM mqtt_messages ORDER BY received_at DESC LIMIT $1", limit)
        result = []
        for row in rows:
            item = dict(row)
            try:
                item["payload"] = json.loads(item["payload"])
            except (TypeError, json.JSONDecodeError):
                pass
            result.append(item)
        return result
    return list(recent)[:limit]


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
