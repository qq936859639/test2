#ifndef GPSDEMO_H
#define GPSDEMO_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class GPSDemo;
}

class GPSDemo : public QWidget
{
    Q_OBJECT

public:
    explicit GPSDemo(QWidget *parent = nullptr);
    ~GPSDemo();
    struct date_time{
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
    };
    /*-------------------北斗位置信息-----------------------*/
    struct GPS_INFO{
        date_time D;//时间
        char status;       //接收状态
        double latituded;   //纬度度
        double latitudef;   //纬度分
        double latitude;   //纬度
        double longituded;  //经度度
        double longitudef;  //经度分
        double longitude;  //经度
        char NS;           //南北极
        char EW;           //东西
        double speed;      //速度
        double high;       //高度
        QString Position;//physical
        int Vid;      //汽车编号
        qint64 Tim;   //时间戳
    };
    void gps_parse(char *line,GPS_INFO *GPS); //将得到的数据解析到GPS中
    bool flagDB=true;  //判断北斗数据解析是否正确
    void show_gps(GPS_INFO *GPS);
    int GetComma(int num,char *str);
    double get_double_number(char *s);
private:
    void gpsParse(QByteArray GPSBuffer);
private slots:
    void portFind();
    void on_open_port_clicked();
    void read_data();

    void on_close_port_clicked();

    void on_clear_receive_button_clicked();

    void on_gps_map_clicked();
    QByteArray checkData(QByteArray temp1,const QByteArray &s1,const QByteArray &s2);
    void DBSend(QByteArray temp3);   //北斗发送函数
private:
    Ui::GPSDemo *ui;
    QSerialPort *serialport;
    int k = 0;
    QByteArray Message,MessageHead,MessageEnd;
    QByteArray temp2;   //存放GPS缓冲数据
};

#endif // GPSDEMO_H
