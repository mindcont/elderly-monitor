cmake_minimum_required(VERSION 3.13)
project(elderly_monitor LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.14 gstreamer-app-1.0>=1.14)
add_executable(elderly-monitor-daemon
    src/native/main.cpp src/native/EnvConfig.cpp src/native/EnvConfig.h
    src/native/RtspPipeline.cpp src/native/RtspPipeline.h)
target_link_libraries(elderly-monitor-daemon PRIVATE PkgConfig::GSTREAMER Threads::Threads)
target_compile_options(elderly-monitor-daemon PRIVATE -Wall -Wextra -Wpedantic)
install(TARGETS elderly-monitor-daemon RUNTIME DESTINATION bin)
