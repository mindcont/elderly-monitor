#pragma once
#include "EnvConfig.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <atomic>
#include <functional>
class RtspPipeline {
public:
    using FrameCallback = std::function<void(const unsigned char *, int, int, std::size_t)>;
    RtspPipeline(const EnvConfig &c, FrameCallback callback);
    ~RtspPipeline();
    bool start();
    void stop();
    void run();
    unsigned long long framesReceived() const { return frames_.load(); }
private:
    static gboolean bus(GstBus *, GstMessage *, gpointer);
    GstElement *pipeline_=nullptr;
    GstAppSink *sink_=nullptr;
    GMainLoop *loop_=nullptr;
    EnvConfig config_;
    FrameCallback callback_;
    std::atomic<unsigned long long> frames_{0};
};
