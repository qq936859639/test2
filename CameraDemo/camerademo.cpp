#include "camerademo.h"
#include "ui_camerademo.h"
#include <QDebug>
#include <QTime>
CameraDemo::CameraDemo(QWidget *parent, CameraThread *camerathread, ModbusThread *modbusthread) :
      QWidget(parent),ui(new Ui::test)
{
    ui->setupUi(this);

    faces_flag = false;
    connect_flag = false;
    this->cameraThread = camerathread;
    this->modbusThread = modbusthread;

    ui->faceTrack->setDisabled(true);
    ui->up->setDisabled(true);
    ui->down->setDisabled(true);
    ui->left->setDisabled(true);
    ui->right->setDisabled(true);

    ui->up->setAutoRepeat(true);
    ui->up->setAutoRepeatDelay(500);
    ui->up->setAutoRepeatInterval(200);
    ui->down->setAutoRepeat(true);
    ui->down->setAutoRepeatDelay(500);
    ui->down->setAutoRepeatInterval(200);

    ui->left->setAutoRepeat(true);
    ui->left->setAutoRepeatDelay(500);
    ui->left->setAutoRepeatInterval(200);
    ui->right->setAutoRepeat(true);
    ui->right->setAutoRepeatDelay(500);
    ui->right->setAutoRepeatInterval(200);

//  ui->faceTrack->setCheckable(true);
//  connect(cameraThread, SIGNAL(errorshow()), this, SLOT(errorshowslot()));
//  connect(this,SIGNAL(Show_complete()),cameraThread,SLOT(startCapture()));

    connect(this, SIGNAL(on_connectType(quint8)), modbusThread, SLOT(on_connectType_currentIndexChanged(quint8)));

    connect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(videoDisplay(QImage)));
    connect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Camera_read_data(int, int)));

    connect(this, SIGNAL(Camera_connect(QString)),modbusThread,SLOT(on_connect(QString)));
//    connect(this, SIGNAL(Camera_write(quint16)),modbusThread,SLOT(on_write(quint16)));//only write
//    connect(this, SIGNAL(Camera_read(int,int)),modbusThread,SLOT(on_read(int,int)));//only read
    connect(this, SIGNAL(Camera_writeRead(int, quint16, quint16,quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16,quint16)));

    connect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Camera_change_connet(bool)));

    faces = new FACES();
    connect(faces,SIGNAL(locationInfo(int,int,int,int)),this,SLOT(faceLocation(int,int,int,int)));

    get_ip();
    pid_x = (PID *)malloc(sizeof(PID));
    pid_y = (PID *)malloc(sizeof(PID));
    pid_x->Kp=3;
    pid_x->Ki=0;
    pid_x->Kd=1;
    pid_x->LastError=0;
    pid_x->PrevError=0;
    pid_x->SumError=0;
    pid_y->Kp=3;
    pid_y->Ki=0;
    pid_y->Kd=1;
    pid_y->LastError=0;
    pid_y->PrevError=0;
    pid_y->SumError=0;
}

CameraDemo::~CameraDemo(){

}
void CameraDemo::closeEvent(QCloseEvent *event)
{
    faces_flag = false;
    connect_flag = false;
    emit Camera_connect(ui->lineEdit->text());

    disconnect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(videoDisplay(QImage)));
    disconnect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Camera_read_data(int, int)));

    disconnect(this, SIGNAL(Camera_connect(QString)),modbusThread,SLOT(on_connect(QString)));
    disconnect(this, SIGNAL(Camera_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    disconnect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Camera_change_connet(bool)));
    disconnect(faces,SIGNAL(locationInfo(int,int,int,int)),this,SLOT(faceLocation(int,int,int,int)));
    free(pid_x);pid_x=nullptr;
    free(pid_y);pid_y=nullptr;
}

void CameraDemo::keyPressEvent(QKeyEvent *event)
{
    if(connect_flag == true){
        if(event->key()==Qt::Key_A)
            on_left_clicked();
        if(event->key()==Qt::Key_D)
            on_right_clicked();
        if(event->key()==Qt::Key_W)
            on_up_clicked();
        if(event->key()==Qt::Key_S)
            on_down_clicked();
    }
}

void CameraDemo::errorshowslot()
{
    ui->labelCamera->setText(tr("摄像头初始化失败，请检查是否插好，并重新启动！"));
}

void CameraDemo::on_connectType_currentIndexChanged(quint8 index)
{
    emit on_connectType(index);
}

void CameraDemo::videoDisplay(const QImage image)
{
    if(image.isNull())
        return;
    image_tmp = image.copy().mirrored(true, false);

    if(faces_flag == true){
        video_times++;
        if(video_times>1){
            video_times = 0;
            image_tmp = image_tmp.scaled(IMAGE_WIDTH/2,IMAGE_HEIGHT/2);
            QImage  qimg = faces->face_recognition(image_tmp);

            QPixmap pixmap = QPixmap::fromImage(qimg);
            ui->labelCamera->setPixmap(pixmap.scaled(ui->labelCamera->size(),Qt::KeepAspectRatio));//Qt::SmoothTransformation 保持比例
        }
    }else{
        ui->labelCamera->setPixmap(QPixmap::fromImage(image_tmp.scaled(ui->labelCamera->size(),Qt::KeepAspectRatio)));//全屏显示
    }
}

void CameraDemo::Camera_read_data(int address, int data)
{
    if(address == CAMERA_ADDR1)
        ui->HorizontalValue->setText(QString::number(data));
    if(address == CAMERA_ADDR2)
        ui->VerticalValue->setText(QString::number(data));
}

void CameraDemo::Camera_change_connet(bool data)
{
    faces_flag = false;
    ui->faceTrack->setText(tr("人脸追踪"));
    if(data == false)
    {
        connect_flag = false;
        ui->connect->setText(tr("Connect"));
        ui->faceTrack->setDisabled(true);

        ui->up->setDisabled(true);
        ui->down->setDisabled(true);
        ui->left->setDisabled(true);
        ui->right->setDisabled(true);
    }
    if(data == true){
        connect_flag = true;
        emit Camera_writeRead(CAMERA_ADDR1, 1, H_Angle_num);
        emit Camera_writeRead(CAMERA_ADDR2, 1, V_Angle_num);
        ui->connect->setText(tr("Disconnect"));
        ui->faceTrack->setDisabled(false);

        ui->up->setDisabled(false);
        ui->down->setDisabled(false);
        ui->left->setDisabled(false);
        ui->right->setDisabled(false);
        save_ip();
    }
}

void CameraDemo::on_connect_clicked()
{
    emit Camera_connect(ui->lineEdit->text());
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

void CameraDemo::on_faceTrack_clicked()
{
    faces_flag = faces_flag ? false : true;
    if(faces_flag == false)
        ui->faceTrack->setText(tr("人脸追踪"));
    else
        ui->faceTrack->setText(tr("取消追踪"));
}
void CameraDemo::get_ip()
{
    // QFile 构造函数中打开文件
    QFile file("./data/ip.sh");
    // 只读打开文件
    if (file.open(QIODevice::ReadOnly))
    {
        char buffer[100];
        // 返回读成功的字节数，失败返回-1
        qint64 lineLen = file.readLine(buffer, sizeof(buffer));
        if (lineLen != -1)
        {
            ui->lineEdit->setText(buffer);
        }
        file.close();
    }

}
void CameraDemo::save_ip()
{
    QString ip = ui->lineEdit->text();
    QFile *myFile;
    QTextStream *outFile;
    QString filename="./data/ip.sh";
    myFile=new QFile(filename);
    if(myFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outFile=new QTextStream(myFile);
        *outFile<<ip;
        myFile->close();
    }
}
double CameraDemo::PIDCalc(PID *pp,double NextPoint)
{
        double dError,Error;
        Error = NextPoint;//Error = pp->SetPoint - NextPoint;
        pp->SumError += Error;
        dError = pp->LastError - pp->PrevError;
        pp->PrevError = pp->LastError;
        pp->LastError = Error;

        return(pp->Kp*Error + pp->Ki*pp->SumError+pp->Kd*dError);
}
void CameraDemo::faceLocation(int x,int y,int width,int height)
{
    int error_x , error_y, pid_x_p, pid_y_p;
    int Xcent = x + (width-x) / 2;//计算距离x、y轴中点
    int Ycent = y + (height-y) / 2;
    error_x = Xcent - IMAGE_WIDTH/2/2;//获取误差(x和y方向)
    error_y = Ycent - IMAGE_HEIGHT/2/2;

    pid_x_p  = PIDCalc(pid_x,error_x);
    pid_y_p  = PIDCalc(pid_y,error_y);
    if(pid_x_p/60==0&&pid_y_p/60==0)
        return;
    if(pid_x_p>320)
            pid_x_p=pid_x_p/2;
    if(pid_x_p<-320)
            pid_x_p=-pid_x_p/2;
    if(pid_y_p>240)
            pid_x_p=240/2;
    if(pid_y_p<-240)
            pid_x_p=-240/2;

    H_Angle_num = H_Angle_num + (int)(pid_x_p/60.0);
    V_Angle_num = V_Angle_num + (int)(pid_y_p/60.0);

    if(H_Angle_num < 45)
        H_Angle_num = 45;
    if(H_Angle_num > 225)
        H_Angle_num = 225;

    if(V_Angle_num < 45)
        V_Angle_num = 45;
    if(V_Angle_num > 100)
        V_Angle_num = 100;

    emit Camera_writeRead(CAMERA_ADDR1, 2, H_Angle_num,V_Angle_num);
}
void CameraDemo::on_quit_clicked()
{
    faces_flag = false;
    connect_flag = false;
    emit Camera_connect(ui->lineEdit->text());

    disconnect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(videoDisplay(QImage)));
    disconnect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Camera_read_data(int, int)));

    disconnect(this, SIGNAL(Camera_connect(QString)),modbusThread,SLOT(on_connect(QString)));
    disconnect(this, SIGNAL(Camera_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    disconnect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Camera_change_connet(bool)));
    disconnect(faces,SIGNAL(locationInfo(int,int,int,int)),this,SLOT(faceLocation(int,int,int,int)));

    free(pid_x);pid_x=nullptr;
    free(pid_y);pid_y=nullptr;
    CameraDemo::deleteLater();//关闭当前窗口
}
