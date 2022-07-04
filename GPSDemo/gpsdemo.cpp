#include "gpsdemo.h"
#include "ui_gpsdemo.h"
#include <QMessageBox>
#include <QList>
#include <QDebug>
#include <QTimer>
#include <QThread>
GPSDemo::GPSDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GPSDemo)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
//    ui->webView->load(QUrl("file:/home/cjf/test/QtBaiduMapApi-master/map.html"));绝对路径
    ui->webView->load(QUrl("file:" + qApp->applicationDirPath() + "/data/baidumap.html"));

    ui->close_port->setEnabled(false);
    //初始化端口
    serialport = new QSerialPort(this);
    portFind();
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
        if(serial_num.open(QIODevice::ReadWrite))
        {
            ui->com->addItem(serial_num.portName());
            serial_num.close();
        }
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
    //qDebug()<<"read";
    //字符串或者十六进制接收
//    if(serialport->bytesAvailable() < 500)
//        return;
//    serialport->flush();

    QByteArray temp3;
    QByteArray buffer;
    buffer = serialport->readAll();

        //存放GPS最终数据
    temp3=checkData(buffer,"$","*");    //连接北斗数据，“$”开始，“*”结束
    if(!temp3.isNull()){    //发送北斗数据
        DBSend(temp3);
    }

  /* if(!buffer.isEmpty())
    {
        QString str = ui->Receive_text_win->toPlainText();
        if(ui->receive_state->currentText() == "string")
        {
            str = tr(buffer);
            ui->Receive_text_win->append(str);
        }else{
           QByteArray temp = buffer.toHex().toUpper();
           str = tr(temp);
           ui->Receive_text_win->append(str);
        }
    }*/

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
    portFind();
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
    QByteArray temp3;    //存放GPS最终数据

    //异常类：无头且变量为空，已丢失头部，数据不可靠，直接返回
    if ((!temp1.contains(s1)) & (temp2.isNull()) )
    {
        return 0;
    }
    //第一种：有头无尾，先清空原有内容，再附加
    if ((temp1.contains(s1))&(!temp1.contains(s2)))
    {
        temp2.clear();
        temp2.append(temp1);
    }
    //第二种：无头无尾且变量已有内容，数据中段部分，继续附加即可
    if ((!temp1.contains(s1))&(!temp1.contains(s2))&(!temp2.isNull()))
    {
        temp2.append(temp1);
    }
    //第三种：无头有尾且变量已有内容，已完整读取，附加后输出数据，并清空变量
    if ((!temp1.contains(s1))&(temp1.contains(s2))&(!temp2.isNull()))
    {
        temp2.append(temp1);
        temp3 = temp2;
        temp2.clear();
        return temp3;
    }
    //第四种：有头有尾（一段完整的内容），先清空原有内容，再附加，然后输出，最后清空变量
    if ((temp1.contains(s1))&(temp1.contains(s2)))
    {
        temp2.clear();
        temp2.append(temp1);
        temp3 = temp2;
        temp2.clear();
        return temp3;
    }
    return 0;
}
void  GPSDemo::DBSend(QByteArray temp3) {

    struct GPS_INFO *GPS=new GPS_INFO;
    char* ch=temp3.data();

    if(temp3.contains("$GNRMC")){
        gps_parse(ch,GPS);          //解析数据，接续到数据存入GPS内
        QString GPSData=QString(temp3);            //GPSData，接收到的数据用于调试
        if(flagDB){                               //表示数据解析正确，进行显示
//            ui->Receive_text_win->setText(GPSData);    //调试北斗数据显示
            QString str = ui->Receive_text_win->toPlainText();
            if(ui->receive_state->currentText() == "string")
            {
                str = tr(temp3);
                ui->Receive_text_win->setText(str);
            }else{
               QByteArray temp = temp3.toHex().toUpper();
               str = tr(temp);
               ui->Receive_text_win->setText(str);
            }

            if(temp3.contains("$GNRMC"))
                show_gps(GPS);          //界面显示
        }
    }
    delete GPS;
}
void GPSDemo::gps_parse(char *line,GPS_INFO *GPS)//将得到的数据解析到GPS中
{
    int tmp,tmp1,tmp2;
    int t;
    char* buf=line;
    flagDB=false;
    t=GetComma(12,buf);//返回第12个逗号的最后位置
    if(t){//包含12个，(12)不为“N”
        //t&&c!='N'
        flagDB=true;
        GPS->D.hour=(buf[7]-'0')*10+(buf[8]-'0');
        GPS->D.minute=(buf[9]-'0')*10+(buf[10]-'0');
        GPS->D.second=(buf[11]-'0')*10+(buf[12]-'0');
        tmp=GetComma(9,buf);      //得到第9个逗号的下一字符序号
        GPS->D.day=(buf[tmp+0]-'0')*10+(buf[tmp+1]-'0');
        GPS->D.month=(buf[tmp+2]-'0')*10+(buf[tmp+3]-'0');
        GPS->D.year=(buf[tmp+4]-'0')*10+(buf[tmp+5]-'0')+2000;

        //------------------------------
        GPS->status=buf[GetComma(2,buf)];     //状态

        tmp1=GetComma(3,buf);
        GPS->latituded=(buf[tmp1+0]-'0')*10+(buf[tmp1+1]-'0');  //纬度度
        GPS->latitudef=get_double_number(&buf[tmp1+2]);  //纬度分
        GPS->latitude=GPS->latituded+(GPS->latitudef)/60;   //纬度
        GPS->NS=buf[GetComma(4,buf)];             //南北纬

        tmp2=GetComma(5,buf);
        GPS->longituded=(buf[tmp2+0]-'0')*100+(buf[tmp2+1]-'0')*10+(buf[tmp2+2]-'0');//经度度
        GPS->longitudef=get_double_number(&(buf[tmp2+3]));//经度分
        GPS->longitude=GPS->longituded+(GPS->longitudef)/60;   //经度
        GPS->EW=buf[GetComma(6,buf)];             //东西经

//        GPS->speed=(buf[GetComma(7,buf)]-'0');   //speed
//        GPS->high=(buf[GetComma(8,buf)]-'0');   //hight
        GPS->speed  = get_double_number(&buf[GetComma(7,buf)]);  //speed
        GPS->high   = get_double_number(&buf[GetComma(8,buf)]);
    }
    else{

    }
}
/*--------------------------------------显示GPS北斗数据------------------------------*/
void GPSDemo::show_gps(GPS_INFO *GPS)
{
//    qDebug()<<(QString("%1-%2-%3").arg(GPS->D.year).arg(GPS->D.month).arg(GPS->D.day));
//    qDebug()<<(QString("%1:%2:%3").arg(GPS->D.hour).arg(GPS->D.minute).arg(GPS->D.second));
//    qDebug()<<(QString("%1 %2").arg(GPS->latitude).arg(GPS->NS));
//    qDebug()<<(QString("%1 %2").arg(GPS->longitude).arg(GPS->EW));
//    qDebug()<<(QString("%1").arg(GPS->status));
//    qDebug()<<(QString("%1").arg(GPS->speed));
//    qDebug()<<(QString("%1").arg(GPS->high));
    if(GPS->D.hour + 8 >=24)
        GPS->D.hour = GPS->D.hour - 24;
    ui->timelineEdit->setText(QString("%1:%2:%3").arg(GPS->D.hour+8).arg(GPS->D.minute).arg(GPS->D.second));
    ui->latlineEdit->setText(QString("%1").arg(GPS->latitude));
    ui->longlineEdit->setText(QString("%1").arg(GPS->longitude));
    ui->headlineEdit->setText(QString("%1").arg(GPS->high));

}
int GPSDemo::GetComma(int num,char *str)
{
    int i,j=0;
    int len=strlen(str);
    for(i=0;i<len;i++)
    {
        if(str[i]==',')j++;
        if(j==num)return i+1;   //返回当前找到的逗号位置的下一个位置
    }
    return 0;
}
double GPSDemo::get_double_number(char *s)
{
    char buf[128];
    int i;
    double rev;
    i=GetComma(1,s);    //得到数据长度
    strncpy(buf,s,i);
    buf[i]=0;           //加字符串结束标志
    rev=atof(buf);      //字符串转float
    return rev;
}
