#pragma once
#include <QString>

struct AppConfig {
    QString cameraId = "living_room";
    QString rtspUrl;
    QString mqttHost = "127.0.0.1";
    int mqttPort = 1883;
    QString mqttTopic = "home/ai/living_room/event";
    QString modelPath = "models/yolo11n.onnx";
    float confidence = 0.5f;
    int enterConfirmFrames = 10;
    int leaveTimeoutMs = 5000;
    int inactiveTimeoutSec = 1800;
    int detectIntervalMs = 1000;
    static AppConfig load(const QString &path);
};
