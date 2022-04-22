#ifndef AICARDEMO_H
#define AICARDEMO_H

#include <QWidget>
#include <QGraphicsScene>
#include <QPainter>
#include "AICarDemo/car/car.h"

#include "services/camerathread.h"
#include "services/modbusthread.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

namespace Ui {
class AICarDemo;
}

class AICarDemo : public QWidget
{
    Q_OBJECT

public:
    explicit AICarDemo(QWidget *parent = nullptr,CameraThread *camerathread=nullptr,ModbusThread *modbusthread=nullptr);
    ~AICarDemo();
    QGraphicsScene *scene;
    QGraphicsView *view;
    Car *car ;

    CameraThread *cameraThread;
    ModbusThread *modbusThread;
    quint16 Car_turn_LR_Angle_num = 750;
    quint16 Car_AD_Rate_num = 450;
    qint8 Car_turn_flag = 0;
    qint8 Car_AD_flag = 0;
    float accx=0, accy=0, accz=0;
    QString radar_data;
    bool Car_RGB_flag;
signals:
    void Car_connect();
    void Car_writeRead(int startAddress, quint16 numberOfEntries, quint16 data);
    void Car_read(int startAddress, quint16 numberOfEntries);
    void Car_times(bool data);
private slots:
    void on_turnLeft_clicked();

    void on_turnRight_clicked();

    void on_accelerate_clicked();

    void on_decelerate_clicked();

    void on_connect_clicked();

    void Car_change_connet(bool data);
    void Car_read_data(int, int data);


    void Car_state_data();
    void Car_videoDisplay(const QImage image);

    void Car_videoDisplay1();

    void on_Car_reset_clicked();

protected:
    void closeEvent(QCloseEvent *event);
private:
    Ui::AICarDemo *ui;
    QTimer *car_state;

    QTimer *car_camera_state;
    QImage image_tmp;

    Scalar lower_red;
    Scalar upper_red;
    Scalar lower_green;
    Scalar upper_green;
    Scalar lower_yellow;
    Scalar upper_yellow;
};

#endif // AICARDEMO_H
