#include "camerademo.h"
#include "ui_camerademo.h"
#include <QDebug>
#include <QTimer>
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

//    ui->faceTrack->setCheckable(true);

//  connect(cameraThread, SIGNAL(errorshow()), this, SLOT(errorshowslot()));
//  connect(this,SIGNAL(Show_complete()),cameraThread,SLOT(startCapture()));

    connect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(videoDisplay(QImage)));
    connect(modbusThread, SIGNAL(on_read_data(int, quint16)),this,SLOT(Camera_read_data(int, quint16)));

    connect(this, SIGNAL(Camera_connect()),modbusThread,SLOT(on_connect()));
//    connect(this, SIGNAL(Camera_write(quint16)),modbusThread,SLOT(on_write(quint16)));//only write
//    connect(this, SIGNAL(Camera_read()),modbusThread,SLOT(on_read()));//only read
    connect(this, SIGNAL(Camera_writeRead(int, int, quint16)),modbusThread,SLOT(on_writeRead(int, int, quint16)));
    connect(this, SIGNAL(Camera_times(bool)),cameraThread,SLOT(Display_times(bool)));

    connect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Camera_change_connet(bool)));

}

CameraDemo::~CameraDemo(){

}
void CameraDemo::closeEvent(QCloseEvent *event)
{
    faces_flag = false;
    connect_flag = false;
    emit Camera_connect();
    emit Camera_times(faces_flag);

    disconnect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(videoDisplay(QImage)));
    disconnect(modbusThread, SIGNAL(on_read_data(int, quint16)),this,SLOT(Camera_read_data(int, quint16)));

    disconnect(this, SIGNAL(Camera_connect()),modbusThread,SLOT(on_connect()));
    disconnect(this, SIGNAL(Camera_writeRead(int, int, quint16)),modbusThread,SLOT(on_writeRead(int, int, quint16)));
    disconnect(this, SIGNAL(Camera_times(bool)),cameraThread,SLOT(Display_times(bool)));

    disconnect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Camera_change_connet(bool)));
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
bool CameraDemo::eventFilter(QObject *watched, QEvent *event)
{
//    qDebug()<<"cjf button JHJ";
//    if(connect_flag==true){
//        if(watched == ui->left){
//            if(event->type()==QEvent::MouseButtonPress){
//                on_left_clicked();
//                qDebug()<<"cjf button P";
//                return  true;
//                }
//            if(event->type()==QEvent::MouseButtonRelease){
//                qDebug()<<"cjf button R";
//                return true;
//            }
//        }
//    }
//    return  false;
}
void CameraDemo::errorshowslot()
{
    ui->labelCamera->setText(tr("摄像头初始化失败，请检查是否插好，并重新启动！"));
}

void CameraDemo::videoDisplay(const QImage image)
{
    QImage image1 = image.mirrored(true, false);
    if(faces_flag == true){
        QImage qImage ;
        vector<Rect> faces;  //创建一个容器保存检测出来的脸
        CascadeClassifier ccf;   //创建分类器对象
        Mat img1, img2, gray;
        string xmlPath="./data/haarcascade_frontalface_default.xml";
        img1=Mat(image1.height(), image1.width(), CV_8UC3, (void*)image1.constBits(), image1.bytesPerLine());
        //flip(img1,img1,1);

        if(!ccf.load(xmlPath))   //加载训练文件
        {
            perror("不能加载指定的xml文件");
        }

        cvtColor(img1, img2, COLOR_BGR2RGB);
        cvtColor(img2, gray, COLOR_BGR2GRAY); //转换成灰度图，因为harr特征从灰度图中提取
        equalizeHist(gray,gray);  //直方图均衡行
        ccf.detectMultiScale(gray,faces,1.3,3,0,Size(50,50),Size(200,200)); //检测人脸
        for(vector<Rect>::const_iterator iter=faces.begin();iter!=faces.end();iter++)
        {
            rectangle(img2,*iter,Scalar(0,0,255),2,10); //画出脸部矩形
//            qDebug()<<"cjfx"<<iter->x<<"y:"<<iter->y<<"w:"<<iter->width<<"h:"<<iter->height;
            if( iter->x > 300)
                H_Angle_num = H_Angle_num +2;//right
            if( iter->x < 180)
                H_Angle_num = H_Angle_num -2;//left
            if ( iter->y  > 150)
                V_Angle_num = V_Angle_num + 2;//down
            if ( iter->y  < 50)
                V_Angle_num = V_Angle_num - 2;//up

            if(H_Angle_num < 45)
                H_Angle_num = 45;
            if(H_Angle_num > 225)
                H_Angle_num = 225;

            if(V_Angle_num < 45)
                V_Angle_num = 45;
            if(V_Angle_num > 100)
                V_Angle_num = 100;
            emit Camera_writeRead(CAMERA_ADDR1, 1, H_Angle_num);
            emit Camera_writeRead(CAMERA_ADDR2, 1, V_Angle_num);
        }

        cvtColor(img2,img2, COLOR_BGR2RGB);
        if(img2.channels() == 3)
        {
            qImage = QImage((const unsigned char*)(img2.data),img2.cols,img2.rows,img2.cols * img2.channels(),
                            QImage::Format_RGB888);
        }else{
            qImage = QImage((const unsigned char*)(img2.data),img2.cols,img2.rows,img2.cols * img2.channels(),
                            QImage::Format_RGB888);
        }

        ui->labelCamera->setPixmap(QPixmap::fromImage(qImage));
  //    ui->labelCamera->setPixmap(QPixmap::fromImage(qImage.scaled(ui->labelCamera->size(),Qt::KeepAspectRatio)));//全屏显示
    }else {
        QPixmap pixmap = QPixmap::fromImage(image1);
        ui->labelCamera->setPixmap(pixmap);
    }

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
    }
    emit Camera_times(faces_flag);

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

void CameraDemo::on_faceTrack_clicked()
{
    faces_flag = faces_flag ? false : true;
    if(faces_flag == false)
        ui->faceTrack->setText(tr("人脸追踪"));
    else
        ui->faceTrack->setText(tr("取消追踪"));
    emit Camera_times(faces_flag);
}
