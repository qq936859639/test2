#include "aicardemo.h"
#include "ui_aicardemo.h"
#include <QTimer>
#include <QDebug>
#include <math.h>
#include <QSound>//声音
//#include <fstream>   //文本读写


AICarDemo::AICarDemo(QWidget *parent, CameraThread *camerathread, ModbusThread *modbusthread) :
    QWidget(parent),
    ui(new Ui::AICarDemo)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-500, -500, 1000, 1000);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    car = new Car();
    scene->addItem(car);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();

    car_state = new QTimer(this);
    car_state->setInterval(500);
    connect(car_state,SIGNAL(timeout()),this,SLOT(Car_state_data()));

//    car_camera_state = new QTimer(this);
//    car_camera_state->setInterval(1000/20*5);
//    connect(car_camera_state,SIGNAL(timeout()),this,SLOT(Car_videoDisplay1()));
//    car_camera_state->start();

    car_rgy_light_play = new QTimer(this);
    car_rgy_light_play->setInterval(3000);
    connect(car_rgy_light_play,SIGNAL(timeout()),this,SLOT(Car_traffic_light_Play()));
//    car_rgy_light_play->start();
    rgy_light_play_flag = 0;

//    lower_red = Scalar(0, 100, 100);
//    upper_red = Scalar(10, 255, 255);
    lower_red = Scalar(0, 100, 100);
    upper_red = Scalar(10, 255, 255);
    lower_green = Scalar(40, 50, 50);
    upper_green = Scalar(90, 255, 255);
    lower_yellow = Scalar(15, 100, 100);
    upper_yellow = Scalar(35, 255, 255);

    ui->RGYButton->setChecked(true);
    this->cameraThread = camerathread;
    this->modbusThread = modbusthread;
    connect(this, SIGNAL(Car_connect()),modbusThread,SLOT(on_connect()));
    connect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Car_change_connet(bool)));

    connect(this, SIGNAL(Car_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    connect(this, SIGNAL(Car_read(int,quint16)),modbusThread,SLOT(on_read(int,quint16)));//only read
    connect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Car_read_data(int, int)));

    connect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(Car_videoDisplay(QImage)));
    plr = new PLR();
    faces = new FACES();
    string xmlPath="./data/haarcascade_frontalface_default.xml";
    if(!ccf.load(xmlPath))   //加载训练文件
    {
        perror("不能加载指定的xml文件");
    }
}

AICarDemo::~AICarDemo()
{
    delete ui;
}
void AICarDemo::closeEvent(QCloseEvent *event)
{
    car_state->stop();
    emit Car_connect();
    emit Car_writeRead(CAR_COMMAND_ADDR, 1, 0);//小车复位
//    car_camera_state->stop();
    disconnect(this, SIGNAL(Car_connect()),modbusThread,SLOT(on_connect()));
    disconnect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Car_change_connet(bool)));

    disconnect(this, SIGNAL(Car_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    disconnect(this, SIGNAL(Car_read(int,quint16)),modbusThread,SLOT(on_read(int,quint16)));//only read
    disconnect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Car_read_data(int, int)));

    disconnect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(Car_videoDisplay(QImage)));
}

void AICarDemo::on_turnLeft_clicked()
{
    car->turnLeft();
    //-5 -4 -3 -2 -1 0 1 2 3 4 5 由Car_turn_flag控制,左右方向各5个档位,0停止
    //800-1000 对应档位, 由Car_turn_LR_Angle_num控制
    //1000 = -5 ,950 = -4, 900 = -3, 850 = -2, 800 = -1, 0 ,800 = 1, 850 = 2,...,1000 = 5
    Car_turn_flag -= 1;

    if(Car_turn_flag < -5)
        Car_turn_flag = -5;

    if(Car_turn_flag == 0){
        Car_turn_LR_Angle_num -= 50;
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, (CAR_ACCELERATE_DATA | CAR_DECELERATE_DATA) & 0xF0);//小车右转向复位
//        qDebug()<<"cjf"<< ((CAR_ACCELERATE_DATA | CAR_DECELERATE_DATA) & 0xF0);

        emit Car_writeRead(CAR_RIGHT_HEAD_LED_DATA, 1, 0);
        emit Car_writeRead(CAR_RIGHT_REAR_LED_DATA, 1, 0);
        emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, CAR_RIGHT_HEAD_LED | CAR_RIGHT_REAR_LED);
    }

    if(Car_turn_flag < 0){
        Car_turn_LR_Angle_num += 50;
        if(Car_turn_LR_Angle_num > 1000)
            Car_turn_LR_Angle_num = 1000;
        emit Car_writeRead(CAR_TURNLEFT_ADDR_DATA, 1, Car_turn_LR_Angle_num);
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, CAR_LEFT_DATA | CAR_ACCELERATE_DATA | CAR_DECELERATE_DATA);

        emit Car_writeRead(CAR_LEFT_HEAD_LED_DATA, 1, Car_turn_flag * -200);
        emit Car_writeRead(CAR_LEFT_REAR_LED_DATA, 1, Car_turn_flag * -200);
        emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, CAR_LEFT_HEAD_LED | CAR_LEFT_REAR_LED);
    }

    if(Car_turn_flag > 0){
        Car_turn_LR_Angle_num -= 50;
        if(Car_turn_LR_Angle_num < 800)
            Car_turn_LR_Angle_num = 800;
        emit Car_writeRead(CAR_TURNRIGHT_ADDR_DATA, 1, Car_turn_LR_Angle_num);//小车右转
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, CAR_RIGHT_DATA | CAR_ACCELERATE_DATA | CAR_DECELERATE_DATA);

        emit Car_writeRead(CAR_RIGHT_HEAD_LED_DATA, 1, Car_turn_flag * 200);
        emit Car_writeRead(CAR_RIGHT_REAR_LED_DATA, 1, Car_turn_flag * 200);
        emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, CAR_RIGHT_HEAD_LED | CAR_RIGHT_REAR_LED);
    }
}

void AICarDemo::on_turnRight_clicked()
{
    car->turnRight();

    Car_turn_flag += 1;

    if(Car_turn_flag > 5)
        Car_turn_flag = 5;

    if(Car_turn_flag == 0){
        Car_turn_LR_Angle_num -= 50;
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, (CAR_ACCELERATE_DATA | CAR_DECELERATE_DATA) & 0xF0);//小车左转向复位
//      emit Car_writeRead(CAR_COMMAND_ADDR, 1, CAR_RESET_DATA);//小车复位
        emit Car_writeRead(CAR_LEFT_HEAD_LED_DATA, 1, 0);
        emit Car_writeRead(CAR_LEFT_REAR_LED_DATA, 1, 0);
        emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, CAR_LEFT_HEAD_LED | CAR_LEFT_REAR_LED);
    }

    if(Car_turn_flag > 0){
        Car_turn_LR_Angle_num += 50;
        if(Car_turn_LR_Angle_num > 1000)
            Car_turn_LR_Angle_num = 1000;
        emit Car_writeRead(CAR_TURNRIGHT_ADDR_DATA, 1, Car_turn_LR_Angle_num);//小车右转
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, CAR_RIGHT_DATA | CAR_ACCELERATE_DATA | CAR_DECELERATE_DATA);

        emit Car_writeRead(CAR_RIGHT_HEAD_LED_DATA, 1, Car_turn_flag * 200);
        emit Car_writeRead(CAR_RIGHT_REAR_LED_DATA, 1, Car_turn_flag * 200);
        emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, CAR_RIGHT_HEAD_LED | CAR_RIGHT_REAR_LED);
    }

    if(Car_turn_flag < 0){
        Car_turn_LR_Angle_num -= 50;
        if(Car_turn_LR_Angle_num < 800)
            Car_turn_LR_Angle_num = 800;
        emit Car_writeRead(CAR_TURNLEFT_ADDR_DATA, 1, Car_turn_LR_Angle_num);
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, CAR_LEFT_DATA | CAR_ACCELERATE_DATA | CAR_DECELERATE_DATA);
//        qDebug()<<"cjf RL1 num"<<Car_turnLeft_Angle_num <<"time"<<Car_turn_flag;
        emit Car_writeRead(CAR_LEFT_HEAD_LED_DATA, 1, Car_turn_flag * -200);
        emit Car_writeRead(CAR_LEFT_REAR_LED_DATA, 1, Car_turn_flag * -200);
        emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, CAR_LEFT_HEAD_LED | CAR_LEFT_REAR_LED);
    }
}

void AICarDemo::on_accelerate_clicked()
{
    car->accelerate();
    //-5 -4 -3 -2 -1 0 1 2 3 4 5 由Car_turn_flag控制,前后方向各5个档位,0停止
    //500-900 对应档位, 由Car_turn_LR_Angle_num控制
    //900 = -5 ,800 = -4, 700 = -3, 600 = -2, 500 = -1, 0 ,500 = 1, 600 = 2,...,900 = 5
    Car_AD_flag -= 1;

    if(Car_AD_flag < -5)
        Car_AD_flag = -5;

    if(Car_AD_flag == 0){
        Car_AD_Rate_num -= 10;
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, (CAR_LEFT_DATA | CAR_RIGHT_DATA) & 0x0F);//小车后退复位

        emit Car_writeRead(CAR_LEFT_REAR_LED_DATA, 1, 0);
        emit Car_writeRead(CAR_RIGHT_REAR_LED_DATA, 1, 0);
        emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, CAR_LEFT_REAR_LED | CAR_RIGHT_REAR_LED);
    }

    if(Car_AD_flag < 0){
        Car_AD_Rate_num += 10;
        if(Car_AD_Rate_num > 550)
            Car_AD_Rate_num = 550;
        emit Car_writeRead(CAR_ACCELERATE_ADDR_DATA, 1, Car_AD_Rate_num);
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, CAR_ACCELERATE_DATA | CAR_LEFT_DATA | CAR_RIGHT_DATA);
//        qDebug()<<"cjf LL1 num"<<Car_turnLeft_Angle_num <<"time"<<Car_turn_flag;
    }

    if(Car_AD_flag > 0){
        Car_AD_Rate_num -= 10;
        if(Car_AD_Rate_num < 500)
            Car_AD_Rate_num = 500;
        emit Car_writeRead(CAR_DECELERATE_ADDR_DATA, 1, Car_AD_Rate_num);//小车后退
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, CAR_DECELERATE_DATA | CAR_LEFT_DATA | CAR_RIGHT_DATA);

        emit Car_writeRead(CAR_LEFT_REAR_LED_DATA, 1, Car_AD_flag * 200);
        emit Car_writeRead(CAR_RIGHT_REAR_LED_DATA, 1, Car_AD_flag * 200);
        emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, CAR_LEFT_REAR_LED | CAR_RIGHT_REAR_LED);
    }
}

void AICarDemo::on_decelerate_clicked()
{
    car->decelerate();
    Car_AD_flag += 1;

    if(Car_AD_flag > 5)
        Car_AD_flag = 5;

    if(Car_AD_flag == 0){
        Car_AD_Rate_num -= 10;
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, (CAR_LEFT_DATA | CAR_RIGHT_DATA) & 0x0F);//小车后退复位
    }

    if(Car_AD_flag > 0){
        Car_AD_Rate_num += 10;
        if(Car_AD_Rate_num > 550)
            Car_AD_Rate_num = 550;
        emit Car_writeRead(CAR_DECELERATE_ADDR_DATA, 1, Car_AD_Rate_num);
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, CAR_DECELERATE_DATA | CAR_LEFT_DATA | CAR_RIGHT_DATA);
//        qDebug()<<"cjf LL1 num"<<Car_turnLeft_Angle_num <<"time"<<Car_turn_flag;
        emit Car_writeRead(CAR_LEFT_REAR_LED_DATA, 1, Car_AD_flag * 200);
        emit Car_writeRead(CAR_RIGHT_REAR_LED_DATA, 1, Car_AD_flag * 200);
        emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, CAR_LEFT_REAR_LED | CAR_RIGHT_REAR_LED);
    }

    if(Car_AD_flag < 0){
        Car_AD_Rate_num -= 10;
        if(Car_AD_Rate_num < 500)
            Car_AD_Rate_num = 500;
        emit Car_writeRead(CAR_ACCELERATE_ADDR_DATA, 1, Car_AD_Rate_num);//小车后退
        emit Car_writeRead(CAR_COMMAND_ADDR, 1, CAR_ACCELERATE_DATA | CAR_LEFT_DATA | CAR_RIGHT_DATA);

    }
}

void AICarDemo::on_connect_clicked()
{
    emit Car_connect();
}
void AICarDemo::Car_change_connet(bool data)
{
    if(data == false)
    {
        ui->connect->setText(tr("Connect"));
        car_state->stop();
    }
    if(data == true){
        ui->connect->setText(tr("Disconnect"));
        car_state->start();

    }

}
void AICarDemo::on_Car_reset_clicked()
{
    Car_turn_flag = 0;
    Car_AD_flag = 0;
    Car_turn_LR_Angle_num = 750;
    Car_AD_Rate_num = 450;
    car->reset();
    emit Car_writeRead(CAR_COMMAND_ADDR, 1, 0);//小车复位
    emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, 0);//小车LED复位

}
void AICarDemo::Car_state_data(){
    emit Car_read(0x0010,9);
}
void AICarDemo::Car_read_data(int address, int data)
{
    if(address == 0x0010)
    {
        accx = (qint16)data*2*9.8/32768.0;
        ui->Acc_X->setText(QString::number((qint16)data*2*9.8/32768.0,'d',2));
    }
    if(address == 0x0011)
    {
        accy = (qint16)data*2*9.8/32768.0;
        ui->Acc_Y->setText(QString::number((qint16)data*2*9.8/32768.0,'d',2));
    }
    if(address == 0x0012)
    {
        accz = (qint16)data*2*9.8/32768.0;
        ui->Acc_Z->setText(QString::number((qint16)data*2*9.8/32768.0,'d',2));
    }
    if((address ==0x0010) || (address == 0x0011) || (address == 0x0012))
    {
        if(accx!=0 && accy!=0 &&accz!=0){
        ui->Angle_X->setText(QString::number(atan(accy /accz) * 180 / 3.14,'d', 2));
        ui->Angle_Y->setText(QString::number(atan(accx /accz) * 180 / 3.14,'d', 2));
        ui->Angle_Z->setText(QString::number(atan(accz /accy) * 180 / 3.14,'d', 2));
        }
    }
    if(address == 0x0013)
    {
        ui->temp_data->setText(QString::number((qint16)data/340.00+36.53,'d', 2));
    }
    if(address == 0x0014)
    {
        ui->Gyro_X->setText(QString::number((qint16)data*2000/32768.0,'d',2));
    }
    if(address == 0x0015)
    {
        ui->Gyro_Y->setText(QString::number((qint16)data*2000/32768.0,'d',2));
    }
    if(address == 0x0016)
    {
        ui->Gyro_Z->setText(QString::number((qint16)data*2000/32768.0,'d',2));
    }
    if(address == 0x0018)
    {
        if(data < 50 )
            radar_data = "有人";
        else
            radar_data = "无人";
        radar_data = radar_data + ",  距离cm:" + QString::number(data);
        ui->radar_data->setText(radar_data);
    }
}
QImage AICarDemo::Mat2QImage(const Mat &mat)
{
    switch (mat.type())
    {
        // 8-bit, 4 channel
        case CV_8UC4:
        {
            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
            return image;
        }

        // 8-bit, 3 channel
        case CV_8UC3:
        {
            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            return image.rgbSwapped();
        }

        // 8-bit, 1 channel
        case CV_8UC1:
        {
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
            QImage image(mat.data, mat.cols, mat.rows, int(mat.step), QImage::Format_Grayscale8);
#else
            QVector<QRgb> sColorTable;
            if (sColorTable.isEmpty())
            {
                sColorTable.resize( 256 );

                for ( int i =0; i <256; ++i )
                {
                    sColorTable[i] = qRgb( i, i, i );
                }
            }

            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8 );
            image.setColorTable(sColorTable);
#endif

            return image;
        }

        // wrong
        default:
            qDebug() << "ERROR: Mat could not be converted to QImage.";
            break;
    }
    return QImage();
}

Mat AICarDemo::QImage2Mat(const QImage& image)
{
    cv::Mat mat,mat_out;    //如果把mat_out变更为mat，那么参数image的r/b被调换。即使被const修饰，依然被更改，比较诡异。参数变为值传递，也依然被更改。
    switch (image.format())
    {
    case QImage::Format_RGB32:                              //一般Qt读入本地彩色图后为此格式
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat_out, cv::COLOR_BGRA2BGR);     //转3通道，OpenCV一般用3通道的
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat_out, cv::COLOR_RGB2BGR);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat_out;
}
void AICarDemo::Car_videoDisplay(const QImage image)
{
    QImage image1 = image.copy();
    image_tmp = image1.mirrored(false, false);

    Mat img = this->QImage2Mat(image_tmp);
    Mat img1 = img;
    if(ui->FACEButton->isChecked()){
        img1 = FaceRecognition(img);//人脸识别
//    img1 = faces->face_recognition(img);
    }else if(ui->RGYButton->isChecked()){
        img1 = rgy_light_identification(img);//红绿黄交通灯识别
    }else if(ui->LPRButton->isChecked()){
        img1 = plr->test_mtcnn_plate(img);//车牌识别

        ui->LPR->setText(QString::fromStdString(plr->LPR_Data));
    }
    QImage qimg = this->Mat2QImage(img1);

    QPixmap pixmap = QPixmap::fromImage(qimg);
    ui->Car_videoDisplay->setPixmap(pixmap.scaled(ui->Car_videoDisplay->size(),Qt::IgnoreAspectRatio));//, Qt::SmoothTransformation 保持比例
}

void AICarDemo::Car_videoDisplay1()
{
    if(!image_tmp.isNull()){
//        if(ui->RGYButton->isChecked()){
//            rgy_light_identification();//红绿黄交通灯识别
//        }
//        if(ui->LPRButton->isChecked()){
//            license_plate_recognition();//车牌识别
//        }
    }
}
void AICarDemo::Car_traffic_light_Play()
{
     rgy_light_play_flag = 0;
}
Mat AICarDemo::rgy_light_identification(const Mat &mat)
{
    ui->red_light->setStyleSheet("");
    ui->green_light->setStyleSheet("");
    ui->yellow_light->setStyleSheet("");

    QImage qImage;
    Mat src,src1, mout, hsv;
    vector<Vec3f>  circles;  //创建一个容器保存检测出来的几个圆
    src=mat;
    cv::resize(src,src,Size(320, 240));

    medianBlur(src, mout, 7);//中值滤波/百分比滤波器
    cvtColor(mout, mout, COLOR_BGR2GRAY);//转化为灰度图

    //HoughCircles(mout, circles, HOUGH_GRADIENT, 1, 10, 100, 35, 15, 60);//霍夫变换圆检测
    HoughCircles(mout, circles, HOUGH_GRADIENT, 1, 10, 100, 35, 15, 60);//霍夫变换圆检测
    Scalar circleColor = Scalar(0,0,255);//圆形的边缘颜色
    //Scalar centerColor = Scalar(0, 0, 255);//圆心的颜色
    for (size_t i = 0; i < circles.size(); i++) {
        Vec3f c = circles[i];
//        circle(src, Point(c[0], c[1]),c[2], circleColor, 2, LINE_AA);//画边缘
        //circle(src, Point(c[0], c[1]), 2, centerColor, 2, LINE_AA);//画圆心

        Point center(c[0], c[1]);
        int radius = c[2];
        Mat split_circle(src.rows, src.cols, src.type(), Scalar(0, 0, 0));

        for (int x = 0; x < src.cols; x++)
        {
            for (int y = 0; y < src.rows; y++)
            {
                int temp = ((x - center.x) * (x - center.x) + (y - center.y) * (y - center.y));
                if (temp < (radius * radius))
                {
                    split_circle.at<Vec3b>(Point(x, y))[0] = src.at<Vec3b>(Point(x, y))[0];//b
                    split_circle.at<Vec3b>(Point(x, y))[1] = src.at<Vec3b>(Point(x, y))[1];//g
                    split_circle.at<Vec3b>(Point(x, y))[2] = src.at<Vec3b>(Point(x, y))[2];//r
                }else{//圆轮廓外
//                    split_circle.at<Vec3b>(Point(x, y))[0] = src.at<Vec3b>(Point(x, y))[0];//b
//                    split_circle.at<Vec3b>(Point(x, y))[1] = src.at<Vec3b>(Point(x, y))[1];//g
//                    split_circle.at<Vec3b>(Point(x, y))[2] = src.at<Vec3b>(Point(x, y))[2];//r
                }
            }
        }
        src1=split_circle;
        //转化为hsv模型
        cvtColor(src1, hsv, COLOR_BGR2HSV);

        Mat maskGreen, maskRed, maskYellow, green_result, red_result, yellow_result;
        int red, green, yellow;
        //根据颜色阈值分别得到掩膜
        inRange(hsv, lower_green, upper_green, maskGreen);
        inRange(hsv, lower_red, upper_red, maskRed);
        inRange(hsv, lower_yellow, upper_yellow, maskYellow);
        //用掩膜进行二值化处理，并统计识别出的像素数目
        medianBlur(maskGreen, green_result, 5);
        green = countNonZero(green_result);

        medianBlur(maskRed, red_result, 5);
        red = countNonZero(red_result);

        medianBlur(maskYellow, yellow_result, 5);
        yellow = countNonZero(yellow_result);
        qDebug()<<"rgb:"<<red<<green<<yellow;
        //颜色判决
        if ((yellow*1.0 >= 0.5)|| (red*1.0  >=0.5 )||  (green*1.0 >= 0.5))//判断红绿黄三色的像素占比是否过半
        {
            circle(src, Point(c[0], c[1]),c[2], circleColor, 2, LINE_AA);//画边缘
            if(green > red && green > yellow && green > 50){
                ui->green_light->setStyleSheet("border-image:url(:/image/res/image/green_light.png);");
                rgy_light_play_flag  |= 0x02;
            }else if(red > yellow && red >50){
                ui->red_light->setStyleSheet("border-image:url(:/image/res/image/red_light.png);");
                rgy_light_play_flag  |= 0x01;

            }else{
                if(yellow > 50){
                    ui->yellow_light->setStyleSheet("border-image:url(:/image/res/image/yellow_light.png);");
                    rgy_light_play_flag  |= 0x04;
                }
            }
        }
    }

    if(rgy_light_play_flag == 0x01){
        QSound *success = new QSound("./mp3/red_light.wav", this);
        success->play();
        rgy_light_play_flag = 8;
        car_rgy_light_play->start();
    }else if(rgy_light_play_flag == 0x02){
        QSound *success = new QSound("./mp3/green_light.wav", this);
        success->play();
        rgy_light_play_flag = 8;
        car_rgy_light_play->start();
    }else if(rgy_light_play_flag == 0x04){
        QSound *success = new QSound("./mp3/yellow_light.wav", this);
        success->play();
        rgy_light_play_flag = 8;
        car_rgy_light_play->start();
    }else if(rgy_light_play_flag==0x05 || rgy_light_play_flag==0x6 ||rgy_light_play_flag==0x7||rgy_light_play_flag==0x3){
        QSound *success = new QSound("./mp3/more_light.wav", this);
        success->play();
        rgy_light_play_flag = 8;
        car_rgy_light_play->start();
    }

    return src;
}


Mat AICarDemo::FaceRecognition(const Mat &mat)
{
        vector<Rect> faces;  //创建一个容器保存检测出来的脸
        Mat img1, gray;

        img1 = mat;
        cv::resize(img1,img1,Size(320, 240));

        cvtColor(img1, gray, COLOR_BGR2GRAY); //转换成灰度图，因为harr特征从灰度图中提取
        equalizeHist(gray,gray);  //直方图均衡行
        ccf.detectMultiScale(gray,faces,1.1,3,0,Size(50,50),Size(200,200)); //检测人脸
        for(vector<Rect>::const_iterator iter=faces.begin();iter!=faces.end();iter++)
        {
            Mat img2, m;
            img1.copyTo(img2);

            rectangle(img1,*iter,Scalar(0,0,255),2,10); //画出脸部矩形
            qDebug()<<"cjfx"<<iter->x<<"y:"<<iter->y<<"w:"<<iter->width<<"h:"<<iter->height;
cv::Rect area(iter->x-10, iter->y-10, iter->width+10, iter->height+10); //需要裁减的矩形区域
           m= img2(area);

//            img2(*iter).copyTo(m);

            QImage qimg = this->Mat2QImage(m);
            QPixmap pixmap = QPixmap::fromImage(qimg);
            ui->faces_data->setPixmap(pixmap.scaled(ui->faces_data->size(),Qt::IgnoreAspectRatio));//Qt::SmoothTransformation 保持比例
        }
        return img1;
}

void AICarDemo::on_pushButton_clicked()
{
    Mat img = this->QImage2Mat(image_tmp);
    Mat m = faces->face_recognition(img);
    m = faces->face_data;
    QImage qimg = this->Mat2QImage(m);
    QPixmap pixmap = QPixmap::fromImage(qimg);
    ui->faces_data->setPixmap(pixmap.scaled(ui->faces_data->size(),Qt::IgnoreAspectRatio));//Qt::SmoothTransformation 保持比例
}
