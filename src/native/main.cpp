#include "EnvConfig.h"
#include "RtspPipeline.h"
#include <gst/gst.h>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
namespace { volatile std::sig_atomic_t running=1; void stop(int){running=0;} }
int main(int argc,char**argv){const std::string env=argc>1?argv[1]:".env";gst_init(&argc,&argv);std::signal(SIGINT,stop);std::signal(SIGTERM,stop);const auto c=EnvConfig::load(env);std::cout<<"elderly-monitor-daemon camera="<<c.cameraId<<std::endl;RtspPipeline p(c);if(!p.start())return 2;std::thread t([&p]{p.run();});unsigned long long previous=0;while(running){std::this_thread::sleep_for(std::chrono::seconds(c.reportIntervalSec));auto current=p.framesReceived();std::cout<<"frames="<<current<<" fps="<<(current-previous)/static_cast<double>(c.reportIntervalSec)<<std::endl;previous=current;}p.stop();t.join();return 0;}
