#ifndef CAMERADEMO_H
#define CAMERADEMO_H

#include <QWidget>
#include "services/camerathread.h"

namespace Ui {
class test;
}


class CameraDemo : public QWidget
{
    Q_OBJECT
public:
    explicit CameraDemo(QWidget *parent = nullptr, CameraThread *camerathread=nullptr);
    ~CameraDemo();
    CameraThread *cameraThread;

signals:
    void Show_complete();
private slots:
    void videoDisplay(const QImage img);
    void errorshowslot();

private:
    Ui::test *ui;
};

#endif // CAMERADEMO_H
