#include "camerademo.h"
#include "ui_camerademo.h"
CameraDemo::CameraDemo(QWidget *parent, CameraThread *camerathread) :
      QWidget(parent),ui(new Ui::test)
{
    ui->setupUi(this);
    this->cameraThread = camerathread;

//  connect(cameraThread, SIGNAL(errorshow()), this, SLOT(errorshowslot()));
//  connect(this,SIGNAL(Show_complete()),cameraThread,SLOT(startCapture()));
    connect(cameraThread,SIGNAL(Collect_complete(QImage)),this,SLOT(videoDisplay(QImage)));
}
CameraDemo::~CameraDemo(){
    //disconnect(cameraThread, SIGNAL(errorshow()), this, SLOT(errorshowslot()));
    disconnect(cameraThread,SIGNAL(Collect_complete(QImage)),this,SLOT(videoDisplay(unsigned char*)));

}
void CameraDemo::errorshowslot()
{
    ui->labelCamera->setText(tr("摄像头初始化失败，请检查是否插好，并重新启动！"));
}
void CameraDemo::videoDisplay(const QImage img)
{
    QPixmap pixmap = QPixmap::fromImage(img);
    ui->labelCamera->setPixmap(pixmap);
}

