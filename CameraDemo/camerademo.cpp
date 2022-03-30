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
    connect(modbusThread, SIGNAL(on_read_data(int, quint16)),this,SLOT(Camera_read_data(int, quint16)));

    connect(this, SIGNAL(Camera_connect()),modbusThread,SLOT(on_connect()));
    connect(this, SIGNAL(Camera_write(quint16)),modbusThread,SLOT(on_write(quint16)));//only write
//    connect(this, SIGNAL(Camera_read()),modbusThread,SLOT(on_read()));//only read
    connect(this, SIGNAL(Camera_writeRead(int, int, quint16)),modbusThread,SLOT(on_writeRead(int, int, quint16)));

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

void CameraDemo::Camera_read_data(int address, quint16 data)
{
    if(address == CAMERA_ADDR1)
        ui->HorizontalValue->setText(QString::number(data));
    if(address == CAMERA_ADDR2)
        ui->VerticalValue->setText(QString::number(data));
}

void CameraDemo::Camera_change_connet(bool data)
{
    if(data == false)
        ui->connect->setText(tr("Connect"));
    if(data == true){
        emit Camera_writeRead(CAMERA_ADDR1, 1, H_Angle_num);
        emit Camera_writeRead(CAMERA_ADDR2, 1, V_Angle_num);
        ui->connect->setText(tr("Disconnect"));
    }
}

void CameraDemo::on_connect_clicked()
{
    emit Camera_connect();
}

void CameraDemo::on_up_clicked()
{
    V_Angle_num = V_Angle_num - 5;
    if(V_Angle_num < 45)
        V_Angle_num = 45;
    emit Camera_writeRead(CAMERA_ADDR2, 1, V_Angle_num);
}

void CameraDemo::on_down_clicked()
{
    V_Angle_num = V_Angle_num + 5;
    if(V_Angle_num > 100)
        V_Angle_num = 100;
    emit Camera_writeRead(CAMERA_ADDR2, 1, V_Angle_num);
}

void CameraDemo::on_left_clicked()
{
    H_Angle_num = H_Angle_num - 5;
    if(H_Angle_num < 45)
        H_Angle_num = 45;
    emit Camera_writeRead(CAMERA_ADDR1, 1, H_Angle_num);
}

void CameraDemo::on_right_clicked()
{
    H_Angle_num = H_Angle_num + 5;
    if(H_Angle_num > 225)
        H_Angle_num = 225;
    emit Camera_writeRead(CAMERA_ADDR1, 1, H_Angle_num);
}

