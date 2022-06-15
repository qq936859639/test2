#include "smarthome.h"
#include "ui_smarthome.h"

#include <QtCore/QDateTime>
#include <QtWidgets/QMessageBox>
#include <QTimer>

SmartHome::SmartHome(QWidget *parent, CameraThread *camerathread) :
    QWidget(parent),
    ui(new Ui::SmartHome)
{
    ui->setupUi(this);

    ui->message_DHT11_data->setEnabled(false);
    ui->Publish_DHT11->setEnabled(false);
    ui->message_PIR_data->setEnabled(false);
    ui->Publish_PIR->setEnabled(false);

    ui->Key_SmartHome->setCheckable(true);
    ui->PIR_data->setCheckable(true);

    pir_flag = 0;

    init_PIR();
    init_DHT11();
    init_LEDB();
    init_OCR();
}
void SmartHome::init_PIR()
{
    m_client_pir = new QMqttClient(this);
    m_client_pir->setHostname(ui->host_PIR_data->text());
    m_client_pir->setPort(ui->port_PIR_data->value());
    m_client_pir->setUsername(ui->user_PIR_data->text());
    m_client_pir->setPassword(ui->password_PIR_data->text());
//    m_client_pir->setClientId("");

    connect(m_client_pir, &QMqttClient::stateChanged, this, &SmartHome::updateLogStateChange);
    connect(m_client_pir, &QMqttClient::disconnected, this, &SmartHome::brokerDisconnected);

    connect(m_client_pir, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" Received Topic: ")
                    + topic.name()
                    + QLatin1String(" Message: ")
                    + message
                    + QLatin1Char('\n');
        ui->editLog_PIR->insertPlainText(content);

        cJSON *proot,*parray;
        proot = cJSON_Parse(message);
        if(proot != NULL)
        {
            if((parray = cJSON_GetObjectItem(proot, "reported")) != NULL)//判断是否有reported
            {
                if(cJSON_GetObjectItem(parray,"RE200B") != NULL)
                {
                    //int data = cJSON_GetObjectItem(parray,"RE200B")->valueint;
                    char *data = cJSON_GetObjectItem(parray, "RE200B")->valuestring;//字符串
                    qDebug()<<"cjf"<<data;//<<":"<<cJSON_GetObjectItem(parray, "RE200B")->valuestring;
                    ui->state_PIR_data->setText(data);
//                    ui->PIR_data->setText(data);
                    if(pir_flag ==1){
                        if(ui->state_PIR_data->text() == "abnormal")
                            on_BUZZER_ON_clicked();
                        else
                            on_BUZZER_OFF_clicked();
                    }
                }
            }
            //获取收到的时间
            /*
            if((parray = cJSON_GetObjectItem(proot, "lastUpdatedTime")) != NULL)//判断是否有lastUpdatedTime
            {
                if((parray = cJSON_GetObjectItem(parray, "reported")) != NULL)//判断是否有reported
                {
                    if(cJSON_GetObjectItem(parray,"RE200B") != NULL)
                    {
                        double buzzer_data = cJSON_GetObjectItem(parray,"RE200B")->valuedouble;
                        QDateTime time = QDateTime::fromMSecsSinceEpoch(buzzer_data); //时间戳-毫秒级
                        qDebug() << "strStartTime0: " << time;
                        QString strStartTime = time.toString("yyyy-MM-dd hh:mm:ss");
                        qDebug() << "strStartTime1: " << strStartTime;
                        ui->label->setText(strStartTime);
                    }
                }
            }*/
        }
        cJSON_Delete(proot);
        //**********
    });

    connect(m_client_pir, &QMqttClient::pingResponseReceived, this, [this]() {
        ui->Ping_PIR->setEnabled(true);
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" PingResponse")
                    + QLatin1Char('\n');
        ui->editLog_PIR->insertPlainText(content);
    });

    connect(ui->host_PIR_data, &QLineEdit::textChanged, m_client_pir, &QMqttClient::setHostname);
    connect(ui->port_PIR_data, QOverload<int>::of(&QSpinBox::valueChanged), this, &SmartHome::setClientPort);
    updateLogStateChange();
}

void SmartHome::init_DHT11()
{
    m_client_dht11 = new QMqttClient(this);
    m_client_dht11->setHostname(ui->host_DHT11_data->text());
    m_client_dht11->setPort(ui->port_DHT11_data->value());
    m_client_dht11->setUsername(ui->user_DHT11_data->text());
    m_client_dht11->setPassword(ui->password_DHT11_data->text());
//    m_client_dht11->setClientId("");

    connect(m_client_dht11, &QMqttClient::stateChanged, this, &SmartHome::updateLogStateChange_dht11);
    connect(m_client_dht11, &QMqttClient::disconnected, this, &SmartHome::brokerDisconnected_dht11);

    connect(m_client_dht11, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" Received Topic: ")
                    + topic.name()
                    + QLatin1String(" Message: ")
                    + message
                    + QLatin1Char('\n');
        ui->editLog_DHT11->insertPlainText(content);

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
                    ui->DHT11_Temp_data->setText(QString::number(buzzer_data,'f',1));
                }
                if(cJSON_GetObjectItem(parray,"DHT11_Humi") != NULL)
                {
                    buzzer_data = cJSON_GetObjectItem(parray, "DHT11_Humi")->valuedouble;//字符串
                    ui->state_DHT11_humi_data->setText(QString::number(buzzer_data,'f',1));
                    ui->DHT11_Humi_data->setText(QString::number(buzzer_data,'f',1));
                }
            }
        }
        cJSON_Delete(proot);
    });

    connect(m_client_dht11, &QMqttClient::pingResponseReceived, this, [this]() {
        ui->Ping_DHT11->setEnabled(true);
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" PingResponse")
                    + QLatin1Char('\n');
        ui->editLog_DHT11->insertPlainText(content);
    });

    connect(ui->host_DHT11_data, &QLineEdit::textChanged, m_client_dht11, &QMqttClient::setHostname);
    connect(ui->port_DHT11_data, QOverload<int>::of(&QSpinBox::valueChanged), this, &SmartHome::setClientPort_dht11);
    updateLogStateChange_dht11();
}

void SmartHome::init_LEDB()
{
    m_client_ledb = new QMqttClient(this);
    m_client_ledb->setHostname(ui->host_LEDB_data->text());
    m_client_ledb->setPort(ui->port_LEDB_data->value());
    m_client_ledb->setUsername(ui->user_LEDB_data->text());
    m_client_ledb->setPassword(ui->password_LEDB_data->text());
//    m_client_ledb->setClientId("");

    connect(m_client_ledb, &QMqttClient::stateChanged, this, &SmartHome::updateLogStateChange_ledb);
    connect(m_client_ledb, &QMqttClient::disconnected, this, &SmartHome::brokerDisconnected_ledb);

    connect(m_client_ledb, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" Received Topic: ")
                    + topic.name()
                    + QLatin1String(" Message: ")
                    + message
                    + QLatin1Char('\n');
        ui->editLog_LEDB->insertPlainText(content);

        cJSON *proot,*parray;
        proot = cJSON_Parse(message);
        if(proot != NULL)
        {
            if((parray = cJSON_GetObjectItem(proot, "reported")) != NULL)//判断是否有reported
            {
                if(cJSON_GetObjectItem(parray,"LED") != NULL)
                {
                    char *led_data = cJSON_GetObjectItem(parray, "LED")->valuestring;//字符串
                    ui->state_LED_data->setText(led_data);
                    if(ui->state_LED_data->text() == "ledon")
                        ui->LED_data->setStyleSheet("background-color: rgba(213, 31, 31, 0);");
                    else
                        ui->LED_data->setStyleSheet("background-color: rgba(100, 100, 31, 0);");
                }
            }
            if((parray = cJSON_GetObjectItem(proot, "reported")) != NULL)//判断是否有reported
            {
                if(cJSON_GetObjectItem(parray,"BUZZER") != NULL)
                {
                    char *buzzer_data = cJSON_GetObjectItem(parray, "BUZZER")->valuestring;//字符串
                    ui->state_BUZZER_data->setText(buzzer_data);
                    if(ui->state_BUZZER_data->text() == "buzzeron")
                        ui->BUZZER_data->setStyleSheet("background-color: rgba(213, 31, 31, 0);");
                    else
                        ui->BUZZER_data->setStyleSheet("background-color: rgba(100, 100, 31, 0);");
                }
            }
        }
        cJSON_Delete(proot);
    });

    connect(m_client_ledb, &QMqttClient::pingResponseReceived, this, [this]() {
        ui->Ping_LEDB->setEnabled(true);
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" PingResponse")
                    + QLatin1Char('\n');
        ui->editLog_LEDB->insertPlainText(content);
    });

    connect(ui->host_LEDB_data, &QLineEdit::textChanged, m_client_ledb, &QMqttClient::setHostname);
    connect(ui->port_LEDB_data, QOverload<int>::of(&QSpinBox::valueChanged), this, &SmartHome::setClientPort_ledb);
    updateLogStateChange_ledb();
}

void SmartHome::init_OCR()
{
    m_client_ocr = new QMqttClient(this);
    m_client_ocr->setHostname(ui->host_OCR_data->text());
    m_client_ocr->setPort(ui->port_OCR_data->value());
    m_client_ocr->setUsername(ui->user_OCR_data->text());
    m_client_ocr->setPassword(ui->password_OCR_data->text());
//    m_client_ocr->setClientId("");

    connect(m_client_ocr, &QMqttClient::stateChanged, this, &SmartHome::updateLogStateChange_ocr);
    connect(m_client_ocr, &QMqttClient::disconnected, this, &SmartHome::brokerDisconnected_ocr);

    connect(m_client_ocr, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" Received Topic: ")
                    + topic.name()
                    + QLatin1String(" Message: ")
                    + message
                    + QLatin1Char('\n');
        ui->editLog_OCR->insertPlainText(content);

        cJSON *proot,*parray;
        proot = cJSON_Parse(message);
        if(proot != NULL)
        {
            if((parray = cJSON_GetObjectItem(proot, "reported")) != NULL)//判断是否有reported
            {
                if(cJSON_GetObjectItem(parray,"OCRelay") != NULL)
                {
                    char *ocr_data = cJSON_GetObjectItem(parray, "OCRelay")->valuestring;//字符串
                    ui->state_OCR_data->setText(ocr_data);
                    if(ui->state_OCR_data->text() == "OCRelay_On")
                        ui->OCR_data->setStyleSheet("background-color: rgba(213, 31, 31, 0);");
                    else
                        ui->OCR_data->setStyleSheet("background-color: rgba(100, 100, 31, 0);");
                }
            }
        }
        cJSON_Delete(proot);

    });

    connect(m_client_ocr, &QMqttClient::pingResponseReceived, this, [this]() {
        ui->Ping_OCR->setEnabled(true);
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" PingResponse")
                    + QLatin1Char('\n');
        ui->editLog_OCR->insertPlainText(content);
    });

    connect(ui->host_OCR_data, &QLineEdit::textChanged, m_client_ocr, &QMqttClient::setHostname);
    connect(ui->port_OCR_data, QOverload<int>::of(&QSpinBox::valueChanged), this, &SmartHome::setClientPort_ocr);
    updateLogStateChange_ocr();
}

SmartHome::~SmartHome()
{
    delete ui;
}

//热释红外操作代码
void SmartHome::on_Connect_PIR_clicked()
{
    if (m_client_pir->state() == QMqttClient::Disconnected) {
        ui->host_PIR_data->setEnabled(false);
        ui->port_PIR_data->setEnabled(false);
        ui->user_PIR_data->setEnabled(false);
        ui->password_PIR_data->setEnabled(false);
        ui->Connect_PIR->setText(tr("Disconnect"));
        m_client_pir->connectToHost();
    } else {
        ui->host_PIR_data->setEnabled(true);
        ui->port_PIR_data->setEnabled(true);
        ui->user_PIR_data->setEnabled(true);
        ui->password_PIR_data->setEnabled(true);
        ui->Connect_PIR->setText(tr("Connect"));
        m_client_pir->disconnectFromHost();
    }
}
void SmartHome::updateLogStateChange()
{
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_client_pir->state())
                    + QLatin1Char('\n');
    ui->editLog_PIR->insertPlainText(content);
}

void SmartHome::brokerDisconnected()
{
    ui->host_PIR_data->setEnabled(true);
    ui->port_PIR_data->setEnabled(true);
    ui->Connect_PIR->setText(tr("Connect"));
}
void SmartHome::setClientPort(int p)
{
    m_client_pir->setPort(p);
}

void SmartHome::on_Publish_PIR_clicked()
{
    if (m_client_pir->publish(ui->topic_PIR_data->text(), ui->message_PIR_data->text().toUtf8()) == -1)
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
}

void SmartHome::on_Subscribe_PIR_clicked()
{
    auto subscription = m_client_pir->subscribe(ui->topic_PIR_data->text());
    if (!subscription) {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
}

void SmartHome::on_Ping_PIR_clicked()
{
    ui->Ping_PIR->setEnabled(false);
    m_client_pir->requestPing();
}
//热释红外结束

//温湿度操作代码
void SmartHome::on_Connect_DHT11_clicked()
{
    if (m_client_dht11->state() == QMqttClient::Disconnected) {
        ui->host_DHT11_data->setEnabled(false);
        ui->port_DHT11_data->setEnabled(false);
        ui->user_DHT11_data->setEnabled(false);
        ui->password_DHT11_data->setEnabled(false);
        ui->Connect_DHT11->setText(tr("Disconnect"));
        m_client_dht11->connectToHost();
    } else {
        ui->host_DHT11_data->setEnabled(true);
        ui->port_DHT11_data->setEnabled(true);
        ui->user_DHT11_data->setEnabled(true);
        ui->password_DHT11_data->setEnabled(true);
        ui->Connect_DHT11->setText(tr("Connect"));
        m_client_dht11->disconnectFromHost();
    }
}
void SmartHome::updateLogStateChange_dht11()
{
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_client_dht11->state())
                    + QLatin1Char('\n');
    ui->editLog_DHT11->insertPlainText(content);
}

void SmartHome::brokerDisconnected_dht11()
{
    ui->host_DHT11_data->setEnabled(true);
    ui->port_DHT11_data->setEnabled(true);
    ui->Connect_DHT11->setText(tr("Connect"));
}
void SmartHome::setClientPort_dht11(int p)
{
    m_client_dht11->setPort(p);
}

void SmartHome::on_Subscribe_DHT11_clicked()
{
    auto subscription = m_client_dht11->subscribe(ui->topic_DHT11_data->text());
    if (!subscription) {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
}

void SmartHome::on_Ping_DHT11_clicked()
{
    ui->Ping_DHT11->setEnabled(false);
    m_client_dht11->requestPing();
}

//LED_BUZZER操作代码
void SmartHome::on_Connect_LEDB_clicked()
{
    if (m_client_ledb->state() == QMqttClient::Disconnected) {
        ui->host_LEDB_data->setEnabled(false);
        ui->port_LEDB_data->setEnabled(false);
        ui->user_LEDB_data->setEnabled(false);
        ui->password_LEDB_data->setEnabled(false);
        ui->Connect_LEDB->setText(tr("Disconnect"));

        ui->LED_ON->setEnabled(true);
        ui->LED_OFF->setEnabled(true);
        ui->BUZZER_ON->setEnabled(true);
        ui->BUZZER_OFF->setEnabled(true);
        m_client_ledb->connectToHost();
    } else {
        ui->host_LEDB_data->setEnabled(true);
        ui->port_LEDB_data->setEnabled(true);
        ui->user_LEDB_data->setEnabled(true);
        ui->password_LEDB_data->setEnabled(true);
        ui->Connect_LEDB->setText(tr("Connect"));

        ui->LED_ON->setEnabled(false);
        ui->LED_OFF->setEnabled(false);
        ui->BUZZER_ON->setEnabled(false);
        ui->BUZZER_OFF->setEnabled(false);
        m_client_ledb->disconnectFromHost();
    }
}
void SmartHome::updateLogStateChange_ledb()
{
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_client_ledb->state())
                    + QLatin1Char('\n');
    ui->editLog_LEDB->insertPlainText(content);
}

void SmartHome::brokerDisconnected_ledb()
{
    ui->host_LEDB_data->setEnabled(true);
    ui->port_LEDB_data->setEnabled(true);
    ui->Connect_LEDB->setText(tr("Connect"));
}
void SmartHome::setClientPort_ledb(int p)
{
    m_client_ledb->setPort(p);
}

void SmartHome::on_Subscribe_LEDB_clicked()
{
    ui->topic_LEDB_data->setText("$baidu/iot/shadow/LED/update/accepted");
    auto subscription = m_client_ledb->subscribe(ui->topic_LEDB_data->text());
    if (!subscription) {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
}

void SmartHome::on_Publish_LEDB_clicked()
{
    ui->topic_LEDB_data->setText("$baidu/iot/shadow/LED/update");
    if (m_client_ledb->publish(ui->topic_LEDB_data->text(), ui->message_LEDB_data->text().toUtf8()) == -1)
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
}

void SmartHome::on_Ping_LEDB_clicked()
{
    ui->Ping_LEDB->setEnabled(false);
    m_client_ledb->requestPing();
}

void SmartHome::on_LED_ON_clicked()
{
    QString topic ="$baidu/iot/shadow/LED/update";//百度LED主题地址
    QString data  ="{\"desired\":{\"LED\":\"on\"}}";
    m_client_ledb->publish(topic,data.toUtf8());
}

void SmartHome::on_LED_OFF_clicked()
{
    QString topic ="$baidu/iot/shadow/LED/update";//百度LED主题地址
    QString data  ="{\"desired\":{\"LED\":\"off\"}}";
    m_client_ledb->publish(topic,data.toUtf8());
}

void SmartHome::on_BUZZER_ON_clicked()
{
    QString topic ="$baidu/iot/shadow/LED/update";//百度LED主题地址
    QString data  ="{\"desired\":{\"BUZZER\":\"on\"}}";
    m_client_ledb->publish(topic,data.toUtf8());
}

void SmartHome::on_BUZZER_OFF_clicked()
{
    QString topic ="$baidu/iot/shadow/LED/update";//百度LED主题地址
    QString data  ="{\"desired\":{\"BUZZER\":\"off\"}}";
    m_client_ledb->publish(topic,data.toUtf8());
}

//继电器操作代码
void SmartHome::on_Connect_OCR_clicked()
{
    if (m_client_ocr->state() == QMqttClient::Disconnected) {
        ui->host_OCR_data->setEnabled(false);
        ui->port_OCR_data->setEnabled(false);
        ui->user_OCR_data->setEnabled(false);
        ui->password_OCR_data->setEnabled(false);
        ui->Connect_OCR->setText(tr("Disconnect"));
        ui->OCR_ON->setEnabled(true);
        ui->OCR_OFF->setEnabled(true);

        m_client_ocr->connectToHost();
    } else {
        ui->host_OCR_data->setEnabled(true);
        ui->port_OCR_data->setEnabled(true);
        ui->user_OCR_data->setEnabled(true);
        ui->password_OCR_data->setEnabled(true);
        ui->Connect_OCR->setText(tr("Connect"));
        ui->OCR_ON->setEnabled(false);
        ui->OCR_OFF->setEnabled(false);

        m_client_ocr->disconnectFromHost();
    }
}
void SmartHome::updateLogStateChange_ocr()
{
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_client_ocr->state())
                    + QLatin1Char('\n');
    ui->editLog_OCR->insertPlainText(content);
}

void SmartHome::brokerDisconnected_ocr()
{
    ui->host_OCR_data->setEnabled(true);
    ui->port_OCR_data->setEnabled(true);
    ui->Connect_OCR->setText(tr("Connect"));
}

void SmartHome::setClientPort_ocr(int p)
{
    m_client_ocr->setPort(p);
}

void SmartHome::on_Subscribe_OCR_clicked()
{
    ui->topic_OCR_data->setText("$baidu/iot/shadow/OCRelay/update/accepted");
    auto subscription = m_client_ocr->subscribe(ui->topic_OCR_data->text());
    if (!subscription) {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
}

void SmartHome::on_Publish_OCR_clicked()
{
    ui->topic_OCR_data->setText("$baidu/iot/shadow/OCRelay/update");
    if (m_client_ocr->publish(ui->topic_OCR_data->text(), ui->message_OCR_data->text().toUtf8()) == -1)
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
}

void SmartHome::on_Ping_OCR_clicked()
{
    ui->Ping_OCR->setEnabled(false);
    m_client_ocr->requestPing();
}

void SmartHome::on_OCR_ON_clicked()
{
    QString topic ="$baidu/iot/shadow/OCRelay/update";//百度OCR主题地址
    QString data  ="{\"desired\":{\"OCRelay\":\"on\"}}";
    m_client_ocr->publish(topic,data.toUtf8());
}

void SmartHome::on_OCR_OFF_clicked()
{
    QString topic ="$baidu/iot/shadow/OCRelay/update";//百度OCR主题地址
    QString data  ="{\"desired\":{\"OCRelay\":\"off\"}}";
    m_client_ocr->publish(topic,data.toUtf8());
}

void SmartHome::on_Key_SmartHome_clicked()
{
    if(ui->Key_SmartHome->isChecked())//按钮按下操作
    {
        m_client_ocr->disconnectFromHost();
        m_client_pir->disconnectFromHost();
        m_client_dht11->disconnectFromHost();
        m_client_ledb->disconnectFromHost();

        on_Connect_OCR_clicked();
        on_Connect_LEDB_clicked();
        on_Connect_DHT11_clicked();
        on_Connect_PIR_clicked();
        ui->Key_SmartHome->setText(tr("断开"));
        QTimer::singleShot(1000,this,SLOT(QTimer_Subscribe()));//延时1秒订阅主题
        ui->Connect_OCR->setEnabled(false);
        ui->Connect_PIR->setEnabled(false);
        ui->Connect_DHT11->setEnabled(false);
        ui->Connect_LEDB->setEnabled(false);

    }else{
        on_Connect_OCR_clicked();
        on_Connect_PIR_clicked();
        on_Connect_DHT11_clicked();
        on_Connect_LEDB_clicked();
        ui->Key_SmartHome->setText(tr("一键连接"));
        ui->Connect_OCR->setEnabled(true);
        ui->Connect_PIR->setEnabled(true);
        ui->Connect_DHT11->setEnabled(true);
        ui->Connect_LEDB->setEnabled(true);
    }
}

void SmartHome::QTimer_Subscribe()
{
    on_Subscribe_PIR_clicked();
    on_Subscribe_DHT11_clicked();
    on_Subscribe_LEDB_clicked();
    on_Subscribe_OCR_clicked();
}
//关闭窗口
void SmartHome::on_Quit_SmartHome_clicked()
{
//    QApplication::quit();//关闭所有窗口
    SmartHome::deleteLater();//关闭当前窗口
}

void SmartHome::on_LED_data_clicked()
{
    if(ui->state_LED_data->text() =="ledon"){
        on_LED_OFF_clicked();
    }
    else{
        on_LED_ON_clicked();
    }
}

void SmartHome::on_BUZZER_data_clicked()
{
    if(ui->state_BUZZER_data->text() =="buzzeron"){
        on_BUZZER_OFF_clicked();
    }
    else{
        on_BUZZER_ON_clicked();
    }
}

void SmartHome::on_PIR_data_clicked()
{
    if(ui->PIR_data->isChecked())//按钮按下操作
    {
        pir_flag = 1;
        ui->PIR_data->setStyleSheet("background-color: rgba(213, 31, 31, 0);");
    }else{
        pir_flag = 0;
        ui->PIR_data->setStyleSheet("background-color: rgba(100, 101, 31, 0);");
    }
}

void SmartHome::on_OCR_data_clicked()
{
    if(ui->state_OCR_data->text() =="OCRelay_On"){
        on_OCR_OFF_clicked();
    }
    else{
        on_OCR_ON_clicked();
    }
}
