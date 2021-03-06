#ifndef SMARTHOME_H
#define SMARTHOME_H

#include <QWidget>
#include <qmqttclient.h>
#include "cjson/cJSON.h"//add
#include "services/camerathread.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "BaiduSpeech/audio.h"
#include "BaiduSpeech/speech.h"

#include "../MICDemo/hidmicdemo/hidmicdemo.h"
using namespace std;
using namespace cv;

namespace Ui {
class SmartHome;
}

class SmartHome : public QWidget
{
    Q_OBJECT

public:
    explicit SmartHome(QWidget *parent = nullptr,CameraThread *camerathread=nullptr);
    ~SmartHome();


public slots:
    void setClientPort(int p);
    void setClientPort_dht11(int p);
    void setClientPort_ledb(int p);
    void setClientPort_ocr(int p);

private slots:
    void on_Connect_PIR_clicked();
    void updateLogStateChange();
    void brokerDisconnected();
    void on_Subscribe_PIR_clicked();
    void on_Ping_PIR_clicked();
    void on_Publish_PIR_clicked();

    void on_Connect_DHT11_clicked();
    void updateLogStateChange_dht11();
    void brokerDisconnected_dht11();
    void on_Subscribe_DHT11_clicked();
    void on_Ping_DHT11_clicked();

    void on_Connect_LEDB_clicked();
    void updateLogStateChange_ledb();
    void brokerDisconnected_ledb();
    void on_Subscribe_LEDB_clicked();
    void on_Publish_LEDB_clicked();
    void on_Ping_LEDB_clicked();
    void on_LED_ON_clicked();
    void on_LED_OFF_clicked();
    void on_BUZZER_ON_clicked();
    void on_BUZZER_OFF_clicked();

    void on_Connect_OCR_clicked();
    void updateLogStateChange_ocr();
    void brokerDisconnected_ocr();
    void on_Subscribe_OCR_clicked();
    void on_Publish_OCR_clicked();
    void on_Ping_OCR_clicked();
    void on_OCR_ON_clicked();
    void on_OCR_OFF_clicked();

    void on_Key_SmartHome_clicked();
    void on_Quit_SmartHome_clicked();

    void QTimer_Subscribe();

    void on_LED_data_clicked();
    void on_BUZZER_data_clicked();
    void on_PIR_data_clicked();
    void on_OCR_data_clicked();

    void SmartHome_videoDisplay(const QImage image);
    void SmartHome_Play();

    void on_speech_pressed();
    void on_speech_released();

private:
    Ui::SmartHome *ui;

    void init_PIR();
    void init_DHT11();
    void init_OCR();
    void init_LEDB();

    QMqttClient *m_client_pir;
    QMqttClient *m_client_dht11;
    QMqttClient *m_client_ocr;
    QMqttClient *m_client_ledb;

    quint8 pir_flag;
    quint8 play_flag;

    CameraThread *cameraThread;

    QImage Mat2QImage(const Mat &mat);
    Mat QImage2Mat(const QImage& image);

    Audio *audio;
protected:
    void closeEvent(QCloseEvent *event);
    Mat FaceRecognition(const Mat &mat);
    CascadeClassifier ccf;   //?????????????????????
    HIDMICDEMO *hidmic;
};

#endif // SMARTHOME_H
