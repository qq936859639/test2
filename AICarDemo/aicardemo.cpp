﻿#include "aicardemo.h"
#include "ui_aicardemo.h"
#include <QTimer>
#include <QDebug>
#include <math.h>
AICarDemo::AICarDemo(QWidget *parent, ModbusThread *modbusthread) :
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

    this->modbusThread = modbusthread;
    connect(this, SIGNAL(Car_connect()),modbusThread,SLOT(on_connect()));
    connect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Car_change_connet(bool)));

    connect(this, SIGNAL(Car_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    connect(this, SIGNAL(Car_read(int,quint16)),modbusThread,SLOT(on_read(int,quint16)));//only read
    connect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Car_read_data(int, int)));
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

    disconnect(this, SIGNAL(Car_connect()),modbusThread,SLOT(on_connect()));
    disconnect(modbusThread, SIGNAL(on_change_connet(bool)),this,SLOT(Car_change_connet(bool)));

    disconnect(this, SIGNAL(Car_writeRead(int, quint16, quint16)),modbusThread,SLOT(on_writeRead(int, quint16, quint16)));

    disconnect(this, SIGNAL(Car_read(int,quint16)),modbusThread,SLOT(on_read(int,quint16)));//only read
    disconnect(modbusThread, SIGNAL(on_read_data(int, int)),this,SLOT(Car_read_data(int, int)));
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

void AICarDemo::on_pushButton_clicked()
{
    Car_turn_flag = 0;
    Car_AD_flag = 0;
    Car_turn_LR_Angle_num = 750;
    Car_AD_Rate_num = 450;
    car->reset();
    emit Car_writeRead(CAR_COMMAND_ADDR, 1, 0);//小车复位
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
    qDebug()<<"cjf"<<radar_data;
    }
}

