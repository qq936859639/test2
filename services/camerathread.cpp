#include "camerathread.h"

CameraThread::CameraThread(QObject *parent) : QThread(parent)
{
    camera = new V4L();
    startflag = false;
    timesflag = false;
    if(-1 == camera->openVideo(DEV_NAME)){//打开摄像头设备
        //emit errorshow();
        perror("openVideo fail");
    }else{
        camera->getVideoFmt();
        if(-1 == (camera->queryVideoCap()))// 查询视频设备的功能
        {
            perror("queryVideoCap fail");
            exit(-1);
        }

        if(-1 == (camera->setVideoFmt()))//设置视频设备的视频数据格式
        {
            printf("setVideoFmt fail, try again!");
            camera->closeVideo(camera->videofd);
            if(-1 == (camera->openVideo(DEV_NAME1)))  //打开摄像头设备
            {
                //emit errorshow();
                perror("openVideo fail");
                exit(-1);
            }
            camera->getVideoFmt(); //获取当前视频设备支持的视频格式
            if(-1 == (camera->queryVideoCap())) // 查询视频设备的功能
            {
                perror("queryVideoCap fail");
                exit(-1);
            }
            if(-1 == (camera->setVideoFmt())) //设置视频设备的视频数据格式
            {
                perror("setVideoFmt fail");
                exit(-1);
            }
        }

        if(-1 == (camera->requestVideoBufsAndMmap())) //请求V4L2驱动分配视频缓冲区（申请V4L2视频驱动分配内存）
        {
            perror("requestVideoBufs fail");
            exit(-1);
        }
        startflag = true;
    }
}

CameraThread::~CameraThread()
{
   // delete ;
    camera->closeVideo(camera->videofd);
    camera->uninitVideo();
    startflag = false;
    qDebug()<<"cjf camerathread";
}

void CameraThread::run(){
    while (1) {
        usleep(1);
        if(startflag == true)
        {
            if(-1 == camera->readFrame())
            {
                perror("readFrame fail");
            }else{
                getImage();
            }
            if(timesflag ==false)
                usleep(20000);
            else
                msleep(200);
        }
    }
}

QImage CameraThread::getImage()
{
    QImage image;
    image = QImage((const unsigned char *)camera->frame_buffer,IMAGE_WIDTH,IMAGE_HEIGHT,QImage::Format_RGB888).mirrored(false, false);
    emit Collect_complete(image);
    return image;
}
void CameraThread::Display_times(bool data)
{
    timesflag = data;
}
