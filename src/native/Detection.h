#pragma once

#include <cstdint>
#include <string>

struct FrameInfo {
    std::uint64_t sequence = 0;
    int width = 320;
    int height = 240;
};

struct PersonDetection {
    bool personPresent = false;
    float confidence = 0.0F;
};

class PersonDetector {
public:
    virtual ~PersonDetector() = default;
    virtual PersonDetection detect(const FrameInfo &frame) = 0;
};

class EventPublisher {
public:
    virtual ~EventPublisher() = default;
    virtual void publish(const std::string &event, const PersonDetection &detection) = 0;
};

class MqttEventPublisher : public EventPublisher {
public:
    ~MqttEventPublisher() override = default;
};
