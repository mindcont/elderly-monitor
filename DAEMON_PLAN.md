# elderly-monitor-daemon 技术路线

## 目标

树莓派 3B 上运行无界面后台服务，完成单路摄像头的“有人进入、仍有人、有人离开、长时间无人”事件判断，并通过 MQTT 交给 Home Assistant。视频画面和 Qt Widgets 不属于正式运行时依赖。

## 第一阶段（当前已落地）

- `QCoreApplication` 后台进程，支持 `-c/--config` 指定 JSON 配置。
- `RtspCamera -> YoloDetector -> EventEngine -> MqttClient` 信号链。
- RTSP、MQTT 和模型路径支持同目录 `.env` 覆盖；真实 `.env` 不进入 Git，使用 `.env.example` 作为模板。
- 事件状态机：连续检测确认 `person_entered`，持续检测 `person_present`，超时产生 `person_left`，离开后达到 `inactive_timeout` 产生 `room_inactive`。
- MQTT 当前为日志适配器，先输出 topic 和 JSON payload，避免在没有 Paho 依赖时假装已经联网。
- RTSP 和人体模型保留独立适配边界，当前启动时会明确报告尚未接入。

## 第二阶段：树莓派取流

使用 GStreamer appsink：

```text
rtspsrc -> rtph264depay -> h264parse -> v4l2h264dec -> videoconvert -> videoscale -> appsink
```

建议使用摄像头子码流，传输 640x360/10fps；推理输入再缩放到 320 或 416。队列采用“只保留最新帧”，禁止积压导致延迟不断增长。

## 第三阶段：轻量人体识别

优先级为 NCNN 或 TFLite 量化模型，其次才是 ARM64 ONNX Runtime。只保留 `person` 类别，每 500～1000ms 推理一次；非推理帧由跟踪/状态机维持。Pi 3B 的目标是事件级响应，不是 25fps 框绘制。

## 第四阶段：MQTT 和运行保障

- 使用 Eclipse Paho MQTT C/C++ 或轻量 MQTT 客户端实现自动重连、QoS 1、保留心跳。
- 事件 topic：`home/ai/{camera_id}/event`。
- 增加 systemd 服务、日志轮转、RTSP 断线重连、模型加载失败告警和进程健康状态。
- MQTT payload 至少包含 `event`、`camera`、`timestamp`、`confidence` 和 `track_id`。

## 验收指标

- 无摄像头时进程可启动并明确报告配置/后端状态。
- RTSP 断线后自动恢复，不阻塞事件线程。
- 单人进入在 3 次连续检测内确认。
- 无人超过 `leave_timeout` 产生一次 `person_left`，超过 `inactive_timeout` 产生一次 `room_inactive`。
- Pi 3B 上内存稳定、检测队列不积压，事件延迟目标小于 3 秒。
