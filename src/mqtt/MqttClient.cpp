#include "MqttClient.h"
#include <QJsonDocument>
#include <QJsonObject>
namespace {
QByteArray mqttString(const QString &value) {
    const auto bytes = value.toUtf8(); QByteArray result;
    result.append(char((bytes.size() >> 8) & 0xff)); result.append(char(bytes.size() & 0xff)); result.append(bytes); return result;
}
}
QByteArray MqttClient::encodeRemainingLength(int length) const {
    QByteArray result;
    do { int digit = length % 128; length /= 128; if (length > 0) digit |= 128; result.append(char(digit)); } while (length > 0);
    return result;
}
void MqttClient::connectToBroker() {
    connect(&socket_, &QTcpSocket::connected, this, [this] {
        QByteArray payload = mqttString("elderly-monitor-daemon");
        QByteArray packet; packet.append(char(0x10)); packet.append(encodeRemainingLength(10 + payload.size()));
        packet.append(mqttString("MQTT")); packet.append(char(4)); packet.append(char(2)); packet.append(char(0)); packet.append(char(60)); packet.append(payload);
        socket_.write(packet); emit statusChanged("MQTT connected to " + config_.mqttHost + ":" + QString::number(config_.mqttPort));
    });
    connect(&socket_, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) { emit statusChanged("MQTT connection error: " + socket_.errorString()); });
    socket_.connectToHost(config_.mqttHost, quint16(config_.mqttPort));
}
void MqttClient::publish(const DetectionEvent &event) {
    const QJsonObject json{{"event", event.type}, {"camera", event.cameraId}, {"track_id", event.trackId}, {"confidence", event.confidence}, {"timestamp", event.timestamp.toString(Qt::ISODate)}};
    const auto body = mqttString(config_.mqttTopic) + QJsonDocument(json).toJson(QJsonDocument::Compact);
    if (socket_.state() == QAbstractSocket::ConnectedState) {
        QByteArray packet; packet.append(char(0x30)); packet.append(encodeRemainingLength(body.size())); packet.append(body); socket_.write(packet);
    }
    emit statusChanged(QString("topic=%1 payload=%2").arg(config_.mqttTopic, QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact))));
}
