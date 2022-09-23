#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "AICarDemo/car/car.h"

#include "CameraDemo/camerademo.h"
#include "AICarDemo/aicardemo.h"
#include "LCDCheck/lcdcheck.h"
#include "SmartHome/smarthome.h"
#include "MICDemo/micdemo.h"
#include "GPSDemo/gpsdemo.h"
#include "KeyboardDemo/keyboarddemo.h"
#include "SensorDemo/sensordemo.h"
#include "SpeakerDemo/speakerdemo.h"
#include "CoreModule/coremodule.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    modbusThread = new ModbusThread(this);
    modbusThread->start();

    cameraThread = new CameraThread(this);
    cameraThread->start();
}

MainWindow::~MainWindow()
{
    modbusThread->deleteLater();
    modbusThread->wait();

    cameraThread->startflag=false;
    cameraThread->deleteLater();
    cameraThread->wait();

    delete modbusThread;
    delete cameraThread;

    delete ui;
}

void MainWindow::on_btcamera_clicked()
{

    CameraDemo *camerademo = new CameraDemo(nullptr, cameraThread, modbusThread);
    camerademo->show();
}

void MainWindow::on_btcar_clicked()
{
    AICarDemo *aicardemo = new AICarDemo(nullptr, cameraThread, modbusThread);
    aicardemo->show();
}

void MainWindow::on_btlcd_clicked()
{
    LCDCheck *lcd = new LCDCheck(nullptr);
    lcd->show();
}

void MainWindow::on_btmic_clicked()
{
    MICDemo *mic = new MICDemo(nullptr);
    mic->show();
}

void MainWindow::on_bta311d_clicked()
{
    CoreModule *core = new CoreModule();
    core->show();
}

void MainWindow::on_btkeyboard_clicked()
{
    KeyboardDemo *keyboard = new KeyboardDemo();
    keyboard->show();
}

void MainWindow::on_btgps_clicked()
{
    GPSDemo *gps = new GPSDemo(nullptr);
    gps->show();
}

void MainWindow::on_btsonor_clicked()
{
    SensorDemo *sensor = new SensorDemo();
    sensor->show();
}

void MainWindow::on_btspeaker_clicked()
{
    SpeakerDemo *speaker = new SpeakerDemo();
    speaker->show();
}

void MainWindow::on_btiotdht_clicked()
{
    SmartHome *smarhome = new SmartHome(nullptr, cameraThread);
    smarhome->show();
}
