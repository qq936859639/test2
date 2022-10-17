#include "modbusthread.h"
#include <QStatusBar>
#include <QDebug>
ModbusThread::ModbusThread(QObject *parent)
    : QThread(parent)
    , lastRequest(nullptr)
    , modbusDevice(nullptr)
{
    on_connectType_currentIndexChanged();

//    rplidar = new RPLIDAR();
//    if(rplidar->rplidar_open()==0){

//    rplidar_flag = true;
//    }
}
ModbusThread::~ModbusThread()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;
    rplidar->rplidar_close();
}
void ModbusThread::on_connectType_currentIndexChanged()
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }
    modbusDevice = new QModbusTcpClient(this);

    if (!modbusDevice) {
     qDebug()<<"error cjf";

    } else {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &ModbusThread::onStateChanged);
    }
}
void ModbusThread::on_connect(QString userip)
{
    if (!modbusDevice)
        return;

    if(modbusDevice->state() != QModbusDevice::ConnectedState)
    {
        //处于非连接状态，进行连接
        //TCP连接,端口502，地址192.168.*.*
        const QUrl url = QUrl::fromUserInput(userip); //获取IP和端口号
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());

        modbusDevice->setTimeout(500);
        modbusDevice->setNumberOfRetries(0);
        if(!modbusDevice->connectDevice()){//连接失败
           isConnected = false;
            perror("modbusDevice Connect failed");
        }else {//成功连接
//            isConnected = true;
//            qDebug() << "Connect ok";
        }
    }else{
//        isConnected = false;
        modbusDevice->disconnectDevice();
        qDebug()<<("modbusDevice disconnectDevice");
    }
}

void ModbusThread::on_write(quint16 data)
{
    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0xB, 1);
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
        int ii = static_cast<int>(i);
//        int j = 4*ii;
        //t = "45";
        //  QString st = t.mid (j,4);
        //  bool ok;
        //  int hex =st.toInt(&ok,10);//将读取到的数据转换为10进制发送
        //quint16 qhex =static_cast<quint16>(hex);
        quint16 qhex = data;
        // qDebug()<<writeUnit.valueCount();
        writeUnit.setValue(ii,qhex);
    }

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    //statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                     //   .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                     //   5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    //statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                     //   arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        //statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }

}

void ModbusThread::on_read(int startAddress, quint16 numberOfEntries)
{
    if (!modbusDevice)
        return;

//    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::InputRegisters, startAddress, numberOfEntries);
    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {  //1->modbus设备地址
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &ModbusThread::readReady);
        else
            delete reply; // broadcast replies return immediately
    }
    else{
        //emit statusBar(tr("Read error: ") + modbusDevice->errorString());
    }
}

void ModbusThread::readReady()
{
    //QModbusReply这个类存储了来自client的数据,sender()返回发送信号的对象的指针
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError){
        //处理成功返回的数据
        const QModbusDataUnit unit = reply->result();
        for (uint i = 0; i < unit.valueCount(); i++) {
//        int address = unit.startAddress();
//        quint16 data = unit.value(0);  //状态（位与关系）

        //待处理
//        qDebug()<<i<<"cjf"<<unit.startAddress()+i<<"-cjf-"<<unit.value(i);
        emit on_read_data(unit.startAddress()+i, unit.value(i));

        }
    }else if (reply->error() == QModbusDevice::ProtocolError){
        //emit statusBar(tr("Read response error: %1 (Mobus exception: 0x%2)").
        //                  arg(reply->errorString()).
        //                   arg(reply->rawResult().exceptionCode(), -1, 16));
    }else{
        //emit statusBar(tr("Read response error: %1 (code: 0x%2)").
        //                   arg(reply->errorString()).
        //                   arg(reply->error(), -1, 16));
    }
    reply->deleteLater();
}

void ModbusThread::on_readWrite(quint16 data)
{
    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0xB, 1);
    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0xB, 1);
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
//        int ii = static_cast<int>(i);
//        int j = 4*ii;
//        QString t = "45";
//        QString st = t.mid (j,4);
//        bool ok;
//        int hex =st.toInt(&ok,16);//将读取到的数据转换为16进制发送
//        quint16 qhex =static_cast<quint16>(hex);
        quint16 qhex = data;
       // qDebug()<<writeUnit.valueCount();
        writeUnit.setValue(i,qhex);
    }

    if (auto *reply = modbusDevice->sendReadWriteRequest(readUnit, writeUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &ModbusThread::readReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        //statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

void ModbusThread::on_writeRead(int startAddress, quint16 numberOfEntries, quint16 data,quint16 data2)
{
    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
//        int hex =st.toInt(&ok,16);//将读取到的数据转换为16进制发送
//        quint16 qhex =static_cast<quint16>(hex);
//        qDebug()<<writeUnit.valueCount();
        if(i==0){
            quint16 qhex = data;
            writeUnit.setValue(i,qhex);
        }
        if(i==1){
            quint16 qhex = data2;
            writeUnit.setValue(i,qhex);
        }
    }

    if (auto *reply = modbusDevice->sendReadWriteRequest(readUnit,writeUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &ModbusThread::readReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        //statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

void ModbusThread::onStateChanged(int state)//连接状态发生改变时处理函数（connect or discennect）
{
    if (state == QModbusDevice::UnconnectedState)
    {
        isConnected = false;
        emit on_change_connet(isConnected);
    }else if (state == QModbusDevice::ConnectedState){
        isConnected = true;
        emit on_change_connet(isConnected);
    }
}
void ModbusThread::modbus_rplidar_startMotor(){
    rplidar->rplidar_startMotor();
}
void ModbusThread::modbus_rplidar_stopMotor(){
    rplidar->rplidar_stopMotor();
}
void ModbusThread::run()
{
    rplidar = new RPLIDAR();
    if(rplidar->rplidar_open()==0){

        rplidar_flag = true;
    }

    while(rplidar_flag==true){
    rplidar->rplidar_read();
    emit rplidar_read(-1,-1,rplidar->rplidar_ranges_flag);
    }
}
