#pragma once
#include <string>
struct EnvConfig { std::string rtspUrl; std::string cameraId = "living_room"; int latencyMs = 200; int reportIntervalSec = 10; static EnvConfig load(const std::string &path); };
