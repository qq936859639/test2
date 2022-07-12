#ifndef CAMERADEMO_H
#define CAMERADEMO_H

#include <QWidget>
#include "services/camerathread.h"
#include "services/modbusthread.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QKeyEvent>

using namespace std;
using namespace cv;

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
    quint16 H_Angle_num = 135;
    quint16 V_Angle_num = 70;
    bool faces_flag;
    bool connect_flag;

    void keyPressEvent(QKeyEvent *event) override;
signals:
    void Show_complete();

    void Camera_connect(const QString ip);
    void Camera_write(quint16 data);
//    void Camera_read(int, int);
    void Camera_writeRead(int startAddress, quint16 numberOfEntries, quint16 data);

private slots:
    void videoDisplay(const QImage img);
    void Camera_read_data(int startAddress, int data);
    void Camera_change_connet(bool data);

    void errorshowslot();

    void on_down_clicked();
    void on_up_clicked();
    void on_left_clicked();
    void on_right_clicked();

    void on_connect_clicked();

    void on_faceTrack_clicked();

    void on_quit_clicked();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::test *ui;
    CascadeClassifier ccf;   //创建分类器对象
    QImage image_tmp;
};

#endif // CAMERADEMO_H
