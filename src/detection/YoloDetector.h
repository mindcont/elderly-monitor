#pragma once
#include "Detection.h"
#include "camera/Frame.h"
#include <QObject>
class YoloDetector : public QObject {
    Q_OBJECT
public: explicit YoloDetector(QObject *parent = nullptr) : QObject(parent) {}
public slots: void process(const Frame &frame);
signals: void detectionsReady(const QList<Detection> &detections, const Frame &frame);
};
