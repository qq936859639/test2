#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include "services/camerathread.h"
#include "services/modbusthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    CameraThread *cameraThread;
    ModbusThread *modbusThread;

private slots:
    void on_btcamera_clicked();

    void on_btcar_clicked();

    void on_btlcd_clicked();

    void on_btmic_clicked();

    void on_bta311d_clicked();

    void on_btkeyboard_clicked();

    void on_btgps_clicked();

    void on_btsonor_clicked();

    void on_btspeaker_clicked();

    void on_btiotdht_clicked();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
