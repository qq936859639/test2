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
//    connect(this, SIGNAL(Camera_write(quint16)),modbusThread,SLOT(on_write(quint16)));//only write
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

void CameraDemo::videoDisplay(const QImage image)
{
    string xmlPath="./haarcascade_frontalface_default.xml";
    CascadeClassifier ccf;   //创建分类器对象
    Mat img;
    img=Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
    //Mat img=imread(img1);
    if(!ccf.load(xmlPath))   //加载训练文件
    {
        perror("不能加载指定的xml文件");
    }
    vector<Rect> faces;  //创建一个容器保存检测出来的脸
    Mat gray;
    cvtColor(img,gray, COLOR_BGR2GRAY); //转换成灰度图，因为harr特征从灰度图中提取
    equalizeHist(gray,gray);  //直方图均衡行
    ccf.detectMultiScale(gray,faces,1.1,3,0,Size(10,10),Size(100,100)); //检测人脸
    for(vector<Rect>::const_iterator iter=faces.begin();iter!=faces.end();iter++)
    {
        rectangle(img,*iter,Scalar(0,0,255),2,10); //画出脸部矩形
    }
    QImage qImage ;
    Mat img2;
    cvtColor(img,img2, COLOR_BGR2RGB);
    if(img2.channels() == 3)
    {
        qImage = QImage((const unsigned char*)(img2.data),img2.cols,img2.rows,
                       QImage::Format_RGB888);
    }else{
        qImage = QImage((const unsigned char*)(img2.data),img2.cols,img2.rows,
                       QImage::Format_RGB888);
    }
ui->labelCamera->setPixmap(QPixmap::fromImage(qImage));
perror("cjf ok");
//    QPixmap pixmap = QPixmap::fromImage(img);
//    ui->labelCamera->setPixmap(pixmap);
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

