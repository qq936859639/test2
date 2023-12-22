#ifndef MODBUSTHREAD_H
#define MODBUSTHREAD_H

#include <QThread>
#include <QObject>

#include <QModbusDataUnit>
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QUrl>

//#include "AICarDemo/rplidar/rplidar.h"
#include "AICarDemo/ydlidar/ydlidar.h"

#define HOST_NAME "192.168.88.132:8232"      //云台IP地址(ESP32)

#define CAMERA_ADDR1 0x000B //舵机1轴旋转角度
#define CAMERA_ADDR2 0x000C //舵机2轴旋转角度

#define CAR_LEFT_HEAD_LED_DATA 0x0001    //小车左前灯亮度
#define CAR_RIGHT_HEAD_LED_DATA 0x0002   //小车右前灯亮度
#define CAR_LEFT_REAR_LED_DATA 0x0003    //小车左后灯亮度
#define CAR_RIGHT_REAR_LED_DATA 0x0004   //小车右后灯亮度
#define CAR_COMMAND_LED_ADDR 0x0005 //小车灯控制命令
#define CAR_AUTOMATIC_LED 0x00    //小车灯自动控制模式
#define CAR_LEFT_HEAD_LED 0x01    //小车左前灯
#define CAR_RIGHT_HEAD_LED 0x02   //小车右前灯
#define CAR_LEFT_REAR_LED 0x04    //小车左后灯
#define CAR_RIGHT_REAR_LED 0x08   //小车右后灯

#define CAR_TURN_ADDR_DATA 0x0006 //小车转向电机[-5,5]
#define CAR_POWER_ADDR_DATA 0x0007 //小车动力电机[-5,5]

class ModbusThread : public QThread
{
    Q_OBJECT
public:
    explicit ModbusThread(QObject *parent = nullptr);
    ~ModbusThread();
    bool isConnected;

    YDLIDAR *rplidar;
    bool rplidar_flag = false;

    void modbus_rplidar_startMotor();
    void modbus_rplidar_stopMotor();

    quint8 connectType = 0;
signals:
    void on_read_data(int startAddress, int data);
    void on_change_connet(bool);
    void rplidar_read(int data1,int data2,int data3);
private slots:
    void on_connect(const QString ip);
    void on_write(quint16 data);
    void on_read(int startAddress, quint16 numberOfEntries);
    void on_readWrite(quint16 data);

    void on_writeRead(int startAddress, quint16 numberOfEntries, quint16 data,quint16 data2 = 0);

    void on_connectType_currentIndexChanged(quint8 index);
protected:

    //void on_connect();
    //void on_write(QString t);

    void readReady();
    void onStateChanged(int state);

    void run();
private:
    QModbusReply *lastRequest;
    QModbusClient *modbusDevice;

};

#endif // MODBUSTHREAD_H
