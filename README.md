# ElderlyMonitor

基于 Qt 6 / C++ / CMake 的边缘 AI 老人活动检测网关骨架。

## 构建

```bash
cmake -S . -B build
cmake --build build --config Release
```

当前已落地模块：配置、摄像头接口、检测接口、跟踪接口、事件引擎、MQTT接口、Qt主窗口。RTSP、YOLO11、ByteTrack 和 MQTT 的具体第三方适配器保留为独立实现点。
