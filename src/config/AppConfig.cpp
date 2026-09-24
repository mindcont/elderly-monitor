#include "AppConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QMap>

static QMap<QString, QString> loadDotEnv(const QString &path)
{
    QMap<QString, QString> values;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return values;
    while (!file.atEnd()) {
        auto line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;
        const auto separator = line.indexOf('=');
        if (separator <= 0) continue;
        auto key = line.left(separator).trimmed();
        auto value = line.mid(separator + 1).trimmed();
        if (value.size() >= 2 && ((value.startsWith('"') && value.endsWith('"')) || (value.startsWith('\'') && value.endsWith('\''))))
            value = value.mid(1, value.size() - 2);
        values.insert(key, value);
    }
    return values;
}

AppConfig AppConfig::load(const QString &path)
{
    AppConfig c;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return c;
    const auto root = QJsonDocument::fromJson(file.readAll()).object();
    const auto camera = root["camera"].toObject();
    const auto mqtt = root["mqtt"].toObject();
    const auto ai = root["ai"].toObject();
    const auto event = root["event"].toObject();
    c.cameraId = camera["id"].toString(c.cameraId);
    c.rtspUrl = camera["rtsp"].toString();
    c.mqttHost = mqtt["host"].toString(c.mqttHost);
    c.mqttPort = mqtt["port"].toInt(c.mqttPort);
    c.mqttTopic = mqtt["topic"].toString(c.mqttTopic);
    c.modelPath = ai["model"].toString(c.modelPath);
    c.confidence = float(ai["confidence"].toDouble(c.confidence));
    c.enterConfirmFrames = event["enter_confirm_frames"].toInt(c.enterConfirmFrames);
    c.leaveTimeoutMs = event["leave_timeout"].toInt(c.leaveTimeoutMs);
    c.inactiveTimeoutSec = event["inactive_timeout"].toInt(c.inactiveTimeoutSec);
    c.detectIntervalMs = ai["detect_interval_ms"].toInt(c.detectIntervalMs);

    const auto envPath = QFileInfo(path).absolutePath() + "/.env";
    const auto env = loadDotEnv(envPath);
    if (env.contains("CAMERA_ID")) c.cameraId = env.value("CAMERA_ID");
    if (env.contains("RTSP_URL")) c.rtspUrl = env.value("RTSP_URL");
    if (env.contains("MQTT_HOST")) c.mqttHost = env.value("MQTT_HOST");
    if (env.contains("MQTT_PORT")) c.mqttPort = env.value("MQTT_PORT").toInt();
    if (env.contains("MQTT_TOPIC")) c.mqttTopic = env.value("MQTT_TOPIC");
    if (env.contains("MODEL_PATH")) c.modelPath = env.value("MODEL_PATH");
    return c;
}
