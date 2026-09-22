#pragma once
#include "DetectionEvent.h"
#include "detection/Detection.h"
#include <QObject>
class EventEngine : public QObject {
    Q_OBJECT
public: explicit EventEngine(QObject *parent = nullptr) : QObject(parent) {}
public slots: void process(const QList<Detection> &detections, const QDateTime &time);
signals: void eventGenerated(const DetectionEvent &event);
};
