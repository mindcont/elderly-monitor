#include "RtspCamera.h"
#include <QTimer>
void RtspCamera::start() { emit error(config_.rtspUrl.isEmpty() ? "RTSP URL is empty; configure camera.rtsp" : "RTSP adapter pending: connect GStreamer appsink here"); }
void RtspCamera::stop() {}
