#pragma once
#include "Frame.h"
#include <QObject>
class RtspCamera : public QObject {
    Q_OBJECT
public: explicit RtspCamera(QObject *parent = nullptr) : QObject(parent) {}
public slots: void start(); void stop();
signals: void frameReady(const Frame &frame); void error(const QString &message);
};
