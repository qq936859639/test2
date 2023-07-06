#include "aicardemo.h"
#include "ui_aicardemo.h"
#include <QTimer>
#include <QDebug>
#include <math.h>
#include <QSound>//声音
#include <QFile>
//#include <fstream>   //文本读写
#include <QMovie>

enum ModbusConnection {
    Serial,
    Tcp
};

AICarDemo::AICarDemo(QWidget *parent, CameraThread *camerathread, ModbusThread *modbusthread) :
    QWidget(parent),
    ui(new Ui::AICarDemo)
{
    ui->setupUi(this);
//    setAttribute(Qt::WA_DeleteOnClose);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(-500+180, -500+174, 720, 530);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    car = new Car();
    car->setScale(0.5);//小车缩小0.5倍
    scene->addItem(car);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
    ui->graphicsView->setStyleSheet("border-image:url(:/image/res/image/car_map.png);");

    QRectF rec(-500+180, -500+174, 720-1, 530-1);
    QLineF topLine(rec.topLeft(),rec.topRight());
    QLineF leftLine(rec.topLeft(),rec.bottomLeft());
    QLineF rightLine(rec.topRight(),rec.bottomRight());
    QLineF bottomLine(rec.bottomLeft(),rec.bottomRight());
    QPen mypen(Qt::red);
    scene->addLine(topLine,mypen);
    scene->addLine(leftLine,mypen);
    scene->addLine(rightLine,mypen);
    scene->addLine(bottomLine,mypen);

    /*防碰撞线 add cjf*/
    QPen mypen_car(Qt::white);
    scene->addLine(-200, -275, 250, -275, mypen_car);//上边界限
    scene->addLine(-75, 30, 130, 30, mypen_car);//下边界限

    scene->addLine(-75, -200, -75, -100, mypen_car);//停车场左边界限1
    scene->addLine(-75, -30, -75, 30, mypen_car);//停车场左边界限2

    scene->addLine(-90+220, -200, -90+220, -100, mypen_car);//停车场右边界限1
    scene->addLine(-90+220, -30, -90+220, 30, mypen_car);//停车场右边界限2

    scene->addLine(-75, -170, -30, -170, mypen_car);//停车场上边界限1
    scene->addLine(30, -170, -90+220, -170, mypen_car);//停车场上边界限2

    scene->addLine(-230, -100, -75, -100, mypen_car);//停车场左边上边界限1
    scene->addLine(-220, -30, -75, -30, mypen_car);//停车场左边下边界限2
    scene->addLine(130, -100, 300, -100, mypen_car);//停车场右边上边界限1
    scene->addLine(130, -30, 300, -30, mypen_car);//停车场右边下边界限2

    scene->addLine(-230, -200, -75, -200, mypen_car);//市政厅上边界限
    scene->addLine(-230, -200, -230, -100, mypen_car);//市政厅左边界限

    scene->addLine(130, -200, 280, -200, mypen_car);//小区上边界限
    scene->addLine(280, -200, 300, -100, mypen_car);//小区右边界限

    scene->addLine(-220, -30, -220, 110, mypen_car);//体育馆左边界限
    scene->addLine(-220, 110, 300, 110, mypen_car);//体育馆下边界限

    scene->addLine(300, -30, 300, 110, mypen_car);//商场右边界限
    /*end*/

    ui->townhall->setCheckable(true);
    ui->mall->setCheckable(true);
    ui->school->setCheckable(true);
    ui->gym->setCheckable(true);

    car_play_flag = 0;
    ul_play_flag = 0;
    Car_END_flag = 0;

    Car_state_flag = 0;
    Car_state1_flag = 0;
    Car_state2_flag = 0;

    car_state = new QTimer(this);
    car_state->setInterval(500);
    connect(car_state,SIGNAL(timeout()),this,SLOT(Car_state_data()));

    video_play = new QTimer(this);
    video_play->setInterval(3000);
    connect(video_play,SIGNAL(timeout()),this,SLOT(Car_traffic_light_Play()));
    rgy_light_play_flag = 0;

    AutoPilot = new QTimer(this);
    AutoPilot->setInterval(1000);
    connect(AutoPilot,SIGNAL(timeout()),this,SLOT(AutoPilotSystem()));

//    lower_red = Scalar(2, 100, 100);
    lower_red = Scalar(0, 100, 100);
    upper_red = Scalar(10, 255, 255);
    lower_green = Scalar(40, 50, 50);
    upper_green = Scalar(90, 255, 255);
    lower_yellow = Scalar(15, 100, 100);
    upper_yellow = Scalar(35, 255, 255);

    this->cameraThread = camerathread;
    this->modbusThread = modbusthread;

    connect(this, SIGNAL(Car_connect(QString)),modbusThread,SLOT(on_connect(QString)));
    connect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Car_change_connet(bool)));

    connect(this, SIGNAL(Car_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    connect(this, SIGNAL(Car_read(int,quint16)),modbusThread,SLOT(on_read(int,quint16)));//only read
    connect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Car_read_data(int, int)));

    connect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(Car_videoDisplay(QImage)));
    plr = new PLR();

    string xmlPath="./data/haarcascade_frontalface_default.xml";
    if(!ccf.load(xmlPath))   //加载训练文件
    {
        perror("不能加载指定的xml文件");
    }
    ui->tabWidget->setDisabled(true);
    ui->Car_reset->setDisabled(true);

    get_ip();

    Car_rplidar_flag = 0;
    Car_rplidar_flag_stop = 0;

    pm = new QMovie(":/image/res/image/rplidar_0.gif");
    ui->rplidar_0->setScaledContents(true);
    ui->rplidar_0->setMovie(pm);

    connect(this, SIGNAL(Car_radar(int,int,int)),this,SLOT(Read_Radar(int,int,int)));//获取雷达数据
    connect(modbusThread, SIGNAL(rplidar_read(int,int,int)),this,SLOT(Read_Radar(int,int,int)));

//    if(ui->scene->text()=="场景关"){
//            on_scene_clicked();
//    }

//    on_connectType_currentIndexChanged(0);
    hidmic = new HIDMICDEMO;
    if(hidmic->hidmic_init()==0){
        ui->speech->setDisabled(false);
        hidmic_open_flag = 1;
    }else{
        ui->speech->setDisabled(true);
        hidmic_open_flag = 0;
    }

//MQTT
    SmartCar_MQTT_init();
}

AICarDemo::~AICarDemo()
{
    delete ui;
}
void AICarDemo::closeEvent(QCloseEvent *event)
{
    if(ui->connect->text()=="Disconnect"){
        Car_Reset();
        emit Car_connect(ui->lineEdit->text());
    }
    Uart_Close();//关闭串口
    disconnect(this, SIGNAL(Car_connect(QString)),modbusThread,SLOT(on_connect(QString)));
    disconnect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Car_change_connet(bool)));

    disconnect(this, SIGNAL(Car_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    disconnect(this, SIGNAL(Car_read(int,quint16)),modbusThread,SLOT(on_read(int,quint16)));//only read
    disconnect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Car_read_data(int, int)));

    disconnect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(Car_videoDisplay(QImage)));
    disconnect(AutoPilot,SIGNAL(timeout()),this,SLOT(AutoPilotSystem()));

    pm->stop();
    disconnect(this, SIGNAL(Car_radar(int,int,int)),this,SLOT(Read_Radar(int,int,int)));//获取雷达数据
    disconnect(modbusThread, SIGNAL(rplidar_read(int,int,int)),this,SLOT(Read_Radar(int,int,int)));
    modbusThread->modbus_rplidar_stopMotor();

    if(hidmic_open_flag)
        hidmic->hidmic_close();//关闭麦克风
}
void AICarDemo::Open_Radar()
{
//    on_radar_clicked();
//    on_ul_clicked();
    on_rplidar_clicked();
}
void AICarDemo::Read_Radar(int mi_data,int ul_data,int la_radar)
{
    //毫米波雷达数据处理
    if(mi_data!=-1){
        int data = mi_data;
        if(Car_millimeter_radar == 1){
            if(0 < data && data < 30){
                if(ul_play_flag == 0){
                    QSound *success = new QSound("./mp3/radar_obstacles.wav", this);
                    success->play();
                    ul_play_flag = 1;
                    video_play->start();
                }
                if(Car_END_flag==0 && Car_AD_flag > 0 && Car_state_flag == 0){
                    Car_Reset();
                    Car_state_flag = 1;
                }
            }
            else{
                if(0 > Car_AD_flag){
                    Car_state_flag = 0;
                }
                if(Car_END_flag==0 && Car_state_flag == 1){
                    on_accelerate_clicked();
                    Car_state_flag = 0;
                }
            }
            radar_data = QString::number(data);
            ui->radar_data->setText(radar_data);
        }
    }
    //超声波雷达处理
    if(ul_data!= -1){
        int temp_data = ul_data;
        ui->ultrasound_data->setText(QString::number(temp_data/10.0));
        if(temp_data <240)
        {
            //ui->ultrasound_data->setText(tr("数据无效"));
        }else if (240 < temp_data && temp_data<500){
            if(ul_play_flag == 0){
                QSound *success = new QSound("./mp3/ur_obstacles.wav", this);
                success->play();
                ul_play_flag = 1;
                video_play->start();
            }
            if(Car_END_flag==0 && Car_AD_flag < 0 && Car_state1_flag == 0){
                Car_Reset();
                Car_state1_flag = 1;
            }
        }else if(temp_data > 500){
            if(Car_AD_flag > 0){
                Car_state1_flag = 0;
            }

            if(Car_END_flag==0 && Car_state1_flag == 1){
                on_decelerate_clicked();
                Car_state1_flag = 0;
            }
        }
    }
    //激光雷达处理
    if(la_radar!=-1){
        if(Car_rplidar_flag == la_radar&& la_radar != 0)
        {
            return;
        }
        if(Car_rplidar_flag !=0)
        {
            ui->rplidar_1->setStyleSheet("");
            ui->rplidar_2->setStyleSheet("");
            ui->rplidar_3->setStyleSheet("");
            ui->rplidar_4->setStyleSheet("");
            Car_rplidar_flag_stop = 0;
        }
        Car_rplidar_flag = la_radar;

        if(la_radar == 0x00 &&Car_rplidar_flag_stop == 0)//周围无障碍物
        {
            Car_rplidar_flag_stop = 1;
            ui->rplidar_1->setStyleSheet("");
            ui->rplidar_2->setStyleSheet("");
            ui->rplidar_3->setStyleSheet("");
            ui->rplidar_4->setStyleSheet("");
            if(Car_state2_flag == 1&&Car_END_flag==0){
                on_accelerate_clicked();
                Car_state2_flag = 0;
            }
            if(Car_state2_flag == 2&&Car_END_flag==0){
                on_decelerate_clicked();
                Car_state2_flag = 0;
            }
            if(Car_END_flag==0){
//                Car_turn_flag = 0;//左右方向复位
//                on_turnLeft_clicked();
//                on_turnRight_clicked();
            }
        }else if(la_radar == 0x01)//前
        {
            ui->rplidar_1->setStyleSheet("border-image:url(:/image/res/image/rplidar_1.png);");
            if(Car_AD_flag > 0&&Car_END_flag==0){
                Car_Reset();
                Car_state2_flag = 1;
            }
        }else if(la_radar == 0x02){//右
            ui->rplidar_2->setStyleSheet("border-image:url(:/image/res/image/rplidar_2.png);");
            if(Car_AD_flag != 0&&Car_END_flag==0){
                on_turnLeft_clicked();
            }
        }else if(la_radar == 0x04){//后
            ui->rplidar_3->setStyleSheet("border-image:url(:/image/res/image/rplidar_3.png);");
            if(Car_AD_flag < 0&&Car_END_flag==0){
                Car_Reset();
                Car_state2_flag = 2;
            }
        }else if(la_radar == 0x08){//左
            ui->rplidar_4->setStyleSheet("border-image:url(:/image/res/image/rplidar_4.png);");
            if(Car_AD_flag != 0&&Car_END_flag==0){
                on_turnRight_clicked();
            }
        }else if(la_radar == 0x03){//前右
            if(Car_AD_flag > 0&&Car_END_flag==0)
                on_turnLeft_clicked();
            ui->rplidar_1->setStyleSheet("border-image:url(:/image/res/image/rplidar_1.png);");
            ui->rplidar_2->setStyleSheet("border-image:url(:/image/res/image/rplidar_2.png);");
        }else if(la_radar == 0x06){//右后
            if(Car_AD_flag < 0&&Car_END_flag==0)
                on_turnLeft_clicked();
            ui->rplidar_2->setStyleSheet("border-image:url(:/image/res/image/rplidar_2.png);");
            ui->rplidar_3->setStyleSheet("border-image:url(:/image/res/image/rplidar_3.png);");
        }else if(la_radar == 0x0C){//后左
            if(Car_AD_flag < 0&&Car_END_flag==0)
                on_turnRight_clicked();
            ui->rplidar_3->setStyleSheet("border-image:url(:/image/res/image/rplidar_3.png);");
            ui->rplidar_4->setStyleSheet("border-image:url(:/image/res/image/rplidar_4.png);");
        }else if(la_radar == 0x09){//左前
            if(Car_AD_flag > 0&&Car_END_flag==0)
                on_turnRight_clicked();
            ui->rplidar_4->setStyleSheet("border-image:url(:/image/res/image/rplidar_4.png);");
            ui->rplidar_1->setStyleSheet("border-image:url(:/image/res/image/rplidar_1.png);");
        }else if(la_radar == 0x0B){//左前右
            ui->rplidar_4->setStyleSheet("border-image:url(:/image/res/image/rplidar_4.png);");
            ui->rplidar_1->setStyleSheet("border-image:url(:/image/res/image/rplidar_1.png);");
            ui->rplidar_2->setStyleSheet("border-image:url(:/image/res/image/rplidar_2.png);");
        }else if(la_radar == 0x0A){//左右
            ui->rplidar_2->setStyleSheet("border-image:url(:/image/res/image/rplidar_2.png);");
            ui->rplidar_4->setStyleSheet("border-image:url(:/image/res/image/rplidar_4.png);");
        }else if(la_radar == 0x05){//前后
            ui->rplidar_1->setStyleSheet("border-image:url(:/image/res/image/rplidar_1.png);");
            ui->rplidar_3->setStyleSheet("border-image:url(:/image/res/image/rplidar_3.png);");
        }else if(la_radar == 0x0F){//前右后左
            if(Car_END_flag==0)
                Car_Reset();
            ui->rplidar_1->setStyleSheet("border-image:url(:/image/res/image/rplidar_1.png);");
            ui->rplidar_2->setStyleSheet("border-image:url(:/image/res/image/rplidar_2.png);");
            ui->rplidar_3->setStyleSheet("border-image:url(:/image/res/image/rplidar_3.png);");
            ui->rplidar_4->setStyleSheet("border-image:url(:/image/res/image/rplidar_4.png);");
        }else if(la_radar == 0x0E){//右后左
            ui->rplidar_2->setStyleSheet("border-image:url(:/image/res/image/rplidar_2.png);");
            ui->rplidar_3->setStyleSheet("border-image:url(:/image/res/image/rplidar_3.png);");
            ui->rplidar_4->setStyleSheet("border-image:url(:/image/res/image/rplidar_4.png);");
        }else if(la_radar == 0x07){//前右后
            ui->rplidar_1->setStyleSheet("border-image:url(:/image/res/image/rplidar_1.png);");
            ui->rplidar_2->setStyleSheet("border-image:url(:/image/res/image/rplidar_2.png);");
            ui->rplidar_3->setStyleSheet("border-image:url(:/image/res/image/rplidar_3.png);");
        }else if(la_radar == 0x0D){//前左后
            ui->rplidar_1->setStyleSheet("border-image:url(:/image/res/image/rplidar_1.png);");
            ui->rplidar_3->setStyleSheet("border-image:url(:/image/res/image/rplidar_3.png);");
            ui->rplidar_4->setStyleSheet("border-image:url(:/image/res/image/rplidar_4.png);");
        }
    }
}
void AICarDemo::Close_Radar()
{
    if(ui->radar->text()=="关")//毫米波雷达
    {
        Car_millimeter_radar = 0;
        ui->radar->setText(tr("开"));
    }
    if(ui->ul->text()=="关")//超声波雷达
    {
        Uart_Close();
        ui->ul->setText(tr("开"));
    }
    if(ui->rplidar->text()=="关")//激光雷达
    {
        pm->stop();
        ui->rplidar->setText(tr("开"));
    }
}
void AICarDemo::on_turnLeft_clicked()
{
    car->turnLeft();

    //-5 -4 -3 -2 -1 0 1 2 3 4 5 由Car_turn_flag控制,左右方向各5个档位,0停止
    Car_TURN_flag -= 1;

    if(Car_TURN_flag < -5)
        Car_TURN_flag = -5;

    emit Car_writeRead(CAR_TURN_ADDR_DATA, 1, Car_TURN_flag);
}

void AICarDemo::on_turnRight_clicked()
{
    car->turnRight();

    Car_TURN_flag += 1;

    if(Car_TURN_flag > 5)
        Car_TURN_flag = 5;

    emit Car_writeRead(CAR_TURN_ADDR_DATA, 1, Car_TURN_flag);
}

void AICarDemo::on_accelerate_clicked()
{
    car->accelerate();

    //-5 -4 -3 -2 -1 0 1 2 3 4 5 由Car_AD_flag控制,前后方向各5个档位,0停止
    Car_AD_flag += 1;

    if(Car_AD_flag > 5)
        Car_AD_flag = 5;

    emit Car_writeRead(CAR_POWER_ADDR_DATA, 1, Car_AD_flag);
}

void AICarDemo::on_decelerate_clicked()
{
    car->decelerate();
    Car_AD_flag -= 1;

    if(Car_AD_flag < -5)
        Car_AD_flag = -5;

    emit Car_writeRead(CAR_POWER_ADDR_DATA, 1, Car_AD_flag);
}

void AICarDemo::on_connect_clicked()
{
    if(ui->connect->text()=="Disconnect")
        Car_Reset();
    emit Car_connect(ui->lineEdit->text());
}
void AICarDemo::Car_change_connet(bool data)
{
    if(data == false)
    {
        ui->connect->setText(tr("Connect"));
        ui->tabWidget->setDisabled(true);
        ui->Car_reset->setDisabled(true);
        car_state->stop();
        Close_Radar();
        ui->frame->setVisible(true);

        ui->Car_GPSMap->setStyleSheet("border-image:url(:/image/res/image/car_gpsmap_off.png);");
    }
    if(data == true){
        ui->connect->setText(tr("Disconnect"));
        ui->tabWidget->setDisabled(false);
        ui->Car_reset->setDisabled(false);
        Car_Reset();
        car_state->start();
        Open_Radar();
        ui->frame->setVisible(false);
        save_ip();

        ui->Key_SmartCar->setCheckable(true);
        on_Key_SmartCar_clicked();

        ui->Car_GPSMap->setStyleSheet("border-image:url(:/image/res/image/car_gpsmap.png);");
    }
}
void AICarDemo::get_ip()
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
void AICarDemo::save_ip()
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
void AICarDemo::Car_Reset()
{
    Car_TURN_flag = 0;
    Car_AD_flag = 0;

    car->reset();

    emit Car_writeRead(CAR_TURN_ADDR_DATA, 1, 0);//小车转向电机复位
    emit Car_writeRead(CAR_POWER_ADDR_DATA, 1, 0);//小车动力电机复位
    emit Car_writeRead(CAR_COMMAND_LED_ADDR, 1, 0);//小车LED自动联动电机
}
void AICarDemo::on_Car_reset_clicked()
{
    Car_Reset();

    ui->scene->setDisabled(false);

    car->reset();
    car->resetTransform();  //car->resetMatrix();
    car->setPos(0,0);       //初始化小车位置

    car_face_flag = 0;
    car_lpr_flag = 0;
    car_play_flag = 0;
    ul_play_flag = 0;
    Car_END_flag = 0;
    rgy_light_play_flag = 0;
    Car_state_flag = 0;
    Car_state1_flag = 0;
    Car_rplidar_flag_stop = 0;

    ui->townhall->setEnabled(true);
    ui->school->setEnabled(true);
    ui->gym->setEnabled(true);
    ui->mall->setEnabled(true);
    ui->townhall->setChecked(false);
    ui->school->setChecked(false);
    ui->gym->setChecked(false);
    ui->mall->setChecked(false);
    on_townhall_clicked();
    on_school_clicked();
    on_gym_clicked();
    on_mall_clicked();

    plr->LPR_Data="";
    ui->LPR->clear();
    ui->faces_data->clear();
    AutoPilot->stop();

//    ui->car_pole_1->setVisible(true);
//    ui->car_pole_2->setVisible(true);
//    ui->car_pole_3->setVisible(true);

//    ui->passengers->setVisible(true);

//    QPointF point = car->mapToScene(scene->sceneRect().x(),scene->sceneRect().y());
//    qDebug()<<"cjf debug"<<point.x()<<point.y();
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
        emit Car_radar(data, -1,-1);
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
        img1 = FaceRecognition(img);            //人脸识别
    }else if(ui->RGYButton->isChecked()){
        img1 = rgy_light_identification(img);   //红绿黄交通灯识别
    }else if(ui->LPRButton->isChecked()){
        cv::resize(img,img,Size(320, 240));
        img1 = plr->test_mtcnn_plate(img);      //车牌识别
        ui->LPR->setText(QString::fromStdString(plr->LPR_Data));
        if(plr->LPR_Data!="")
        {
            car_lpr_flag = 1;
        }
//        plr->LPR_Data="";
    }
    QImage qimg = this->Mat2QImage(img1);

    QPixmap pixmap = QPixmap::fromImage(qimg);
    ui->Car_videoDisplay->setPixmap(pixmap.scaled(ui->Car_videoDisplay->size(),Qt::IgnoreAspectRatio));//, Qt::SmoothTransformation 保持比例
}

void AICarDemo::Car_traffic_light_Play()
{
     rgy_light_play_flag = 0;
     ul_play_flag = 0;

     if(car_play_flag ==1)
     {
         video_play->stop();
         ui->faces_data->clear();
         car_play_flag = 3;
     }
     if(car_play_flag ==2)
     {
         video_play->stop();
         car_play_flag = 0;
     }
}
Mat AICarDemo::rgy_light_identification(const Mat &mat)
{
    ui->red_light->setStyleSheet("");
    ui->green_light->setStyleSheet("");
    ui->yellow_light->setStyleSheet("");

    ui->school_red_light->setStyleSheet("");
    ui->school_green_light->setStyleSheet("");
    ui->school_yellow_light->setStyleSheet("");

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
    for (size_t i = 0; i < circles.size()&&i<3; i++) {
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
//        qDebug()<<"rgb:"<<red<<green<<yellow;
        //颜色判决
        if ((yellow*1.0 >= 0.5)|| (red*1.0  >=0.5 )||  (green*1.0 >= 0.5))//判断红绿黄三色的像素占比是否过半
        {
            circle(src, Point(c[0], c[1]),c[2], circleColor, 2, LINE_AA);//画边缘
            if(green > red && green > yellow && green > 50){
                ui->green_light->setStyleSheet("border-image:url(:/image/res/image/green_light.png);");
                ui->school_green_light->setStyleSheet("border-image:url(:/image/res/image/green_light.png);");
                rgy_light_play_flag  |= 0x02;
            }else if(red > yellow && red >50){
                ui->red_light->setStyleSheet("border-image:url(:/image/res/image/red_light.png);");
                ui->school_red_light->setStyleSheet("border-image:url(:/image/res/image/red_light.png);");
                rgy_light_play_flag  |= 0x01;

            }else{
                if(yellow > 50){
                    ui->yellow_light->setStyleSheet("border-image:url(:/image/res/image/yellow_light.png);");
                    ui->school_yellow_light->setStyleSheet("border-image:url(:/image/res/image/yellow_light.png);");
                    rgy_light_play_flag  |= 0x04;
                }
            }
        }
    }

    if(rgy_light_play_flag == 0x01){
        QSound *success = new QSound("./mp3/red_light.wav", this);
        success->play();
        rgy_light_play_flag = 8;
        video_play->start();
    }else if(rgy_light_play_flag == 0x02){
        QSound *success = new QSound("./mp3/green_light.wav", this);
        success->play();
        rgy_light_play_flag = 8;
        video_play->start();
        car_rgy_flag = 1;
    }else if(rgy_light_play_flag == 0x04){
        QSound *success = new QSound("./mp3/yellow_light.wav", this);
        success->play();
        rgy_light_play_flag = 8;
        video_play->start();
    }else if(rgy_light_play_flag==0x05 || rgy_light_play_flag==0x6 ||rgy_light_play_flag==0x7||rgy_light_play_flag==0x3){
        QSound *success = new QSound("./mp3/more_light.wav", this);
        success->play();
        rgy_light_play_flag = 8;
        video_play->start();
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
        //qDebug()<<"cjfx"<<iter->x<<"y:"<<iter->y<<"w:"<<iter->width<<"h:"<<iter->height;
        if(iter->x-5 > 0 && iter->y-5 >0){
            cv::Rect area(iter->x-5, iter->y-5, iter->width+5, iter->height+5); //需要裁减的矩形区域
            m = img2(area);//img2(*iter).copyTo(m);
            car_face_flag = 1;
        }

        QImage qimg = this->Mat2QImage(m);
        QPixmap pixmap = QPixmap::fromImage(qimg);
        ui->faces_data->setPixmap(pixmap.scaled(ui->faces_data->size(),Qt::IgnoreAspectRatio));//Qt::SmoothTransformation 保持比例
    }

    return img1;
}

void AICarDemo::Uart_Connect()
{
    QList<QSerialPortInfo> list3;//获取串口列表
    list3=QSerialPortInfo::availablePorts();
    for(int i=0;i<list3.size();i++)
        qDebug()<<list3[i].portName();//打印串口信息

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())//搜索串口，获取串口列表
    {
        if(info.portName() == "ttyS3"){//在串口列表中查找
            qDebug() << "所需串口已找到，具体信息如下：";
            qDebug() << "Name : " << info.portName();//串口名称，比如com3
            qDebug() << "Description : " << info.description();//串口描述说明
            qDebug() << "Manufacturer: " << info.manufacturer();
            qDebug() << "Serial Number: " << info.serialNumber();//串口号
            qDebug() << "System Location: " << info.systemLocation();//系统位置
            SerialPort.setPort(info);//设置串口
            break;//找到所需要的串口信息，退出循环
        }//使用if语句判断是否所需串口
    }
    if(SerialPort.open(QIODevice::ReadOnly))//打开串口，并设置串口为只读模式
    {
        ui->ul->setText(tr("关"));
        SerialPort.setBaudRate(QSerialPort::Baud9600);//设置串口波特率（9600）
        SerialPort.setDataBits(QSerialPort::Data8);//设置数据位（8）
        SerialPort.setParity(QSerialPort::NoParity); //设置奇偶校检（无）
        SerialPort.setStopBits(QSerialPort::OneStop);//设置停止位(一位)
        SerialPort.setFlowControl(QSerialPort::NoFlowControl);//设置流控制（无）

        if(SerialPort.isOpen()){
            qDebug()<<"串口已经打开";
        }

        connect(&SerialPort, SIGNAL(readyRead()), this, SLOT(Uart_ReadData()));//读取数据的函数
    }
}

void AICarDemo::Uart_ReadData()
{
    if(SerialPort.bytesAvailable() < 4)
        return;
    QByteArray data = SerialPort.readAll();

    int len = data.length();
    int i = 0, status=0, temp[3];
    quint8 ch;

    if (!data.isEmpty()){
        while(len--)
        {
            ch = data.at(i);
            switch(status){
            case 0:
                if(ch == 0xFF)
                    status = 1;
                break;
            case 1:
                temp[0] = ch;
                status = 2;
                break;
            case 2:
                temp[1] = ch;
                status = 3;
                break;
            case 3:
                temp[2] = (temp[0] + temp[1] + 0xFF)&0x00FF;
                if(ch == temp[2])
                {
                    int temp_data = temp[0]<<8 | temp[1];

                    //过滤异常数据
                    if(240 < temp_data && temp_data < 500)
                    {
                        int av_data = (ul_last_data + temp_data)/2-temp_data;
                        ul_last_data = temp_data;
                        if(-20<av_data && av_data<20)
                        {

                        }else{
                            return;
                        }
                    }
                    ul_last_data = temp_data;

                    emit Car_radar(-1,temp_data,-1);
                }
                status = 0;
                break;
            }
            i++;
        }
    }
}

void AICarDemo::Uart_Close()
{
    if(!SerialPort.isOpen())
        return ;
    SerialPort.close();
}

void AICarDemo::Uart_WriteData()
{
    QString sendData = "write test";
    QByteArray Send_temp=sendData.toLatin1();
    SerialPort.write(Send_temp);
}

void AICarDemo::on_pushButton_clicked()
{
    if(ui->scene->text()=="场景开"){
            on_scene_clicked();
    }
    ui->scene->setDisabled(true);

        ui->car_pole_1->setVisible(true);
        ui->car_pole_2->setVisible(true);
        ui->car_pole_3->setVisible(true);
        ui->red_light->setStyleSheet("");

        ui->green_light->setStyleSheet("");
        ui->yellow_light->setStyleSheet("");
    ui->school_red_light->setStyleSheet("");

    ui->school_green_light->setStyleSheet("");
    ui->school_yellow_light->setStyleSheet("");

    Car_Reset();

    car->reset();
    car->resetTransform();  //car->resetMatrix();
    car->setPos(0,0);       //初始化小车位置

    car_face_flag = 0;
    car_lpr_flag = 0;
    car_play_flag = 0;
    ul_play_flag = 0;
    rgy_light_play_flag = 0;

    plr->LPR_Data="";
    ui->LPR->clear();
    ui->faces_data->clear();
    ui->passengers->setVisible(true);

    AutoPilot->setInterval(1000/33);
    AutoPilot->start();

    //请选择目的地
    if(Car_END_flag == 0){
        QSound *success = new QSound("./mp3/arrive_where.wav", this);
        success->play();
        AutoPilot->stop();
    }else{
        ui->townhall->setEnabled(false);
        ui->school->setEnabled(false);
        ui->gym->setEnabled(false);
        ui->mall->setEnabled(false);
    }

}
void AICarDemo::AutoPilotSystem()
{
    QPointF point = car->mapToScene(scene->sceneRect().x(),scene->sceneRect().y());
//    qDebug()<<point.x()<<point.y();
    if(Car_END_flag == 1){
        Car_Map_TownHall(point);
    }
    if(Car_END_flag == 2||Car_END_flag== 3||Car_END_flag== 4){
        Car_Map_Mall(point);
    }

    //    Car_Map_Home(point);//保留路径
}
void AICarDemo::Car_Map_Home(QPointF point)
{
    if(point.x()==-160&&point.y()==-163)//起始坐标
    {
        on_accelerate_clicked();
    }

    if(point.x()==-160&&point.y()==-334)//右转
    {
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }

    if((int)point.x()==215&&(int)point.y()==-382)//前行
    {
        car->reset();
        on_accelerate_clicked();
    }

    if((int)point.x()==442&&(int)point.y()==-382)//右上角坐标点
    {
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }
    if((int)point.x()==490&&(int)point.y()==-6)
    {
        car->reset();
        on_accelerate_clicked();
    }
    if((int)point.x()==490&&(int)point.y()==40)//右边中间坐标点
    {
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }
    if((int)point.x()==115&&(int)point.y()==88)
    {
        car->reset();
        on_accelerate_clicked();
    }
    if((int)point.x()==-210&&(int)point.y()==88)//自动泊车-左转
    {
        car->reset();
        on_decelerate_clicked();

        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
    }
    if((int)point.x()==-156&&(int)point.y()==-182)//自动泊车-左转
    {
        car->reset();
        on_decelerate_clicked();
    }
    if((int)point.x()==-156&&(int)point.y()==-160)//自动泊车-左转
    {
        car->reset();
    }
}

void AICarDemo::Car_Map_Mall(QPointF point)
{
    if(point.x()==-160&&point.y()==-163)//起始坐标
    {
        on_accelerate_clicked();
    }

    if(point.x()==-160&&point.y()==-280)//车牌识别
    {
        if(car_play_flag==0)
        {
            Car_Reset();
            QSound *success = new QSound("./mp3/car_lpr_play.wav", this);
            success->play();
            car_play_flag = 2;
            video_play->start();
            ui->LPRButton->setChecked(true);//车牌检测
        }
        if(car_lpr_flag==1)
        {
            on_accelerate_clicked();
            car_lpr_flag = 0;
            ui->car_pole_2->setVisible(false);
            ui->LPRButton->setAutoExclusive(false);
            ui->LPRButton->setChecked(false);
            ui->LPRButton->setAutoExclusive(true);
        }
    }

    if(point.x()==-160&&point.y()==-334)//右转
    {
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }

    if((int)point.x()==215&&(int)point.y()==-382)//前行
    {
        Car_Reset();
        on_accelerate_clicked();
        ui->car_pole_2->setVisible(true);
    }

    if((int)point.x()==445&&(int)point.y()==-382)//右上角坐标点
    {
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }

    if((int)point.x()==493&&(int)point.y()==-6)
    {
        if(car_play_flag==0)
        {
            Car_Reset();
            QSound *success = new QSound("./mp3/car_face.wav", this);
            success->play();
            car_play_flag = 2;
            video_play->start();
            ui->FACEButton->setChecked(true);//人脸检测
        }
        if(car_face_flag==1)
        {
            on_accelerate_clicked();
            ui->passengers->setVisible(false);
            car_face_flag = 0;
            ui->FACEButton->setAutoExclusive(false);
            ui->FACEButton->setChecked(false);
            ui->FACEButton->setAutoExclusive(true);
        }
    }

    if((int)point.x()==493&&(int)point.y()==250)//右下角坐标点
    {
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }
    if((int)point.x()==118&&(int)point.y()==298)
    {
        Car_Reset();
        on_accelerate_clicked();
    }
    if((int)point.x()==105&&(int)point.y()==298&&Car_END_flag == 2)
    {
        if(car_play_flag==0)
        {
            Car_Reset();
            QSound *success = new QSound("./mp3/arrive_mall.wav", this);
            success->play();
            car_play_flag = 1;
            video_play->start();
        }
        if(car_play_flag ==3){
            on_accelerate_clicked();
            car_play_flag=0;
        }
    }
    if((int)point.x()==-55&&(int)point.y()==298&&Car_END_flag == 3)
    {
        if(car_play_flag==0)
        {
            Car_Reset();
            QSound *success = new QSound("./mp3/arrive_school.wav", this);
            success->play();
            car_play_flag = 1;
            video_play->start();
        }
        if(car_play_flag ==3){
            on_accelerate_clicked();
            car_play_flag=0;
        }
    }
    if((int)point.x()==-59&&(int)point.y()==298)
    {
        QSound *success = new QSound("./mp3/car_school_rgy.wav", this);
        success->play();
    }
    if((int)point.x()==-60&&(int)point.y()==298)
    {
        if(car_play_flag==0)
        {
            Car_Reset();
            car_play_flag = 2;
            video_play->start();
            ui->RGYButton->setChecked(true);//红绿灯识别
        }
        if(car_rgy_flag == 1)
        {
            on_accelerate_clicked();
            car_rgy_flag = 0;
            ui->RGYButton->setAutoExclusive(false);
            ui->RGYButton->setChecked(false);
            ui->RGYButton->setAutoExclusive(true);
        }
    }
    if((int)point.x()==-310&&(int)point.y()==298&&Car_END_flag == 4)
    {
        if(car_play_flag==0)
        {
            Car_Reset();
            QSound *success = new QSound("./mp3/arrive_gym.wav", this);
            success->play();
            car_play_flag = 1;
            video_play->start();
        }
        if(car_play_flag ==3){
            on_accelerate_clicked();
            car_play_flag=0;
        }
    }
    if((int)point.x()==-363&&(int)point.y()==298)//左下角坐标点
    {
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }

    if((int)point.x()==-412&&(int)point.y()==-76)
    {
        Car_Reset();
        on_accelerate_clicked();
    }
    if((int)point.x()==-412&&(int)point.y()==-175)//左中间坐标点
    {
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }
    if((int)point.x()==-36&&(int)point.y()==-224)
    {
        Car_Reset();
        on_accelerate_clicked();
    }
    if((int)point.x()==20&&(int)point.y()==-224)
    {
        ui->car_pole_3->setVisible(false);
    }
    if((int)point.x()==110&&(int)point.y()==-224)
    {
        ui->car_pole_3->setVisible(true);
    }

    if((int)point.x()==214&&(int)point.y()==-224)//自动泊车-右转倒车
    {
        QSound *success = new QSound("./mp3/automatic_parking.wav", this);
        success->play();
        Car_Reset();
        on_decelerate_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }

    if((int)point.x()==-163&&(int)point.y()==-170)//小车倒车
    {
        Car_Reset();
        on_decelerate_clicked();
    }
    if((int)point.x()==-162&&(int)point.y()==-157)//小车停止
    {
        Car_Reset();

        ui->mall->setChecked(false);
        ui->school->setChecked(false);
        ui->gym->setChecked(false);
        on_mall_clicked();
        on_school_clicked();
        on_gym_clicked();

        ui->LPR->clear();
        AutoPilot->stop();

        ui->passengers->setVisible(true);
        ui->townhall->setEnabled(true);
        ui->school->setEnabled(true);
        ui->gym->setEnabled(true);
        ui->mall->setEnabled(true);

        ui->scene->setDisabled(false);
    }
}
void AICarDemo::Car_Map_TownHall(QPointF point)
{
    if(point.x()==-160&&point.y()==-163)//起始坐标
    {
        on_accelerate_clicked();
    }

    if(point.x()==-160&&point.y()==-180)//右转
    {
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }

    if((int)point.x()==215&&(int)point.y()==-228)
    {
        Car_Reset();
        on_accelerate_clicked();
    }

    if((int)point.x()==250&&(int)point.y()==-228)//车牌识别
    {
        if(car_play_flag==0)
        {
            Car_Reset();
            QSound *success = new QSound("./mp3/car_lpr_play.wav", this);
            success->play();
            car_play_flag = 2;
            video_play->start();
            ui->LPRButton->setChecked(true);//车牌检测
        }
        if(car_lpr_flag==1)
        {
            on_accelerate_clicked();
            car_lpr_flag = 0;
            ui->car_pole_1->setVisible(false);
            ui->LPRButton->setAutoExclusive(false);
            ui->LPRButton->setChecked(false);
            ui->LPRButton->setAutoExclusive(true);
        }
    }
    if((int)point.x()==340&&(int)point.y()==-228)
    {
        ui->car_pole_1->setVisible(true);
    }
    if((int)point.x()==476&&(int)point.y()==-228)//右边中间坐标点
    {
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
    }

    if((int)point.x()==204&&(int)point.y()==-283)
    {
        Car_Reset();
        on_accelerate_clicked();
    }

    if((int)point.x()==204&&(int)point.y()==-300)
    {
        if(car_play_flag==0)
        {
            Car_Reset();
            QSound *success = new QSound("./mp3/car_face.wav", this);
            success->play();
            car_play_flag = 2;
            video_play->start();
            ui->FACEButton->setChecked(true);//人脸检测
        }
        if(car_face_flag==1)
        {
            on_accelerate_clicked();
            ui->passengers->setVisible(false);
            car_face_flag = 0;
            ui->FACEButton->setAutoExclusive(false);
            ui->FACEButton->setChecked(false);
            ui->FACEButton->setAutoExclusive(true);
        }
    }

    if((int)point.x()==204&&(int)point.y()==-367)//右边上坐标点
    {
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
    }
    if((int)point.x()==149&&(int)point.y()==-95)
    {
        Car_Reset();
        on_accelerate_clicked();
    }
    if((int)point.x()==-300&&(int)point.y()==-95)
    {
        if(car_play_flag==0)
        {
            Car_Reset();
            QSound *success = new QSound("./mp3/arrive_townhall.wav", this);
            success->play();
            car_play_flag = 1;
            video_play->start();

        }
        if(car_play_flag ==3){
            on_accelerate_clicked();
            car_play_flag=0;
        }
    }
    if((int)point.x()==-396&&(int)point.y()==-95)//左边上坐标点
    {
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
    }
    if((int)point.x()==-124&&(int)point.y()==-40)
    {
        Car_Reset();
        on_accelerate_clicked();
    }
    if((int)point.x()==-124&&(int)point.y()==40)//左边中间坐标点
    {
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
        on_turnLeft_clicked();
    }
    if((int)point.x()==-69&&(int)point.y()==-231)
    {
        Car_Reset();
        on_accelerate_clicked();
    }
    if((int)point.x()==20&&(int)point.y()==-231)
    {
        ui->car_pole_3->setVisible(false);
    }
    if((int)point.x()==110&&(int)point.y()==-231)
    {
        ui->car_pole_3->setVisible(true);
    }
    if((int)point.x()==212&&(int)point.y()==-231)
    {
        QSound *success = new QSound("./mp3/automatic_parking.wav", this);
        success->play();
        Car_Reset();
        on_decelerate_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
        on_turnRight_clicked();
    }
    if((int)point.x()==-161&&(int)point.y()==-181)
    {
        Car_Reset();
        on_decelerate_clicked();
    }
    if((int)point.x()==-161&&(int)point.y()==-160)
    {
        Car_Reset();
        ui->townhall->setChecked(false);
        on_townhall_clicked();
        ui->LPR->clear();

        AutoPilot->stop();

        ui->passengers->setVisible(true);
        ui->townhall->setEnabled(true);
        ui->school->setEnabled(true);
        ui->gym->setEnabled(true);
        ui->mall->setEnabled(true);
        ui->scene->setDisabled(false);
    }
}


void AICarDemo::on_townhall_clicked()
{
    if(ui->townhall->isChecked())//按钮按下操作
    {
        ui->townhall->setStyleSheet("border-image:url(:/image/res/image/townhall.png);");
        ui->mall->setChecked(false);
        ui->school->setChecked(false);
        ui->gym->setChecked(false);
        on_mall_clicked();
        on_school_clicked();
        on_gym_clicked();

        Car_END_flag = 1;
    }else{
        ui->townhall->setStyleSheet("border-image:url(:/image/res/image/townhall_balck.png);");
        Car_END_flag = 0;
    }
}

void AICarDemo::on_mall_clicked()
{
    if(ui->mall->isChecked())//按钮按下操作
    {
        ui->mall->setStyleSheet("border-image:url(:/image/res/image/mall.png);");
        ui->townhall->setChecked(false);
        ui->school->setChecked(false);
        ui->gym->setChecked(false);
        on_townhall_clicked();
        on_school_clicked();
        on_gym_clicked();
        Car_END_flag = 2;
    }else{
        ui->mall->setStyleSheet("border-image:url(:/image/res/image/mall_balck.png);");
        Car_END_flag = 0;
    }
}

void AICarDemo::on_school_clicked()
{
    if(ui->school->isChecked())//按钮按下操作
    {
        ui->school->setStyleSheet("border-image:url(:/image/res/image/school.png);");
        ui->townhall->setChecked(false);
        ui->mall->setChecked(false);
        ui->gym->setChecked(false);
        on_townhall_clicked();
        on_mall_clicked();
        on_gym_clicked();
        Car_END_flag = 3;
    }else{
        ui->school->setStyleSheet("border-image:url(:/image/res/image/school_balck.png);");
        Car_END_flag = 0;
    }
}

void AICarDemo::on_gym_clicked()
{
    if(ui->gym->isChecked())//按钮按下操作
    {
        ui->gym->setStyleSheet("border-image:url(:/image/res/image/gym.png);");
        ui->townhall->setChecked(false);
        ui->mall->setChecked(false);
        ui->school->setChecked(false);
        on_townhall_clicked();
        on_mall_clicked();
        on_school_clicked();

        Car_END_flag = 4;
    }else{
        ui->gym->setStyleSheet("border-image:url(:/image/res/image/gym_balck.png);");
        Car_END_flag = 0;
    }
}

void AICarDemo::on_radar_clicked()
{
    if(ui->radar->text() == "开"){
        Car_millimeter_radar = 1;
        ui->radar->setText(tr("关"));
    }else{
        Car_millimeter_radar = 0;
        ui->radar->setText(tr("开"));
    }
}

void AICarDemo::on_ul_clicked()
{
    if(ui->ul->text()== "开")//按钮按下操作
    {
        Uart_Connect();
    }else{
        Uart_Close();
        ui->ul->setText(tr("开"));
    }
}

void AICarDemo::on_rplidar_clicked()
{
    if(ui->rplidar->text()=="开")
    {
        modbusThread->modbus_rplidar_startMotor();
        ui->rplidar->setText(tr("关"));
        pm->start();
    }else{
        modbusThread->modbus_rplidar_stopMotor();
        Car_Reset();
        ui->rplidar->setText(tr("开"));
        pm->stop();
    }
}

void AICarDemo::on_scene_clicked()
{
    if(ui->scene->text()=="场景关"){
        ui->graphicsView->setStyleSheet("border-image:url(:/image/res/image/car_map_1.png);");

        ui->passengers->setVisible(false);
        ui->townhall->setVisible(false);
        ui->school->setVisible(false);
        ui->gym->setVisible(false);
        ui->mall->setVisible(false);
        ui->home->setVisible(false);

        ui->car_pole_1->setVisible(false);
        ui->car_pole_2->setVisible(false);
        ui->car_pole_3->setVisible(false);

        ui->school_rgy_light->setVisible(false);
        ui->scene->setText(tr("场景开"));
    }else{
        ui->graphicsView->setStyleSheet("border-image:url(:/image/res/image/car_map.png);");

        ui->passengers->setVisible(true);
        ui->townhall->setVisible(true);
        ui->school->setVisible(true);
        ui->gym->setVisible(true);
        ui->mall->setVisible(true);
        ui->home->setVisible(true);

        ui->car_pole_1->setVisible(true);
        ui->car_pole_2->setVisible(true);
        ui->car_pole_3->setVisible(true);

        ui->school_rgy_light->setVisible(true);
        ui->scene->setText(tr("场景关"));
    }
}

void AICarDemo::on_connectType_currentIndexChanged(int index)
{
    auto type = static_cast<ModbusConnection> (index);
    if (type == Serial) {
//        portFind();
    }else if (type == Tcp) {
//        get_ip();
//        qDebug()<<"cjf 1a";
    }
}

/*自动查询串口*/
void AICarDemo::portFind()
{
    QList<QSerialPortInfo> list3;//获取串口列表
    list3 = QSerialPortInfo::availablePorts();
    for(int i = 0; i < list3.size(); i++)
        qDebug()<<list3[i].portName();//打印串口信息

    //检查可用的端口，添加到下拉框以供选择
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "Name : " << info.portName();
        QSerialPort serial_num;
        serial_num.setPort(info);
        if(serial_num.open(QIODevice::ReadWrite))
        {
         //   ui->com->addItem(serial_num.portName());
            serial_num.close();
        }
//        if(serial_num.portName()=="ttyUSB1"){
//            ui->lineEdit->setText(QLatin1Literal("ttyUSB1"));
//        }
    }

}

void AICarDemo::on_speech_pressed()
{
    ui->speech->setText("松开识别");
    //开始录音-1.USB麦克或自带的麦克
//    audio = new Audio;
//    audio->startAudio("file_16k.pcm");

    //开始录音-2.使用科大讯飞麦克阵列录音
    hidmic->hidmic_start_record();
}

void AICarDemo::on_speech_released()
{
    //停止录音-1.USB麦克或自带的麦克
    //audio->stopAudio();

    //停止录音-2.使用科大讯飞麦克阵列
    hidmic->hidmic_stop_record();
    //修改按钮文字
    ui->speech->setText("开始识别");

    //开始识别
    Speech m_speech;
    //QString text = m_speech.speechIdentify("file_16k.pcm");//自带的麦克
    QString text = m_speech.speechIdentify("./audio/mic_demo_vvui_deno.pcm");

    if(text == "向左。"){
        Car_Reset();
        on_turnLeft_clicked();
    }else if(text == "向右。"){
        Car_Reset();
        on_turnRight_clicked();
    }else if(text.contains("加档")){
        Car_Reset();
        on_accelerate_clicked();
    }else if(text.contains("加挡")){
        Car_Reset();
        on_accelerate_clicked();
    }else if(text == "家长。"){
        Car_Reset();
        on_accelerate_clicked();
    }else if(text.contains("前")){
        Car_Reset();
        on_accelerate_clicked();
    }else if(text.contains("减档")){
        Car_Reset();
        on_decelerate_clicked();
    }else if(text.contains("减挡")){
        Car_Reset();
        on_decelerate_clicked();
    }else if(text.contains("后")){
        Car_Reset();
        on_decelerate_clicked();
    }else if(text.contains("开灯")){
        QString topic = m_publish;
        QString data  = "{\"desired\":{\"LED\":\"on\"}}";
        m_client->publish(topic,data.toUtf8());
    }else if(text.contains("关灯")){
        QString topic = m_publish;
        QString data  = "{\"desired\":{\"LED\":\"off\"}}";
        m_client->publish(topic,data.toUtf8());
    }else if(text.contains("开门")){
        QString topic = m_publish;
        QString data  ="{\"desired\":{\"OCRelay\":\"on\"}}";
        m_client->publish(topic,data.toUtf8());
    }else if(text.contains("关门")){
        QString topic = m_publish;
        QString data  ="{\"desired\":{\"OCRelay\":\"off\"}}";
        m_client->publish(topic,data.toUtf8());
    }else if(text.contains("鸣笛")){
        QString topic = m_publish;
        QString data  ="{\"desired\":{\"BUZZER\":\"on\"}}";
        m_client->publish(topic,data.toUtf8());
        QTimer::singleShot(5000,this,SLOT(QTimer_Buzzer()));//延时5秒
    }else if(text.contains("复位")){
        on_Car_reset_clicked();
    }else if(text.contains("停")){
        Car_Reset();
    }

    ui->textEdit->setText(text);

    ui->speech->setText("按住说话");
}

void AICarDemo::SmartCar_MQTT_init()
{
    m_client = new QMqttClient(this);

    // QFile 构造函数中打开文件
    QFile *myFile;
    QString filename="./data/baidu_mqtt.sh";
    myFile=new QFile(filename);

    // 只读打开文件
    if(myFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString buffer;
        // 返回字节数，
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
            m_client->setHostname(buffer.trimmed());
        }
        buffer  = myFile->readLine();

        if (!buffer.isNull())
        {
            m_client->setPort(buffer.toInt());
        }
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
            m_client->setUsername(buffer.trimmed());
        }
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
            m_client->setPassword(buffer.trimmed());
        }
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
             m_subscribe = buffer.trimmed();
//            m_client->subscribe(buffer.trimmed());
        }
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
            m_publish = buffer.trimmed();
//            m_client->publish(buffer.trimmed());
        }
        myFile->close();
    }

    connect(m_client, &QMqttClient::disconnected, this, &AICarDemo::brokerDisconnected);
    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        cJSON *proot,*parray;
        proot = cJSON_Parse(message);
        if(proot != NULL)
        {
            double buzzer_data;
            if((parray = cJSON_GetObjectItem(proot, "reported")) != NULL)//判断是否有reported
            {
                if(cJSON_GetObjectItem(parray,"DHT11_Temp") != NULL)
                {
                    buzzer_data = cJSON_GetObjectItem(parray, "DHT11_Temp")->valuedouble;//字符串
                    ui->state_DHT11_temp_data->setText(QString::number(buzzer_data,'f',1));
                }
                if(cJSON_GetObjectItem(parray,"DHT11_Humi") != NULL)
                {
                    buzzer_data = cJSON_GetObjectItem(parray, "DHT11_Humi")->valuedouble;//字符串
                    ui->state_DHT11_humi_data->setText(QString::number(buzzer_data,'f',1));
                }

                if(cJSON_GetObjectItem(parray,"RE200B") != NULL)
                {
                    QString data = cJSON_GetObjectItem(parray, "RE200B")->valuestring;//字符串
                    if(data == "abnormal!")
                    {
                        ui->state_PIR_data->setText("有人");
                    }else{
                        ui->state_PIR_data->setText("无人");
                    }
                }

                if(cJSON_GetObjectItem(parray,"LED") != NULL)
                {
                    QString led_data = cJSON_GetObjectItem(parray, "LED")->valuestring;//字符串
                    state_LED_data = led_data;
                    if(led_data == "ledon")
                    {
                        ui->LED_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/car_led_on.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/car_led_on.png);}");
                    }
                    else{
                        ui->LED_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/car_led_off.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/car_led_off.png);}");
                    }
                }
                if(cJSON_GetObjectItem(parray,"BUZZER") != NULL)
                {
                    QString buzzer_data = cJSON_GetObjectItem(parray, "BUZZER")->valuestring;//字符串
                    state_BUZZER_data = buzzer_data;
                    if(buzzer_data == "buzzeron"){
                        ui->BUZZER_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/buzzer_on.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/buzzer_on.png);}");
                    }
                    else{
                        ui->BUZZER_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/buzzer_off.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/buzzer_off.png);}");
                    }
                }

                if(cJSON_GetObjectItem(parray,"OCRelay") != NULL)
                {
                    QString ocr_data = cJSON_GetObjectItem(parray, "OCRelay")->valuestring;//字符串
                    state_OCR_data = ocr_data;
                    if(ocr_data == "OCRelay_On"){
                        ui->OCR_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/car_door_on.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/car_door_on.png);}");

                    }
                    else{
                        ui->OCR_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/car_door_off.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/car_door_off.png);}");

                    }
                }

            }
        }
        cJSON_Delete(proot);
    });

}

void AICarDemo::brokerDisconnected()
{
    ui->Key_SmartCar->setText(tr("开"));
}

void AICarDemo::on_Key_SmartCar_clicked()
{
    if (m_client->state() == QMqttClient::Disconnected) {
        m_client->connectToHost();
        ui->Key_SmartCar->setText(tr("关"));
        QTimer::singleShot(5000,this,SLOT(QTimer_Subscribe()));//延时1秒订阅主题
    } else {
        m_client->disconnectFromHost();
        ui->Key_SmartCar->setText(tr("开"));
    }
}

void AICarDemo::on_Subscribe()
{
    auto subscription = m_client->subscribe(m_subscribe);
    if (!subscription) {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
}

void AICarDemo::QTimer_Subscribe()
{
    on_Subscribe();
}

void AICarDemo::QTimer_Buzzer()
{
    QString topic = m_publish;
    QString data  ="{\"desired\":{\"BUZZER\":\"off\"}}";
    m_client->publish(topic,data.toUtf8());
}

void AICarDemo::on_LED_data_clicked()
{
    if(state_LED_data == "ledon"){
        QString topic = m_publish;
        QString data  = "{\"desired\":{\"LED\":\"off\"}}";
        m_client->publish(topic,data.toUtf8());
    }
    else{
        QString topic = m_publish;//百度LED主题地址
        QString data  ="{\"desired\":{\"LED\":\"on\"}}";
        m_client->publish(topic,data.toUtf8());
    }
}

void AICarDemo::on_BUZZER_data_clicked()
{
    if(state_BUZZER_data == "buzzeron"){
        QString topic = m_publish;
        QString data  ="{\"desired\":{\"BUZZER\":\"off\"}}";
        m_client->publish(topic,data.toUtf8());
    }
    else{
        QString topic = m_publish;
        QString data  ="{\"desired\":{\"BUZZER\":\"on\"}}";
        m_client->publish(topic,data.toUtf8());
    }
}

void AICarDemo::on_OCR_data_clicked()
{
    if(state_OCR_data == "OCRelay_On"){
        QString topic = m_publish;
        QString data  ="{\"desired\":{\"OCRelay\":\"off\"}}";
        m_client->publish(topic,data.toUtf8());
    }
    else{
        QString topic = m_publish;
        QString data  ="{\"desired\":{\"OCRelay\":\"on\"}}";
        m_client->publish(topic,data.toUtf8());
    }
}

void AICarDemo::on_Car_GPSMap_clicked()
{
    gps = new GPSDemo(nullptr);
    gps->show();
}

void AICarDemo::on_exit_clicked()
{
    if(ui->connect->text()=="Disconnect"){
        Car_Reset();
        emit Car_connect(ui->lineEdit->text());
    }
    Uart_Close();//关闭串口
    disconnect(this, SIGNAL(Car_connect(QString)),modbusThread,SLOT(on_connect(QString)));
    disconnect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Car_change_connet(bool)));

    disconnect(this, SIGNAL(Car_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    disconnect(this, SIGNAL(Car_read(int,quint16)),modbusThread,SLOT(on_read(int,quint16)));//only read
    disconnect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Car_read_data(int, int)));

    disconnect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(Car_videoDisplay(QImage)));
    disconnect(AutoPilot,SIGNAL(timeout()),this,SLOT(AutoPilotSystem()));

    pm->stop();
    disconnect(this, SIGNAL(Car_radar(int,int,int)),this,SLOT(Read_Radar(int,int,int)));//获取雷达数据
    disconnect(modbusThread, SIGNAL(rplidar_read(int,int,int)),this,SLOT(Read_Radar(int,int,int)));
    modbusThread->modbus_rplidar_stopMotor();

    if(hidmic_open_flag)
        hidmic->hidmic_close();//关闭麦克风
    AICarDemo::deleteLater();//关闭当前窗口
}
