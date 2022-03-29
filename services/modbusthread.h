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
//#include <QModbusRtuSerialMaster>
//#include <QModbusClient>
//#include <QModbusReply>
//class QModbusClient;
//class QModbusReply;
#define HOST_NAME "192.168.88.132:8232"      //云台IP地址(ESP32)
class ModbusThread : public QThread
{
    Q_OBJECT
public:
    explicit ModbusThread(QObject *parent = nullptr);
    ~ModbusThread();
    bool isConnected;
signals:
    void on_read_data(quint16 t);
    void on_change_connet(bool);
private slots:
    void on_connect();
    void on_write(quint16 t);
    void on_read();
    void on_readWrite(quint16 t);

    void on_writeRead(quint16 t);
protected:

    //void on_connect();
    //void on_write(QString t);


    void readReady();
    void onStateChanged(int state);

    void on_connectType_currentIndexChanged();
private:
    QModbusReply *lastRequest;
    QModbusClient *modbusDevice;
};

#endif // MODBUSTHREAD_H
