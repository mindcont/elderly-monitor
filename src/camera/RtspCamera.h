#pragma once
#include "Frame.h"
#include <QObject>
#include "../config/AppConfig.h"
class RtspCamera : public QObject {
    Q_OBJECT
public: explicit RtspCamera(const AppConfig &config, QObject *parent = nullptr) : QObject(parent), config_(config) {}
public slots: void start(); void stop();
signals: void frameReady(const Frame &frame); void error(const QString &message);
private: AppConfig config_;
};
