#include "EventEngine.h"
void EventEngine::process(const QList<Detection> &detections, const QDateTime &time) {
    for (const auto &d : detections) emit eventGenerated({"person_detected", "living_room", d.trackId, d.confidence, time});
}
