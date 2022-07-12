#ifndef GPSDEMO_H
#define GPSDEMO_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
extern "C"{
#include "gps_bd/gps.h"
}
namespace Ui {
class GPSDemo;
}

class GPSDemo : public QWidget
{
    Q_OBJECT

public:
    explicit GPSDemo(QWidget *parent = nullptr);
    ~GPSDemo();

private slots:
    void portFind();
    void on_open_port_clicked();
    void read_data();
    void on_close_port_clicked();
    void on_clear_receive_button_clicked();
    void on_gps_map_clicked();

    void on_quit_clicked();

private:
    Ui::GPSDemo *ui;
    QSerialPort *serialport;

    GPS_INFO *GPS;
    QByteArray temp;   //存放GPS缓冲数据
    QByteArray checkData(QByteArray temp1,const QByteArray &s1,const QByteArray &s2);
    void GPS_DB_PARSE(QByteArray temp3);   //GPS/北斗数据解析
};

#endif // GPSDEMO_H
