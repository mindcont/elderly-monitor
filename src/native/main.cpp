#include "EnvConfig.h"
#include "RtspPipeline.h"
#include "NcnnPersonDetector.h"
#include "EventState.h"
#include "MqttEventPublisher.h"
#include <gst/gst.h>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include <memory>
#include <mutex>
namespace { volatile std::sig_atomic_t running=1; void stop(int){running=0;} }
int main(int argc,char**argv){
    const std::string env=argc>1?argv[1]:".env";
    gst_init(&argc,&argv); std::signal(SIGINT,stop); std::signal(SIGTERM,stop);
    const auto c=EnvConfig::load(env);
    std::cout<<"elderly-monitor-daemon camera="<<c.cameraId<<std::endl;
    try {
        NcnnPersonDetector detector(c.modelParam, c.modelBin, c.modelInputSize);
        std::unique_ptr<MqttEventPublisher> mqtt;
        if (!c.mqttHost.empty()) mqtt = std::make_unique<MqttEventPublisher>(c);
        std::mutex detectionMutex;
        PersonDetection latestDetection;
        bool hasDetection = false;
        EventState events(c.absenceTimeoutSec, [&](const std::string &name, const PersonDetection &d) {
            if (mqtt) mqtt->publish(name, d);
        });
        while (running) {
            RtspPipeline p(c, [&](const unsigned char *pixels, int width, int height, std::size_t stride) {
                const auto detection = detector.detectBgr(pixels, width, height, stride);
                {
                    std::lock_guard<std::mutex> lock(detectionMutex);
                    latestDetection = detection;
                    hasDetection = true;
                }
                std::cerr << "inference person=" << (detection.personPresent ? "true" : "false")
                          << " confidence=" << detection.confidence << '\n';
                events.update(detection);
            });
            if(!p.start()) return 2;
            std::thread t([&p]{p.run();});
            unsigned long long previous=0;
            int elapsed=0;
            while(running && !p.finished()){
                std::this_thread::sleep_for(std::chrono::seconds(1));
                ++elapsed;
                if (elapsed >= c.reportIntervalSec) {
                    auto current=p.framesReceived();
                    std::cout<<"frames="<<current<<" fps="<<(current-previous)/static_cast<double>(elapsed)<<std::endl;
                    if (mqtt) {
                        std::lock_guard<std::mutex> lock(detectionMutex);
                        if (hasDetection)
                            mqtt->publish("regular_report", latestDetection);
                    }
                    previous=current;
                    elapsed=0;
                }
            }
            p.stop();
            t.join();
            if (running && p.failed()) {
                std::cerr << "RTSP stopped; reconnecting in 5 seconds\n";
                for (int i=0; i<5 && running; ++i)
                    std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "fatal: " << e.what() << '\n';
        return 3;
    }
    return 0;
}
