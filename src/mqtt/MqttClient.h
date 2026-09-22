#pragma once
#include "event/DetectionEvent.h"
#include <QObject>
class MqttClient : public QObject {
    Q_OBJECT
public: explicit MqttClient(QObject *parent = nullptr) : QObject(parent) {}
public slots: void connectToBroker(); void publish(const DetectionEvent &event);
signals: void statusChanged(const QString &status);
};
