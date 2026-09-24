#pragma once
#include "event/DetectionEvent.h"
#include <QObject>
#include "../config/AppConfig.h"
#include <QTcpSocket>
class MqttClient : public QObject {
    Q_OBJECT
public: explicit MqttClient(const AppConfig &config, QObject *parent = nullptr) : QObject(parent), config_(config) {}
public slots: void connectToBroker(); void publish(const DetectionEvent &event);
signals: void statusChanged(const QString &status);
private: AppConfig config_;
    QTcpSocket socket_;
    QByteArray encodeRemainingLength(int length) const;
};
