#pragma once
#include <QDateTime>
struct DetectionEvent { QString type; QString cameraId; int trackId = -1; float confidence = 0; QDateTime timestamp; };
