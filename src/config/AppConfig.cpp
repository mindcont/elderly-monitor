#include "AppConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

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
    return c;
}
