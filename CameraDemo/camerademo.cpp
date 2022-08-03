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

//    ui->faceTrack->setCheckable(true);
//  connect(cameraThread, SIGNAL(errorshow()), this, SLOT(errorshowslot()));
//  connect(this,SIGNAL(Show_complete()),cameraThread,SLOT(startCapture()));

    connect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(videoDisplay(QImage)));
    connect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Camera_read_data(int, int)));

    connect(this, SIGNAL(Camera_connect(QString)),modbusThread,SLOT(on_connect(QString)));
//    connect(this, SIGNAL(Camera_write(quint16)),modbusThread,SLOT(on_write(quint16)));//only write
//    connect(this, SIGNAL(Camera_read(int,int)),modbusThread,SLOT(on_read(int,int)));//only read
    connect(this, SIGNAL(Camera_writeRead(int, quint16, quint16,quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16,quint16)));

    connect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Camera_change_connet(bool)));
    string xmlPath="./data/haarcascade_frontalface_default.xml";
    if(!ccf.load(xmlPath))   //加载训练文件
    {
        perror("不能加载指定的xml文件");
    }
    get_ip();
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

void CameraDemo::videoDisplay(const QImage image)
{
    image_tmp = image.copy().mirrored(true, false);

    if(faces_flag == true){
//        video_times ++;
//        if(video_times>3)
        {
//            video_times=0;
        QImage qImage ;
        vector<Rect> faces;  //创建一个容器保存检测出来的脸

        Mat img1, img2, gray;
        img1=Mat(image_tmp.height(), image_tmp.width(), CV_8UC3, (void*)image_tmp.constBits(), image_tmp.bytesPerLine());
        //flip(img1,img1,1);
        cv::resize(img1,img2,Size(320, 240));

        cvtColor(img2, img2, COLOR_BGR2RGB);
        cvtColor(img2, gray, COLOR_BGR2GRAY); //转换成灰度图，因为harr特征从灰度图中提取
        equalizeHist(gray,gray);  //直方图均衡行
        ccf.detectMultiScale(gray,faces,1.3,3,0,Size(50,50),Size(150,150)); //检测人脸
        for(vector<Rect>::const_iterator iter=faces.begin();iter!=faces.end();iter++)
        {
//            rectangle(img2,*iter,Scalar(0,0,255),2,10); //画出脸部矩形
            rectangle(img2,iter[0],Scalar(0,0,255),2,10); //画出脸部矩形
            qDebug()<<"cjfx"<<iter->x<<"y:"<<iter->y<<"w:"<<iter->width<<"h:"<<iter->height;

            {
            if( iter->x+(iter->width/2) > 200)
                H_Angle_num = H_Angle_num +2;//right
            if( iter->x+(iter->width/2) < 100)
                H_Angle_num = H_Angle_num -2;//left
            if ( iter->y+(iter->height/2)  > 140)
                V_Angle_num = V_Angle_num + 2;//down
            if ( iter->y+(iter->height/2)  < 80)
                V_Angle_num = V_Angle_num - 2;//up

            if(H_Angle_num < 45)
                H_Angle_num = 45;
            if(H_Angle_num > 225)
                H_Angle_num = 225;

            if(V_Angle_num < 45)
                V_Angle_num = 45;
            if(V_Angle_num > 100)
                V_Angle_num = 100;

            emit Camera_writeRead(CAMERA_ADDR1, 2, H_Angle_num,V_Angle_num);
//            emit Camera_writeRead(CAMERA_ADDR1, 1, H_Angle_num);
//            emit Camera_writeRead(CAMERA_ADDR2, 1, V_Angle_num);
//QDateTime current_date_time = QDateTime::currentDateTime();
//QString current_date = current_date_time.toString("yyyy-MM-dd");
//QString current_time = current_date_time.toString("hh:mm:ss.zzz ");
//qDebug()<<"time "<<current_date_time<<"H:"<<H_Angle_num<<"V:"<<V_Angle_num;
            }
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
//        ui->labelCamera->setPixmap(QPixmap::fromImage(qImage));
        ui->labelCamera->setPixmap(QPixmap::fromImage(qImage.scaled(ui->labelCamera->size(),Qt::KeepAspectRatio)));//全屏显示
    }
    }
    else
    {
//        QPixmap pixmap = QPixmap::fromImage(image_tmp);
//        ui->labelCamera->setPixmap(pixmap);
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

    CameraDemo::deleteLater();//关闭当前窗口
}
