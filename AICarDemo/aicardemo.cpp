#include "aicardemo.h"
#include "ui_aicardemo.h"
#include <QTimer>
#include <QDebug>
#include <math.h>
#include <QSound>//声音

#include <fstream>   //文本读写


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

    car_camera_state = new QTimer(this);
    car_camera_state->setInterval(1000/20*5);
    connect(car_camera_state,SIGNAL(timeout()),this,SLOT(Car_videoDisplay1()));
    car_camera_state->start();


    car_rgy_light_play = new QTimer(this);
    car_rgy_light_play->setInterval(3000);
    connect(car_rgy_light_play,SIGNAL(timeout()),this,SLOT(Car_traffic_light_Play()));
//    car_rgy_light_play->start();
    rgy_light_play_flag = 0;

//    lower_red = Scalar(0, 100, 100);
//    upper_red = Scalar(10, 255, 255);
    lower_red = Scalar(2, 100, 100);
    upper_red = Scalar(10, 255, 255);
    lower_green = Scalar(40, 50, 50);
    upper_green = Scalar(90, 255, 255);
    lower_yellow = Scalar(15, 100, 100);
    upper_yellow = Scalar(35, 255, 255);

    this->cameraThread = camerathread;
    this->modbusThread = modbusthread;
    connect(this, SIGNAL(Car_connect()),modbusThread,SLOT(on_connect()));
    connect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Car_change_connet(bool)));

    connect(this, SIGNAL(Car_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    connect(this, SIGNAL(Car_read(int,quint16)),modbusThread,SLOT(on_read(int,quint16)));//only read
    connect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Car_read_data(int, int)));

    connect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(Car_videoDisplay(QImage)));

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
    car_camera_state->stop();
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
//    faces_flag = false;
//    ui->faceTrack->setText(tr("人脸追踪"));
    if(data == false)
    {
//        connect_flag = false;

        ui->connect->setText(tr("Connect"));
        car_state->stop();

//        ui->faceTrack->setDisabled(true);
//        ui->up->setDisabled(true);
//        ui->down->setDisabled(true);
//        ui->left->setDisabled(true);
//        ui->right->setDisabled(true);
    }
    if(data == true){
//        connect_flag = true;
//        emit Camera_writeRead(CAMERA_ADDR1, 1, H_Angle_num);
//        emit Camera_writeRead(CAMERA_ADDR2, 1, V_Angle_num);
        ui->connect->setText(tr("Disconnect"));
        car_state->start();
//        ui->faceTrack->setDisabled(false);
//        ui->up->setDisabled(false);
//        ui->down->setDisabled(false);
//        ui->left->setDisabled(false);
//        ui->right->setDisabled(false);
    }
//    emit Camera_times(faces_flag);

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

void AICarDemo::Car_videoDisplay(const QImage image)
{
    QImage image1 = image.copy();
    image_tmp=image1;
//    image_tmp = image1.mirrored(true, false);

//    QPixmap pixmap = QPixmap::fromImage(image1);
//    ui->Car_videoDisplay->setPixmap(pixmap.scaled(ui->Car_videoDisplay->size(),Qt::IgnoreAspectRatio));//, Qt::SmoothTransformation 保持比例
}

void AICarDemo::Car_videoDisplay1()
{
    if(!image_tmp.isNull()){
//        rgy_light_identification();//红绿黄交通灯识别
        license_plate_recognition();//车牌识别
    }
}
void AICarDemo::Car_traffic_light_Play()
{
     rgy_light_play_flag = 0;
}
void AICarDemo::rgy_light_identification()
{
    ui->red_light->setStyleSheet("");
    ui->green_light->setStyleSheet("");
    ui->yellow_light->setStyleSheet("");

    QImage qImage;
    Mat src,src1, mout, hsv;
    vector<Vec3f>  circles;  //创建一个容器保存检测出来的几个圆
    src=Mat(image_tmp.height(), image_tmp.width(), CV_8UC3, (void*)image_tmp.constBits(), image_tmp.bytesPerLine());
    cv::resize(src,src,Size(320, 240));

    cvtColor(src, src, COLOR_RGB2BGR);
    medianBlur(src, mout, 7);//中值滤波/百分比滤波器
    cvtColor(mout, mout, COLOR_BGR2GRAY);//转化为灰度图

    //HoughCircles(mout, circles, HOUGH_GRADIENT, 1, 10, 100, 35, 15, 60);//霍夫变换圆检测
    HoughCircles(mout, circles, HOUGH_GRADIENT, 1, 10, 100, 40, 15, 60);//霍夫变换圆检测
    Scalar circleColor = Scalar(0,0,255);//圆形的边缘颜色
    //Scalar centerColor = Scalar(0, 0, 255);//圆心的颜色
    for (size_t i = 0; i < circles.size(); i++) {
        Vec3f c = circles[i];
        circle(src, Point(c[0], c[1]),c[2], circleColor, 2, LINE_AA);//画边缘
        //circle(src, Point(c[0], c[1]), 2, centerColor, 2, LINE_AA);//画圆心
        Point center(c[0], c[1]);
        int radius = c[2];
        Mat split_circle(src.rows, src.cols, src.type(), Scalar(0, 0, 0));
        int count = 0;
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
                    count++;
                }
            }
        }
        src1=split_circle;
        //转化为hsv模型
        cvtColor(src1, hsv, COLOR_BGR2HSV);

        Mat maskGreen,maskRed,maskYellow,green_result,red_result,yellow_result;
        int red,green,yellow;
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
            if (green > red && green > yellow && green > 50){
                ui->green_light->setStyleSheet("border-image:url(:/image/res/image/green_light.png);");
                rgy_light_play_flag  |= 0x02;
            }
            else if (red > yellow && red >50){
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

    cvtColor(src,src, COLOR_BGR2RGB);
    if(src.channels() == 3)
    {
        qImage = QImage((const unsigned char*)(src.data),src.cols,src.rows,src.cols * src.channels(),
                        QImage::Format_RGB888);
    }else{
        qImage = QImage((const unsigned char*)(src.data),src.cols,src.rows,src.cols * src.channels(),
                        QImage::Format_RGB888);
    }

    QPixmap pixmap = QPixmap::fromImage(qImage);
    ui->Car_videoDisplay->setPixmap(pixmap.scaled(ui->Car_videoDisplay->size(),Qt::IgnoreAspectRatio));
}


void AICarDemo::license_plate_recognition()
{
    QImage qImage;
    Mat src,src1;

    src=Mat(image_tmp.height(), image_tmp.width(), CV_8UC3, (void*)image_tmp.constBits(), image_tmp.bytesPerLine());

    Mat img_gray;
    //cv::resize(src,src,Size(320, 240));
    cvtColor(src, src, COLOR_RGB2BGR);

    if(Get_License_ROI(src, License_ROI))//获取车牌所在ROI区域--车牌定位
    {

    }
    if (!License_ROI.mat.empty()){

        vector<License> Character_ROI;
        if(Get_Character_ROI(License_ROI, Character_ROI))//获取车牌每一个字符ROI区域
        {
            vector<int>result_index;
            if(License_Recognition(Character_ROI, result_index))
            {
                Draw_Result(src, License_ROI, Character_ROI,result_index);

                src1 = License_ROI.mat;
                cvtColor(src1,src1, COLOR_BGR2RGB);
                if(src1.channels() == 3)
                {
                    qImage = QImage((const unsigned char*)(src1.data),src1.cols,src1.rows,src1.step,
                                    QImage::Format_RGB888);
                }else{
                    qImage = QImage((const unsigned char*)(src1.data),src1.cols,src1.rows,src1.step,
                                    QImage::Format_RGB888);
                }

                QPixmap pixmap = QPixmap::fromImage(qImage);
                ui->LPR->setPixmap(pixmap.scaled(ui->LPR->size(),Qt::IgnoreAspectRatio));

            }else{
                qDebug()<<"未能识别字符";
            }
        }else{
            qDebug()<<"未能切割出字符";
        }
    }else{
        qDebug()<<"未定位到车牌位置";
    }

    cvtColor(src,src, COLOR_BGR2RGB);
    if(src.channels() == 3)
    {
        qImage = QImage((const unsigned char*)(src.data),src.cols,src.rows,src.step,
                        QImage::Format_RGB888);
    }else{
        qImage = QImage((const unsigned char*)(src.data),src.cols,src.rows,src.step,
                        QImage::Format_RGB888);
    }

    QPixmap pixmap = QPixmap::fromImage(qImage);
    ui->Car_videoDisplay->setPixmap(pixmap.scaled(ui->Car_videoDisplay->size(),Qt::IgnoreAspectRatio));

}
//获取车牌所在ROI区域--车牌定位
bool AICarDemo::Get_License_ROI(Mat src, License &License_ROI)
{
    Mat gray, dst;
    cvtColor(src, gray, COLOR_BGR2GRAY);

    //阈值滤波得到黑白图像， CV_THRESH_OTSU是自适应阈值
    Mat thresh;
    threshold(gray, thresh, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);

    //使用形态学开操作去除一些小轮廓
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat open;
    morphologyEx(thresh, open, MORPH_OPEN, kernel);

    //使用 RETR_EXTERNAL 找到最外轮廓
    vector<vector<Point>>contours;
    findContours(open, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    vector<vector<Point>>conPoly(contours.size());

    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        double peri = arcLength(contours[i], true);

        //根据面积筛选出可能属于车牌区域的轮廓
        if (area > 1000)
        {
            //使用多边形近似，进一步确定车牌区域轮廓
            approxPolyDP(contours[i], conPoly[i], 0.02*peri, true);
//            if (conPoly[i].size() == 4)
            {
                //计算矩形区域宽高比
                Rect box = boundingRect(contours[i]);

                double ratio = double(box.width) / double(box.height);
                if (ratio > 2 && ratio < 4)
                {
                    //截取ROI区域
                    Rect rect = boundingRect(contours[i]);
                    License_ROI = { src(rect),rect };
                }
            }
        }
    }

    if (License_ROI.mat.empty())
    {
        return false;
    }
    return true;
}
//获取车牌每一个字符ROI区域
bool AICarDemo::Get_Character_ROI(License &License_ROI, vector<License>&Character_ROI)
{
    Mat gray;
    cvtColor(License_ROI.mat, gray, COLOR_BGR2GRAY);

    Mat thresh;
    threshold(gray, thresh, 0, 255, THRESH_BINARY | THRESH_OTSU);

    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat close;
    morphologyEx(thresh, close, MORPH_CLOSE, kernel);

    vector<vector<Point>>contours;
    findContours(close, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        //由于我们筛选出来的轮廓是无序的，故后续我们需要将字符重新排序
        if (area > 200)
        {
            Rect rect = boundingRect(contours[i]);
            //计算外接矩形宽高比
            double ratio = double(rect.height) / double(rect.width);
            if (ratio > 1)
            {
                Mat roi = License_ROI.mat(rect);
                cv::resize(roi, roi, Size(50, 100), 1, 1, INTER_LINEAR);
                Character_ROI.push_back({ roi ,rect });
            }
        }
    }

    //将筛选出来的字符轮廓 按照其左上角点坐标从左到右依次顺序排列
    //冒泡排序
    qDebug()<<"cjf1   "<<Character_ROI.size();
    if(Character_ROI.size() > 6){
    for (size_t i = 0; i < Character_ROI.size()-1; i++)
    {
        for (size_t j = 0; j < Character_ROI.size() - 1 - i; j++)
        {
            if (Character_ROI[j].rect.x > Character_ROI[j + 1].rect.x)
            {
                License temp = Character_ROI[j];
                Character_ROI[j] = Character_ROI[j + 1];
                Character_ROI[j + 1] = temp;
            }
        }
    }}
//    for(int i=0;i<7;i++)
//         cv::imshow(QString("a%1").arg(i).toLocal8Bit().data(),Character_ROI[i].mat);
    if (Character_ROI.size() != 7)
    {
        return false;
    }
    return true;
}
//计算像素点个数
int pixCount(Mat image)
{
    int count = 0;
    if (image.channels() == 1)
    {
        for (int i = 0; i < image.rows; i++)
        {
            for (int j = 0; j < image.cols; j++)
            {
                if (image.at<uchar>(i, j) == 0)
                {
                    count++;
                }
            }
        }

        return count;
    }
    else
    {
        return -1;
    }
}
//读取文件  图片
bool Read_Data(string filename,vector<Mat>&dataset)
{
    vector<String>imagePathList;
    glob(filename, imagePathList);
    if (imagePathList.empty())return false;

    for (int i = 0; i < imagePathList.size(); i++)
    {
        Mat image = imread(imagePathList[i]);
        resize(image, image, Size(50, 100), 1, 1, INTER_LINEAR);
        dataset.push_back(image);
    }

    return true;
}

//识别车牌字符
bool AICarDemo::License_Recognition(vector<License>&Character_ROI, vector<int>&result_index)
{
    string filename = "data/";
    vector<Mat>dataset;
    if (!Read_Data(filename, dataset)) return false;

    for (int i = 0; i < Character_ROI.size(); i++)
    {
        Mat roi_gray;
        cvtColor(Character_ROI[i].mat, roi_gray, COLOR_BGR2GRAY);

        Mat roi_thresh;
        threshold(roi_gray, roi_thresh, 0, 255, THRESH_BINARY | THRESH_OTSU);

        int minCount = 20;
        int index = 0;
        for (int j = 0; j < dataset.size(); j++)
        {
            Mat temp_gray;
            cvtColor(dataset[j], temp_gray, COLOR_BGR2GRAY);

            Mat temp_thresh;
            threshold(temp_gray, temp_thresh, 0, 255, THRESH_BINARY | THRESH_OTSU);

            //计算两张图片的像素差，以此判断两张图片是否相同
            Mat dst;
            absdiff(roi_thresh, temp_thresh, dst);

            int count = pixCount(dst);
            if (count > minCount)//如果模板是黑字count < 100000
            {
                minCount = count;
                index = j;
            }
        }

        result_index.push_back(index);
    }
    return true;
}

//显示最终效果
bool AICarDemo::Draw_Result(Mat src, License &License_ROI, vector<License>&Character_ROI,vector<int>&result_index)
{
    rectangle(src, License_ROI.rect, Scalar(0, 255, 0), 2);

    for (size_t i = 0; i < Character_ROI.size(); i++)
    {
        qDebug()<<"cjf Draw "<<i<<result_index[i];
    }
    return true;
}

