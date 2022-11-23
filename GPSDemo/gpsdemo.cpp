#include "gpsdemo.h"
#include "ui_gpsdemo.h"
#include <QMessageBox>
#include <QList>
#include <QDebug>

GPSDemo::GPSDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GPSDemo)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
//  ui->webView->load(QUrl("file:/home/cjf/test/QtBaiduMapApi-master/map.html"));绝对路径
    ui->webView->load(QUrl("file:" + qApp->applicationDirPath() + "/data/baidumap.html"));

    ui->close_port->setEnabled(false);
    //初始化端口
    serialport = new QSerialPort(this);
    portFind();
    //GPS实例
    GPS = new GPS_INFO();
}

GPSDemo::~GPSDemo()
{
    delete ui;
}

/*自动查询串口*/
void GPSDemo::portFind()
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
//        if(serial_num.open(QIODevice::ReadWrite))
//        {
            ui->com->addItem(serial_num.portName());
//            serial_num.close();
//        }
        if(serial_num.portName()=="ttyS4")
            ui->com->setCurrentText("ttyS4");
    }

}

void GPSDemo::on_open_port_clicked()
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

//        serialport->setBaudRate(QSerialPort::Baud9600,QSerialPort::AllDirections);//波特率9600
//        serialport->setDataBits(QSerialPort::Data8);        //数据位8
//        serialport->setParity(QSerialPort::NoParity);       //无校验位
//        serialport->setStopBits(QSerialPort::OneStop);         //停止位1
//        serialport->setFlowControl(QSerialPort::NoFlowControl);    //无线流
        //串口设置完成，监听数据
        connect(serialport, &QSerialPort::readyRead, this, &GPSDemo::read_data);
    }
    else
    {
        QMessageBox::information(this, tr("Warning"), tr("Cannot open this serialport!"), QMessageBox::Ok);
    }
}
/*接收数据*/
void GPSDemo::read_data()
{
    //字符串或者十六进制接收
    QByteArray buffer, buffer_temp;
    buffer = serialport->readAll();

    buffer_temp=checkData(buffer,"$","\n");    //连接北斗数据，“$”开始，“\n”结束

    if(!buffer_temp.isNull()){    //解析北斗数据
        QString str = ui->Receive_text_win->toPlainText();
        if(ui->receive_state->currentText() == "string")
        {
            str = tr(buffer_temp);
            ui->Receive_text_win->append(str);
        }else{
            QByteArray temp_data = buffer_temp.toHex().toUpper();
            str = tr(temp_data);
            ui->Receive_text_win->append(str);
        }
        GPS_DB_PARSE(buffer_temp);
    }
    buffer.clear();
}
/*关闭串口*/
void GPSDemo::on_close_port_clicked()
{
    //关闭串口，重置串口，重查串口，清空文本框
    ui->open_port->setEnabled(true);
    ui->close_port->setEnabled(false);

    serialport->clear();
    serialport->close();
}

void GPSDemo::on_clear_receive_button_clicked()
{
     ui->Receive_text_win->clear();
}

void GPSDemo::on_gps_map_clicked()
{
//    QString allStr  = QDateTime::currentDateTime().toString();
    QString strVal = QString("callfromqt(\"%1\",\"%2\",\"%3\",\"%4\");").arg("").arg("").arg(ui->longlineEdit->text()).arg(ui->latlineEdit->text());
    ui->webView->page()->runJavaScript(strVal);
}
QByteArray GPSDemo::checkData(QByteArray temp1,const QByteArray &s1,const QByteArray &s2)
{
    QByteArray temp2;    //存放GPS最终数据

    //异常类：无头且变量为空，已丢失头部，数据不可靠，直接返回
    if ((!temp1.contains(s1)) & (temp.isNull()) )
    {
        return 0;
    }
    //第一种：有头无尾，先清空原有内容，再附加
    if ((temp1.contains(s1))&(!temp1.contains(s2)))
    {
        temp.clear();
        temp.append(temp1);
    }
    //第二种：无头无尾且变量已有内容，数据中段部分，继续附加即可
    if ((!temp1.contains(s1))&(!temp1.contains(s2))&(!temp.isNull()))
    {
        temp.append(temp1);
    }
    //第三种：无头有尾且变量已有内容，已完整读取，附加后输出数据，并清空变量
    if ((!temp1.contains(s1))&(temp1.contains(s2))&(!temp.isNull()))
    {
        temp.append(temp1);
        temp2 = temp;
        temp.clear();
        return temp2;
    }
    //第四种：有头有尾（一段完整的内容），先清空原有内容，再附加，然后输出，最后清空变量
    if ((temp1.contains(s1))&(temp1.contains(s2)))
    {
        temp.clear();
        temp.append(temp1);
        temp2 = temp;
        temp.clear();
        return temp2;
    }
    return 0;
}

void  GPSDemo::GPS_DB_PARSE(QByteArray temp3)
{
    char* ch=temp3.data();
    gps_parse(ch,GPS);          //解析数据，接续到数据存入GPS内
    ui->timelineEdit->setText(QString("%1:%2:%3").arg(GPS->D.hour).arg(GPS->D.minute).arg(GPS->D.second));
    ui->latlineEdit->setText(QString("%1").arg(GPS->latitude_ok));
    ui->longlineEdit->setText(QString("%1").arg(GPS->longitude_ok));
    ui->headlineEdit->setText(QString("%1").arg(GPS->high));
//    delete GPS;
}

void GPSDemo::on_quit_clicked()
{
    GPSDemo::deleteLater();//关闭当前窗口
}
