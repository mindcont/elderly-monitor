#pragma once
#include "EnvConfig.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <atomic>
class RtspPipeline { public: explicit RtspPipeline(const EnvConfig &c); ~RtspPipeline(); bool start(); void stop(); void run(); unsigned long long framesReceived() const { return frames_.load(); } private: static GstFlowReturn sample(GstAppSink *, gpointer); static gboolean bus(GstBus *, GstMessage *, gpointer); GstElement *pipeline_=nullptr; GMainLoop *loop_=nullptr; EnvConfig config_; std::atomic<unsigned long long> frames_{0}; };
