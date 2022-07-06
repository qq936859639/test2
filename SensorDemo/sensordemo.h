#ifndef SENSORDEMO_H
#define SENSORDEMO_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
namespace Ui {
class SensorDemo;
}

class SensorDemo : public QWidget
{
    Q_OBJECT

public:
    explicit SensorDemo(QWidget *parent = nullptr);
    ~SensorDemo();

private slots:
    void on_open_port_clicked();
    void on_close_port_clicked();
    void on_clear_receive_button_clicked();

    void portFind();
    void read_data();

    void on_quit_clicked();

private:
    Ui::SensorDemo *ui;
    QSerialPort *serialport;
    void Sensor_PARSE(QByteArray temp);
};

#endif // SENSORDEMO_H
