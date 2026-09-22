cmake_minimum_required(VERSION 3.21)
project(ElderlyMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(elderly_monitor
    src/main.cpp
    src/app/Application.cpp src/app/Application.h
    src/config/AppConfig.cpp src/config/AppConfig.h
    src/camera/RtspCamera.cpp src/camera/RtspCamera.h src/camera/Frame.h
    src/detection/YoloDetector.cpp src/detection/YoloDetector.h src/detection/Detection.h
    src/tracking/ByteTracker.cpp src/tracking/ByteTracker.h
    src/event/EventEngine.cpp src/event/EventEngine.h src/event/DetectionEvent.h
    src/mqtt/MqttClient.cpp src/mqtt/MqttClient.h
    src/ui/MainWindow.cpp src/ui/MainWindow.h
    src/ui/MainWindow.ui
)
target_include_directories(elderly_monitor PRIVATE src)
target_link_libraries(elderly_monitor PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

install(TARGETS elderly_monitor RUNTIME DESTINATION bin)
