﻿#ifndef CAMERADEMO_H
#define CAMERADEMO_H

#include <QWidget>
#include "services/camerathread.h"
#include "services/modbusthread.h"

namespace Ui {
class test;
}


class CameraDemo : public QWidget
{
    Q_OBJECT
public:
    explicit CameraDemo(QWidget *parent = nullptr, CameraThread *camerathread=nullptr, ModbusThread *modbusthread=nullptr);
    ~CameraDemo();
    CameraThread *cameraThread;
    ModbusThread *modbusThread;
    quint16 num = 135;
signals:
    void Show_complete();

    void Camera_connect();
    void Camera_write(quint16 data);
    void Camera_read();
    void Camera_readWrite(quint16 data);
private slots:
    void videoDisplay(const QImage img);
    void Camera_read_data(quint16 data);
    void Camera_change_connet(bool data);

    void errorshowslot();

    void on_down_clicked();
    void on_up_clicked();
    void on_left_clicked();
    void on_right_clicked();

    void on_connect_clicked();

private:
    Ui::test *ui;
};

#endif // CAMERADEMO_H
