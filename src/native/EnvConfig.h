#pragma once
#include <string>
struct EnvConfig {
    std::string rtspUrl;
    std::string cameraId = "living_room";
    std::string modelParam = "models/yolo11n.ncnn.param";
    std::string modelBin = "models/yolo11n.ncnn.bin";
    std::string mqttHost;
    std::string mqttUser;
    std::string mqttPassword;
    std::string mqttTopic = "home/ai/living_room/event";
    std::string mqttCaFile = "/etc/ssl/certs/ca-certificates.crt";
    int latencyMs = 200;
    int reportIntervalSec = 10;
    int inferenceIntervalMs = 1000;
    int modelInputSize = 320;
    int absenceTimeoutSec = 300;
    int mqttPort = 8883;
    static EnvConfig load(const std::string &path);
};
