#pragma once
#include <QList>
#include "detection/Detection.h"
class ByteTracker { public: QList<Detection> update(const QList<Detection> &detections) { return detections; } };
