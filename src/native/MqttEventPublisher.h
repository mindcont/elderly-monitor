#pragma once

#include "Detection.h"
#include "EnvConfig.h"
#include <memory>

class MqttEventPublisher final : public EventPublisher {
public:
    explicit MqttEventPublisher(const EnvConfig &config);
    ~MqttEventPublisher() override;
    void publish(const std::string &event, const PersonDetection &detection) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
