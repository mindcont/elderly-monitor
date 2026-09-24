#include "EventEngine.h"
void EventEngine::process(const QList<Detection> &detections, const QDateTime &time) {
    if (!detections.isEmpty()) {
        ++consecutivePresent_; lastSeen_ = time; inactiveReported_ = false;
        if (!present_ && consecutivePresent_ >= config_.enterConfirmFrames) { present_ = true; emit eventGenerated({"person_entered", config_.cameraId, detections.first().trackId, detections.first().confidence, time}); }
        if (present_) emit eventGenerated({"person_present", config_.cameraId, detections.first().trackId, detections.first().confidence, time});
    } else {
        consecutivePresent_ = 0;
        if (present_ && lastSeen_.msecsTo(time) >= config_.leaveTimeoutMs) { present_ = false; emit eventGenerated({"person_left", config_.cameraId, -1, 0, time}); }
        if (!present_ && !inactiveReported_ && lastSeen_.isValid() && lastSeen_.msecsTo(time) >= config_.inactiveTimeoutSec * 1000) { inactiveReported_ = true; emit eventGenerated({"room_inactive", config_.cameraId, -1, 0, time}); }
    }
}
