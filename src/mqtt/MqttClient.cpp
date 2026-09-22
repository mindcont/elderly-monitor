#include "MqttClient.h"
#include <QJsonDocument>
#include <QJsonObject>
void MqttClient::connectToBroker() { emit statusChanged("MQTT backend pending: add Eclipse Paho adapter"); }
void MqttClient::publish(const DetectionEvent &event) {
    Q_UNUSED(event); emit statusChanged("event queued");
}
