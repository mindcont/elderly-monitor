#pragma once
#include "Detection.h"
#include "camera/Frame.h"
#include <QObject>
#include "../config/AppConfig.h"
class YoloDetector : public QObject {
    Q_OBJECT
public: explicit YoloDetector(const AppConfig &config, QObject *parent = nullptr) : QObject(parent), config_(config) {}
public slots: void process(const Frame &frame);
signals: void detectionsReady(const QList<Detection> &detections, const Frame &frame);
private: AppConfig config_;
};
