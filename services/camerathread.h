#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QObject>
#include <QDebug>
#include <QImage>
#include "v4l.h"

class CameraThread : public QThread
{
    Q_OBJECT
public:
    explicit CameraThread(QObject *parent = nullptr);
    ~CameraThread();
    V4L *camera;
    QImage getImage();

signals:
    void Collect_complete(const QImage);
private slots:
    void Display_times(bool data);
protected:
    bool startflag;
    bool timesflag;
    void run();

private:

};

#endif // CAMERATHREAD_H
