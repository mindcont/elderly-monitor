#include "RtspPipeline.h"
#include <iostream>
#include <thread>
#include <chrono>
RtspPipeline::RtspPipeline(const EnvConfig &c, FrameCallback callback):config_(c),callback_(std::move(callback)){}
RtspPipeline::~RtspPipeline(){stop();if(sink_){gst_object_unref(sink_);sink_=nullptr;}if(pipeline_){gst_object_unref(pipeline_);pipeline_=nullptr;}if(loop_){g_main_loop_unref(loop_);loop_=nullptr;}}
gboolean RtspPipeline::bus(GstBus*,GstMessage*m,gpointer data){auto*s=static_cast<RtspPipeline*>(data); if(GST_MESSAGE_TYPE(m)==GST_MESSAGE_ERROR){GError*e=nullptr;gchar*d=nullptr;gst_message_parse_error(m,&e,&d);std::cerr<<"GStreamer error: "<<(e?e->message:"unknown")<<std::endl;if(d){std::cerr<<"GStreamer debug: "<<d<<std::endl;g_free(d);}if(e)g_error_free(e);s->failed_=true;s->stopRequested_=true;if(s->loop_)g_main_loop_quit(s->loop_);}else if(GST_MESSAGE_TYPE(m)==GST_MESSAGE_EOS){s->stopRequested_=true;if(s->loop_)g_main_loop_quit(s->loop_);}return G_SOURCE_CONTINUE;}
bool RtspPipeline::start(){if(config_.rtspUrl.empty()){std::cerr<<"RTSP_URL is empty"<<std::endl;return false;} stopRequested_=false;finished_=false;failed_=false; const auto d="rtspsrc name=src location=\""+config_.rtspUrl+"\" protocols=tcp latency="+std::to_string(config_.latencyMs)+" timeout=5000000 tcp-timeout=5000000 buffer-mode=none drop-on-latency=true src. ! application/x-rtp,media=video,encoding-name=H264 ! rtph264depay ! h264parse ! avdec_h264 ! videoscale ! videoconvert ! video/x-raw,format=BGR,width=320,height=240 ! appsink name=sink sync=false max-buffers=1 drop=true"; GError*e=nullptr;pipeline_=gst_parse_launch(d.c_str(),&e);if(!pipeline_){std::cerr<<"Cannot create GStreamer pipeline: "<<(e?e->message:"unknown")<<std::endl;if(e)g_error_free(e);return false;}sink_=GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipeline_),"sink"));if(!sink_){std::cerr<<"Cannot find appsink"<<std::endl;return false;}auto*b=gst_element_get_bus(pipeline_);gst_bus_add_watch(b,&RtspPipeline::bus,this);gst_object_unref(b);loop_=g_main_loop_new(nullptr,false);const auto result=gst_element_set_state(pipeline_,GST_STATE_PLAYING);std::cerr<<"GStreamer state request="<<gst_element_state_change_return_get_name(result)<<std::endl;return true;}
void RtspPipeline::run(){
    auto nextInference = std::chrono::steady_clock::now();
    while(pipeline_&&sink_&&!stopRequested_){
        while(g_main_context_pending(nullptr))g_main_context_iteration(nullptr,false);
        if(auto*sample=gst_app_sink_try_pull_sample(sink_,100*GST_MSECOND)){
            ++frames_;
            const auto now = std::chrono::steady_clock::now();
            if (callback_ && now >= nextInference) {
                auto *buffer = gst_sample_get_buffer(sample);
                GstMapInfo map{};
                if (buffer && gst_buffer_map(buffer, &map, GST_MAP_READ)) {
                    callback_(map.data, 320, 240, map.size / 240U);
                    gst_buffer_unmap(buffer, &map);
                }
                nextInference = now + std::chrono::milliseconds(config_.inferenceIntervalMs);
            }
            gst_sample_unref(sample);
        }
    }
    finished_=true;
    if(loop_)g_main_loop_quit(loop_);
}
void RtspPipeline::stop(){stopRequested_=true;if(pipeline_)gst_element_set_state(pipeline_,GST_STATE_NULL);if(loop_)g_main_loop_quit(loop_);}
