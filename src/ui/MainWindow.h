#pragma once
#include <QMainWindow>
class QLabel; class MainWindow : public QMainWindow {
    Q_OBJECT
public: explicit MainWindow(QWidget *parent = nullptr);
public slots: void setMqttStatus(const QString &status);
private: QLabel *statusLabel_;
};
