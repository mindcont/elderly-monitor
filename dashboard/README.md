# Elderly Monitor Dashboard

FastAPI + Vue 3/Vite dashboard for MQTT event history and live monitoring. The dashboard includes a monthly presence calendar, daily in-home summaries, a 30-minute timeline, and an event-based presence trend chart.

## Run

Copy `.env.example` to `.env`, fill the PostgreSQL/MQTT values, then run:

```bash
docker compose up --build
```

The API is available at `http://localhost:8000`, and the web UI at `http://localhost:5173`.
The history and calendar endpoints use `LOCAL_TIMEZONE` (default `Asia/Shanghai`) to group MQTT reports into local days.

## Production

Put the API and frontend behind the existing Caddy instance at `elder.wio.me`. The API exposes `/api/health`, `/api/status`, `/api/messages`, `/api/history`, `/api/calendar`, and `/api/events` (SSE). Basic Auth is enforced when `BASIC_AUTH_USER` and `BASIC_AUTH_PASSWORD` are set.
