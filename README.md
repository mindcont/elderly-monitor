# elderly-monitor-daemon

基于 Qt 6 / C++ / CMake 的边缘 AI 老人活动检测网关骨架。

## 定位

无界面边缘服务：从摄像头取流，识别人是否在场，并通过 MQTT 发布事件。Qt Widgets 界面不参与正式运行。

## 构建

```bash
cmake -S . -B build
cmake --build build-daemon --config Release

运行：

```powershell
.\build-daemon\Release\elderly-monitor-daemon.exe --config config\config.json
```

当前版本已经完成后台进程、配置、事件状态机和 MQTT payload 日志适配；GStreamer RTSP 和 NCNN/TFLite 模型适配仍按 [DAEMON_PLAN.md](DAEMON_PLAN.md) 的第二、三阶段接入。
```

当前已落地模块：配置、摄像头接口、检测接口、跟踪接口、事件引擎、MQTT接口、Qt主窗口。RTSP、YOLO11、ByteTrack 和 MQTT 的具体第三方适配器保留为独立实现点。
