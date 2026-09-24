#pragma once

#include "Detection.h"
#include <chrono>
#include <functional>
#include <string>

class EventState {
public:
    using Publish = std::function<void(const std::string &, const PersonDetection &)>;
    EventState(int absenceTimeoutSec, Publish publish);
    void update(const PersonDetection &detection);

private:
    int absenceTimeoutSec_;
    Publish publish_;
    bool present_ = false;
    std::chrono::steady_clock::time_point lastSeen_{};
    bool absencePublished_ = false;
};
