#pragma once
#include <QMainWindow>
#include <QLabel>

namespace Ui { class MainWindow; }
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
public slots:
    void setMqttStatus(const QString &status);
private:
    QLabel *statusLabel_;
    Ui::MainWindow *ui_;
};
