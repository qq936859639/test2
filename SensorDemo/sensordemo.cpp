#include "sensordemo.h"
#include "ui_sensordemo.h"
#include <QDebug>
#include <QMessageBox>
SensorDemo::SensorDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SensorDemo)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->close_port->setEnabled(false);
    //初始化端口
    serialport = new QSerialPort(this);
    portFind();
}

SensorDemo::~SensorDemo()
{
    delete ui;
}

void SensorDemo::on_open_port_clicked()
{
    //初始化界面并初始化端口
    update();

    //初始化串口的各项参数
    serialport->setPortName(ui->com->currentText());

    //如果成功打开串口则允许设置参数，否则报错提示
    if(serialport->open(QIODevice::ReadWrite))
    {
        ui->open_port->setEnabled(false);
        ui->close_port->setEnabled(true);

        //设置波特率
        serialport->setBaudRate(ui->baud->currentText().toInt());
        //设置数据位
        switch (ui->databit->currentIndex()) {
        case 5:
            serialport->setDataBits(QSerialPort::Data5);
            break;
        case 6:
            serialport->setDataBits(QSerialPort::Data6);
            break;
        case 7:
            serialport->setDataBits(QSerialPort::Data7);
            break;
        case 8:
            serialport->setDataBits(QSerialPort::Data8);
            break;
        default:
            break;
        }
        //设置奇偶校验位
        switch (ui->checkbit->currentIndex()) {
        case 0:
            serialport->setParity(QSerialPort::NoParity);
            break;
        default:
            break;
        }
        //设置停止位
        switch (ui->stopbit->currentIndex()) {
        case 1:
            serialport->setStopBits(QSerialPort::OneStop);
            break;
        case 2:
            serialport->setStopBits(QSerialPort::TwoStop);
            break;
        default:
            break;
        }
        //设置流控制
        serialport->setFlowControl(QSerialPort::NoFlowControl);

        //串口设置完成，监听数据
        connect(serialport, &QSerialPort::readyRead, this, &SensorDemo::read_data);
    }
    else
    {
        QMessageBox::information(this, tr("Warning"), tr("Cannot open this serialport!"), QMessageBox::Ok);
    }
}

void SensorDemo::on_close_port_clicked()
{
    //关闭串口，重置串口，重查串口，清空文本框
    ui->open_port->setEnabled(true);
    ui->close_port->setEnabled(false);

    serialport->clear();
    serialport->close();
}

void SensorDemo::on_clear_receive_button_clicked()
{
    ui->Receive_text_win->clear();
}
/*自动查询串口*/
void SensorDemo::portFind()
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
            ui->com->addItem(serial_num.portName());
            serial_num.close();
        }
        if(serial_num.portName()=="ttyS3")
            ui->com->setCurrentText("ttyS3");
    }

}
/*接收数据*/
void SensorDemo::read_data()
{
    //设置接收字节数
    if(serialport->bytesAvailable() < 4)
        return;
    //字符串或者十六进制接收
    QByteArray buffer;
    buffer = serialport->readAll();

    if(!buffer.isNull()){    //解析数据
        QString str = ui->Receive_text_win->toPlainText();
        if(ui->receive_state->currentText() == "string")
        {
            str = tr(buffer);
            ui->Receive_text_win->append(str);
        }else{
            QByteArray temp_data = buffer.toHex().toUpper();
            str = tr(temp_data);
            ui->Receive_text_win->append(str);
        }
        Sensor_PARSE(buffer);
    }
    buffer.clear();
}
void SensorDemo::Sensor_PARSE(QByteArray data)
{
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
                    ui->sensor_data->setText(QString::number(temp_data));
                    if(temp_data <240)
                    {
                        ui->message->setText(tr("数据无效"));
                    }else if (240 < temp_data && temp_data < ui->spinBox->value()){
                        ui->message->setText(tr("有障碍物"));
                    }else{
                        ui->message->setText(tr("无障碍物"));
                    }
                }
                status = 0;
                break;
            }
            i++;
        }
    }
}

void SensorDemo::on_quit_clicked()
{
    SensorDemo::deleteLater();//关闭当前窗口
}
