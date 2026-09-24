#include "Application.h"
#include "config/AppConfig.h"
#include "camera/RtspCamera.h"
#include "detection/YoloDetector.h"
#include "event/EventEngine.h"
#include "mqtt/MqttClient.h"
#include <QCommandLineParser>
#include <QLoggingCategory>
Application::Application(int &argc, char **argv) : QCoreApplication(argc, argv) {}
int Application::run() {
    QCommandLineParser parser; parser.addHelpOption();
    QCommandLineOption option({"c", "config"}, "JSON config path", "path", "config/config.json");
    parser.addOption(option); parser.process(*this);
    const auto config = AppConfig::load(parser.value(option));
    qInfo() << "elderly-monitor-daemon starting, camera=" << config.cameraId;
    RtspCamera camera(config); YoloDetector detector(config); EventEngine events(config); MqttClient mqtt(config);
    connect(&camera, &RtspCamera::frameReady, &detector, &YoloDetector::process);
    connect(&detector, &YoloDetector::detectionsReady, &events,
            [&events](const QList<Detection> &detections, const Frame &frame) {
                events.process(detections, frame.timestamp);
            });
    connect(&events, &EventEngine::eventGenerated, &mqtt, &MqttClient::publish);
    connect(&camera, &RtspCamera::error, [](const QString &m) { qWarning() << m; });
    connect(&mqtt, &MqttClient::statusChanged, [](const QString &m) { qInfo() << m; });
    mqtt.connectToBroker(); camera.start(); return exec();
}
