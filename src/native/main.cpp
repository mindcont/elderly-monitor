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
        EventState events(c.absenceTimeoutSec, [&](const std::string &name, const PersonDetection &d) {
            if (mqtt) mqtt->publish(name, d);
        });
        RtspPipeline p(c, [&](const unsigned char *pixels, int width, int height, std::size_t stride) {
            const auto detection = detector.detectBgr(pixels, width, height, stride);
            std::cerr << "inference person=" << (detection.personPresent ? "true" : "false")
                      << " confidence=" << detection.confidence << '\n';
            events.update(detection);
        });
        if(!p.start()) return 2;
        std::thread t([&p]{p.run();});
        unsigned long long previous=0;
        while(running){
            std::this_thread::sleep_for(std::chrono::seconds(c.reportIntervalSec));
            auto current=p.framesReceived();
            std::cout<<"frames="<<current<<" fps="<<(current-previous)/static_cast<double>(c.reportIntervalSec)<<std::endl;
            previous=current;
        }
        p.stop(); t.join();
    } catch (const std::exception &e) {
        std::cerr << "fatal: " << e.what() << '\n';
        return 3;
    }
    return 0;
}
