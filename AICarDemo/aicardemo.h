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

#include "lpr/include/plate_detectors.h"
#include "lpr/include/plate_recognizers.h"
#include "lpr/cvUniText.hpp"
#include "lpr/lpr.h"

#include <QSerialPortInfo>
#include <QSerialPort>

//#include "rplidar/rplidar.h"

//语音
#include "SmartHome/BaiduSpeech/audio.h"
#include "SmartHome/BaiduSpeech/speech.h"
#include "MICDemo/hidmicdemo/hidmicdemo.h"

//MQTT
#include <qmqttclient.h>
#include "SmartHome/cjson/cJSON.h"

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
//    qint8 Car_turn_LR_Angle_num = 0;
//    qint8 Car_AD_Rate_num = 0;
    qint8 Car_TURN_flag = 0;
    qint8 Car_AD_flag = 0;
    float accx=0, accy=0, accz=0;
    QString radar_data;
    bool Car_RGB_flag;

    QImage Mat2QImage(const Mat &mat);
    Mat QImage2Mat(const QImage& image);
    void get_ip();
    void save_ip();

    void portFind();
//    RPLIDAR *rplidar;
signals:
    void Car_connect(const QString ip);
    void Car_writeRead(int startAddress, quint16 numberOfEntries, quint16 data);
    void Car_read(int startAddress, quint16 numberOfEntries);

    void Car_radar(int mi_data,int ul_data,int la_radar);
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

    void Car_traffic_light_Play();
    void on_Car_reset_clicked();

    void Uart_ReadData();//串口读取接口槽函数
    void on_pushButton_clicked();
    void AutoPilotSystem();
    void on_townhall_clicked();
    void on_mall_clicked();
    void on_school_clicked();
    void on_gym_clicked();
    void on_ul_clicked();
    void on_rplidar_clicked();
    void on_radar_clicked();

    void Read_Radar(int mi_data,int ul_data,int la_radar);
    void on_scene_clicked();

    void on_connectType_currentIndexChanged(int index);

    void on_speech_pressed();

    void on_speech_released();

    //MQTT
    void SmartCar_MQTT_init();
    void brokerDisconnected();
    void on_Key_SmartCar_clicked();
    void on_Subscribe();
    void QTimer_Subscribe();
    void QTimer_Buzzer();

    void on_LED_data_clicked();

    void on_BUZZER_data_clicked();

    void on_OCR_data_clicked();

protected:
    void closeEvent(QCloseEvent *event);

    Mat rgy_light_identification(const Mat &mat);
    Mat FaceRecognition(const Mat &mat);

    void Uart_Connect();
    void Uart_WriteData();
    void Uart_Close();
CascadeClassifier ccf;   //创建分类器对象
private:
    Ui::AICarDemo *ui;
    QTimer *car_state;
    QTimer *video_play;
    QTimer *AutoPilot;

    QImage image_tmp;

    Scalar lower_red;
    Scalar upper_red;
    Scalar lower_green;
    Scalar upper_green;
    Scalar lower_yellow;
    Scalar upper_yellow;

    quint8 rgy_light_play_flag;
    quint8 ul_play_flag;
    quint8 car_play_flag;
    quint8 car_face_flag;
    quint8 car_lpr_flag;
    quint8 car_rgy_flag;

    PLR *plr;

    QSerialPortInfo *SerialPortInfo=NULL;
    QSerialPort SerialPort;

    void Car_Reset();
    void Car_Map_Home(QPointF point);
    void Car_Map_Mall(QPointF point);
    void Car_Map_TownHall(QPointF point);
    quint16 Car_END_flag;

    quint8 Car_state_flag;
    quint8 Car_state1_flag;
    quint8 Car_state2_flag;
    quint8 Car_rplidar_flag;
    quint8 Car_rplidar_flag_stop;
    QMovie *pm;

    void Open_Radar();
    void Close_Radar();
    quint8 Car_millimeter_radar=0;
    int ul_last_data;

    HIDMICDEMO *hidmic;
    quint8 hidmic_open_flag=0;

    //MQTT
    QMqttClient *m_client;
    QString m_subscribe;
    QString m_publish;
    QString state_LED_data;
    QString state_BUZZER_data;
    QString state_OCR_data;
};

#endif // AICARDEMO_H
