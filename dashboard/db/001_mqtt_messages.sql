CREATE TABLE IF NOT EXISTS mqtt_messages (
  id BIGSERIAL PRIMARY KEY,
  topic TEXT NOT NULL,
  payload JSONB NOT NULL,
  received_at TIMESTAMPTZ NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS ix_mqtt_messages_received_at ON mqtt_messages(received_at DESC);
