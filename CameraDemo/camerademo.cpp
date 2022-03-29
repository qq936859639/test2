#include "camerademo.h"
#include "ui_camerademo.h"
CameraDemo::CameraDemo(QWidget *parent, CameraThread *camerathread, ModbusThread *modbusthread) :
      QWidget(parent),ui(new Ui::test)
{
    ui->setupUi(this);
    this->cameraThread = camerathread;
    this->modbusThread = modbusthread;

//  connect(cameraThread, SIGNAL(errorshow()), this, SLOT(errorshowslot()));
//  connect(this,SIGNAL(Show_complete()),cameraThread,SLOT(startCapture()));

    connect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(videoDisplay(QImage)));
    connect(modbusThread, SIGNAL(on_read_data(quint16)),this,SLOT(Camera_read_data(quint16)));

    connect(this, SIGNAL(Camera_connect()),modbusThread,SLOT(on_connect()));
//    connect(this, SIGNAL(Camera_write(quint16)),modbusThread,SLOT(on_write(quint16)));//only write
//    connect(this, SIGNAL(Camera_read()),modbusThread,SLOT(on_read()));//only read
    connect(this, SIGNAL(Camera_write(quint16)),modbusThread,SLOT(on_writeRead(quint16)));

    connect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Camera_change_connet(bool)));



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

void CameraDemo::Camera_read_data(quint16 data)
{
    ui->VerticalValue->setText(QString::number(data));
}

void CameraDemo::Camera_change_connet(bool data)
{

    if(data == false)
        ui->connect->setText(tr("Connect"));
    if(data == true)
        ui->connect->setText(tr("Disconnect"));
}

void CameraDemo::on_connect_clicked()
{
    emit Camera_connect();
}

void CameraDemo::on_up_clicked()
{

}

void CameraDemo::on_down_clicked()
{

}

void CameraDemo::on_left_clicked()
{
    num = num - 5;
    if(num < 45)
        num = 45;
    emit Camera_write(num);
}

void CameraDemo::on_right_clicked()
{
    num = num + 5;
    if(num > 225)
        num = 225;
    emit Camera_write(num);
}

