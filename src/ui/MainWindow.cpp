#include "MainWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("老人智能监测系统"); resize(1100, 700);
    auto *root = new QWidget(this); auto *layout = new QVBoxLayout(root);
    layout->addWidget(new QLabel("RTSP 实时画面 / 检测框将在此显示", root));
    statusLabel_ = new QLabel("系统启动中", root); layout->addWidget(statusLabel_);
    setCentralWidget(root);
}
void MainWindow::setMqttStatus(const QString &status) { statusLabel_->setText(status); }
