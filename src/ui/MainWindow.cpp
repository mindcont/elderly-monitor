#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), statusLabel_(nullptr), ui_(new Ui::MainWindow) {
    ui_->setupUi(this);
    statusLabel_ = ui_->statusLabel;
}

MainWindow::~MainWindow() {
    delete ui_;
}

void MainWindow::setMqttStatus(const QString &status) {
    statusLabel_->setText(status);
}
