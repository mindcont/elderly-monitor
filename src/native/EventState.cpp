#include "EventState.h"
#include <iostream>

EventState::EventState(int absenceTimeoutSec, Publish publish)
    : absenceTimeoutSec_(absenceTimeoutSec), publish_(std::move(publish)) {}

void EventState::update(const PersonDetection &detection)
{
    const auto now = std::chrono::steady_clock::now();
    if (detection.personPresent) {
        lastSeen_ = now;
        absencePublished_ = false;
        if (!present_) {
            present_ = true;
            std::cerr << "event=person_entered confidence=" << detection.confidence << '\n';
            publish_("person_entered", detection);
        } else {
            publish_("elderly_present", detection);
        }
        return;
    }

    if (!present_ || absencePublished_ || lastSeen_.time_since_epoch().count() == 0)
        return;
    const auto absentSec = std::chrono::duration_cast<std::chrono::seconds>(now - lastSeen_).count();
    if (absentSec >= absenceTimeoutSec_) {
        present_ = false;
        absencePublished_ = true;
        std::cerr << "event=long_time_absent seconds=" << absentSec << '\n';
        publish_("long_time_absent", detection);
    }
}
