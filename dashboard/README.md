# Elderly Monitor Dashboard

FastAPI + Vue 3/Vite dashboard for MQTT event history and live monitoring.

## Run

Copy `.env.example` to `.env`, fill the PostgreSQL/MQTT values, then run:

```bash
docker compose up --build
```

The API is available at `http://localhost:8000`, and the web UI at `http://localhost:5173`.

## Production

Put the API and frontend behind the existing Caddy instance at `elder.wio.me`. The API exposes `/api/health`, `/api/status`, `/api/messages`, and `/api/events` (SSE). Basic Auth is enforced when `BASIC_AUTH_USER` and `BASIC_AUTH_PASSWORD` are set.
