#include "Application.h"
#include "ui/MainWindow.h"
#include "mqtt/MqttClient.h"
Application::Application(int &argc, char **argv) : QApplication(argc, argv) {}
int Application::run() { MainWindow w; MqttClient mqtt; QObject::connect(&mqtt, &MqttClient::statusChanged, &w, &MainWindow::setMqttStatus); w.show(); mqtt.connectToBroker(); return exec(); }
