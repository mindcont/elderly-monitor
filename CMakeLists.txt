cmake_minimum_required(VERSION 3.13)
project(elderly_monitor LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.14 gstreamer-app-1.0>=1.14)
find_package(ncnn CONFIG REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED IMPORTED_TARGET libmosquitto)
add_executable(elderly-monitor-daemon
    src/native/main.cpp src/native/EnvConfig.cpp src/native/EnvConfig.h src/native/Detection.h
    src/native/RtspPipeline.cpp src/native/RtspPipeline.h
    src/native/NcnnPersonDetector.cpp src/native/NcnnPersonDetector.h
    src/native/EventState.cpp src/native/EventState.h
    src/native/MqttEventPublisher.cpp src/native/MqttEventPublisher.h)
target_link_libraries(elderly-monitor-daemon PRIVATE PkgConfig::GSTREAMER ncnn PkgConfig::MOSQUITTO Threads::Threads atomic)
target_compile_options(elderly-monitor-daemon PRIVATE -Wall -Wextra -Wpedantic)
install(TARGETS elderly-monitor-daemon RUNTIME DESTINATION bin)
