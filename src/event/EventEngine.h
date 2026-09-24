#pragma once
#include "DetectionEvent.h"
#include "detection/Detection.h"
#include <QObject>
#include "../config/AppConfig.h"
class EventEngine : public QObject {
    Q_OBJECT
public: explicit EventEngine(const AppConfig &config, QObject *parent = nullptr) : QObject(parent), config_(config) {}
public slots: void process(const QList<Detection> &detections, const QDateTime &time);
signals: void eventGenerated(const DetectionEvent &event);
private: AppConfig config_; bool present_ = false; bool inactiveReported_ = false; QDateTime lastSeen_; int consecutivePresent_ = 0;
};
