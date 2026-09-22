#pragma once
#include <QRectF>
struct Detection { int classId = 0; float confidence = 0; QRectF bbox; int trackId = -1; };
