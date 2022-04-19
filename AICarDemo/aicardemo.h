#ifndef AICARDEMO_H
#define AICARDEMO_H

#include <QWidget>
#include <QGraphicsScene>
#include <QPainter>
#include "AICarDemo/car/car.h"

#include "services/modbusthread.h"

namespace Ui {
class AICarDemo;
}

class AICarDemo : public QWidget
{
    Q_OBJECT

public:
    explicit AICarDemo(QWidget *parent = nullptr,ModbusThread *modbusthread=nullptr);
    ~AICarDemo();
    QGraphicsScene *scene;
    QGraphicsView *view;
    Car *car ;

    ModbusThread *modbusThread;
    quint16 Car_turn_LR_Angle_num = 750;
    quint16 Car_AD_Rate_num = 450;
    qint8 Car_turn_flag = 0;
    qint8 Car_AD_flag = 0;
    float accx=0, accy=0, accz=0;
    QString radar_data;
signals:
    void Car_connect();
    void Car_writeRead(int startAddress, quint16 numberOfEntries, quint16 data);
    void Car_read(int startAddress, quint16 numberOfEntries);
private slots:
    void on_turnLeft_clicked();

    void on_turnRight_clicked();

    void on_accelerate_clicked();

    void on_decelerate_clicked();

    void on_connect_clicked();

    void Car_change_connet(bool data);
    void Car_read_data(int, int data);

    void on_pushButton_clicked();
    void Car_state_data();

protected:
    void closeEvent(QCloseEvent *event);
private:
    Ui::AICarDemo *ui;
    QTimer *car_state;

};

#endif // AICARDEMO_H
