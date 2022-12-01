#include "smarthome.h"
#include "ui_smarthome.h"

#include <QtCore/QDateTime>
#include <QtWidgets/QMessageBox>
#include <QTimer>
#include <QSound>//声音
#include <string.h>

#include <QFileInfo>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDir>
SmartHome::SmartHome(QWidget *parent, CameraThread *camerathread) :
    QWidget(parent),
    ui(new Ui::SmartHome)
{
    ui->setupUi(this);

    ui->Key_SmartHome->setCheckable(true);
    ui->PIR_data->setCheckable(true);

    pir_flag = 0;
    play_flag = 0;

    this->cameraThread = camerathread;
//    string xmlPath="./data/haarcascade_frontalface_alt.xml";
//    string xmlPath="./data/haarcascade_frontalface_default.xml";
//    string xmlPath="./data/haarcascade_frontalface_alt2.xml";
//    if(!ccf.load(xmlPath))   //加载训练文件
//    {
//        perror("不能加载指定的xml文件");
//    }
    hidmic = new HIDMICDEMO;
    if(hidmic->hidmic_init()==0){
        ui->speech->setDisabled(false);
    }else{
        ui->speech->setDisabled(true);
    }

    faceUtils = FaceUtils::getInstance();
    connect(faceUtils, SIGNAL(startTrain()), this, SLOT(startTrainSlot()));
    connect(faceUtils, SIGNAL(finishTrain(bool)), this, SLOT(finishTrainSlot(bool)));
//    faceUtils->startTrainFace();
    qsound_master = new QSound("./mp3/master_visit.wav", this);
    qsound_guest = new QSound("./mp3/guests_visit.wav", this);
    process = new QProcess(this);

    ui->fisherfacesRb->setVisible(false);
    get_ip();
    init_DHT11();
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

                if(cJSON_GetObjectItem(parray,"RE200B") != NULL)
                {
                    char *data = cJSON_GetObjectItem(parray, "RE200B")->valuestring;//字符串
                    ui->state_PIR_data->setText(data);
                    if(pir_flag ==1){
                        if(ui->state_PIR_data->text() == "abnormal!")
                            on_BUZZER_ON_clicked();
                        else
                            on_BUZZER_OFF_clicked();
                    }
                }

                if(cJSON_GetObjectItem(parray,"LED") != NULL)
                {
                    char *led_data = cJSON_GetObjectItem(parray, "LED")->valuestring;//字符串
                    ui->state_LED_data->setText(led_data);
                    if(ui->state_LED_data->text() == "ledon")
                    {
                        ui->LED_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/led_on.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/led_on.png);}");
                        ui->label->setStyleSheet("border-image: url(:/image/res/image/smarthome_1.jpg);");
                    }
                    else{
                        ui->LED_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/led_off.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/led_off.png);}");
                        ui->label->setStyleSheet("border-image: url(:/image/res/image/smarthome_0.jpg);");
                    }
                }
                if(cJSON_GetObjectItem(parray,"BUZZER") != NULL)
                {
                    char *buzzer_data = cJSON_GetObjectItem(parray, "BUZZER")->valuestring;//字符串
                    ui->state_BUZZER_data->setText(buzzer_data);
                    if(ui->state_BUZZER_data->text() == "buzzeron"){
                        ui->BUZZER_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/buzzer_on.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/buzzer_on.png);}");
                    }
                    else{
                        ui->BUZZER_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/buzzer_off.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/buzzer_off.png);}");
                    }
                }

                if(cJSON_GetObjectItem(parray,"OCRelay") != NULL)
                {
                    char *ocr_data = cJSON_GetObjectItem(parray, "OCRelay")->valuestring;//字符串
                    ui->state_OCR_data->setText(ocr_data);
                    if(ui->state_OCR_data->text() == "OCRelay_On"){
                        ui->OCR_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/tv_on.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/tv_on.png);}");
                        ui->TV->setVisible(true);
                    }
                    else{
                        ui->OCR_data->setStyleSheet("QPushButton{border-image: url(:/image/res/image/tv_off.png);}"
                                                    "QPushButton:pressed{border-width:3px;"
                                                    "border-image: url(:/image/res/image/tv_off.png);}");
                        ui->TV->setVisible(false);
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

SmartHome::~SmartHome()
{
    delete ui;
}
void SmartHome::closeEvent(QCloseEvent *event)
{
    disconnect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(SmartHome_videoDisplay(QImage)));
    if(ui->speech->isEnabled())
        hidmic->hidmic_close();//关闭麦克风
}

//温湿度操作代码
void SmartHome::on_Connect_DHT11_clicked()
{
    if (m_client_dht11->state() == QMqttClient::Disconnected) {

        m_client_dht11->setHostname(ui->host_DHT11_data->text());
        m_client_dht11->setPort(ui->port_DHT11_data->value());
        m_client_dht11->setUsername(ui->user_DHT11_data->text());
        m_client_dht11->setPassword(ui->password_DHT11_data->text());

        ui->host_DHT11_data->setEnabled(false);
        ui->port_DHT11_data->setEnabled(false);
        ui->user_DHT11_data->setEnabled(false);
        ui->password_DHT11_data->setEnabled(false);
        ui->Connect_DHT11->setText(tr("Disconnect"));
        m_client_dht11->connectToHost();
        save_ip();
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
    auto subscription = m_client_dht11->subscribe(ui->SubTopic_DHT11_data->text());
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

//LED蜂鸣器操作代码
void SmartHome::on_LED_ON_clicked()
{
    QString topic = ui->PubTopic_DHT11_data->text();//百度LED主题地址
    QString data  ="{\"desired\":{\"LED\":\"on\"}}";
    m_client_dht11->publish(topic,data.toUtf8());
}

void SmartHome::on_LED_OFF_clicked()
{
    QString topic = ui->PubTopic_DHT11_data->text();//百度LED主题地址
    QString data  ="{\"desired\":{\"LED\":\"off\"}}";
    m_client_dht11->publish(topic,data.toUtf8());
}

void SmartHome::on_BUZZER_ON_clicked()
{
    QString topic = ui->PubTopic_DHT11_data->text();//百度LED主题地址
    QString data  ="{\"desired\":{\"BUZZER\":\"on\"}}";
    m_client_dht11->publish(topic,data.toUtf8());
}

void SmartHome::on_BUZZER_OFF_clicked()
{
    QString topic = ui->PubTopic_DHT11_data->text();//百度LED主题地址
    QString data  ="{\"desired\":{\"BUZZER\":\"off\"}}";
    m_client_dht11->publish(topic,data.toUtf8());
}

//继电器操作代码
void SmartHome::on_OCR_ON_clicked()
{
    QString topic = ui->PubTopic_DHT11_data->text();//百度OCR主题地址
    QString data  ="{\"desired\":{\"OCRelay\":\"on\"}}";
    m_client_dht11->publish(topic,data.toUtf8());
}

void SmartHome::on_OCR_OFF_clicked()
{
    QString topic = ui->PubTopic_DHT11_data->text();//百度OCR主题地址
    QString data  ="{\"desired\":{\"OCRelay\":\"off\"}}";
    m_client_dht11->publish(topic,data.toUtf8());
}

void SmartHome::on_Key_SmartHome_clicked()
{
    if(ui->Key_SmartHome->isChecked())//按钮按下操作
    {
        if(!faceUtils->faceHasTrain())
            faceUtils->startTrainFace();

        m_client_dht11->disconnectFromHost();

        on_Connect_DHT11_clicked();

        ui->Key_SmartHome->setText(tr("断开"));
        QTimer::singleShot(1000,this,SLOT(QTimer_Subscribe()));//延时1秒订阅主题

        ui->Connect_DHT11->setEnabled(false);

        connect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(SmartHome_videoDisplay(QImage)));

    }else{
        on_Connect_DHT11_clicked();

        ui->Key_SmartHome->setText(tr("一键连接"));

        ui->Connect_DHT11->setEnabled(true);

        disconnect(cameraThread, SIGNAL(Collect_complete(QImage)),this,SLOT(SmartHome_videoDisplay(QImage)));
    }
}

void SmartHome::QTimer_Subscribe()
{
      on_Subscribe_DHT11_clicked();
}
//关闭窗口
void SmartHome::on_Quit_SmartHome_clicked()
{
//    QApplication::quit();//关闭所有窗口
    if(ui->speech->isEnabled())
        hidmic->hidmic_close();//关闭麦克风
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
        ui->PIR_data->setStyleSheet("border-image:url(:/image/res/image/security_on.png);");
    }else{
        pir_flag = 0;
        ui->PIR_data->setStyleSheet("border-image:url(:/image/res/image/security_off.png);");
    }
}

void SmartHome::on_OCR_data_clicked()
{
    if(ui->state_OCR_data->text() == "OCRelay_On"){
        on_OCR_OFF_clicked();
    }
    else{
        on_OCR_ON_clicked();
    }
}
QImage SmartHome::Mat2QImage(const Mat &mat)
{
    switch (mat.type())
    {
        // 8-bit, 4 channel
        case CV_8UC4:
        {
            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
            return image;
        }

        // 8-bit, 3 channel
        case CV_8UC3:
        {
            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            return image.rgbSwapped();
        }

        // 8-bit, 1 channel
        case CV_8UC1:
        {
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
            QImage image(mat.data, mat.cols, mat.rows, int(mat.step), QImage::Format_Grayscale8);
#else
            QVector<QRgb> sColorTable;
            if (sColorTable.isEmpty())
            {
                sColorTable.resize( 256 );

                for ( int i =0; i <256; ++i )
                {
                    sColorTable[i] = qRgb( i, i, i );
                }
            }

            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8 );
            image.setColorTable(sColorTable);
#endif

            return image;
        }

        // wrong
        default:
            qDebug() << "ERROR: Mat could not be converted to QImage.";
            break;
    }
    return QImage();
}

Mat SmartHome::QImage2Mat(const QImage& image)
{
    cv::Mat mat,mat_out;    //如果把mat_out变更为mat，那么参数image的r/b被调换。即使被const修饰，依然被更改，比较诡异。参数变为值传递，也依然被更改。
    switch (image.format())
    {
    case QImage::Format_RGB32:                              //一般Qt读入本地彩色图后为此格式
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat_out, cv::COLOR_BGRA2BGR);     //转3通道，OpenCV一般用3通道的
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat_out, cv::COLOR_RGB2BGR);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat_out;
}
void SmartHome::SmartHome_videoDisplay(const QImage image)
{
    QImage image1 = image.copy();
    image1 = image1.mirrored(false, false);

    Mat img = this->QImage2Mat(image1);
    cv::resize(img,img,Size(320, 240));

    videos_times++;
    if(ui->Key_SmartHome->isChecked()&&videos_times>1){
        img.copyTo(image_tmp);
        videos_times = 0;
        //img = FaceRecognition(img);            //人脸识别
        vector<Rect> faces= faceUtils->faceDetection(img);

        foreach (Rect face, faces)
            rectangle((Mat)img, face, Scalar(0, 0, 255), 2, 8);

        foreach (Rect face, faces)
        {
            Mat imageRIO = img(face);
            cv::resize(imageRIO, imageRIO, Size(92, 112));

            RecognizerModel recognizerModel = PCA_MODEL;
            if(ui->eigenfacesRb->isChecked())
                recognizerModel = PCA_MODEL;
            else if(ui->fisherfacesRb->isChecked())
                recognizerModel = FISHER_MODEL;
            else if(ui->lbphRb->isChecked())
                recognizerModel = LBPH_MODEL;
            int result = faceUtils->faceRecognition(imageRIO, recognizerModel);
            cout << result << endl;
            if(0<result &&result<6){
                if(qsound_play_flag ==0){
                    if(qsound_master->isFinished())
                        qsound_master->play();
                }
            }else if(result==-1){
            }else{
                if(qsound_play_flag ==0){
                    if(qsound_guest->isFinished())
                        qsound_guest->play();
                }
            }

            switch(result)
            {
            case 1:
                cv::putText(img, "1", Point(face.x, face.y),
                        FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255));
                break;
            case 2:
                cv::putText(img, "2", Point(face.x, face.y),
                        FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255));
                break;
            case 3:
                cv::putText(img, "3", Point(face.x, face.y),
                        FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255));
                break;
            case 4:
                cv::putText(img, "4", Point(face.x, face.y),
                        FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255));
                break;
            case 5:
                cv::putText(img, "5", Point(face.x, face.y),
                        FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255));
                break;
            default:
                cv::putText(img, "?", Point(face.x, face.y),
                        FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255));
            }
            QImage qimg_rio = this->Mat2QImage(imageRIO);
            QPixmap pixmap_rio = QPixmap::fromImage(qimg_rio);
            ui->video_1->setPixmap(pixmap_rio.scaled(ui->video_1->size(),Qt::IgnoreAspectRatio));//Qt::SmoothTransformation 保持比例
        }
        QImage qimg = this->Mat2QImage(img);
        QPixmap pixmap = QPixmap::fromImage(qimg);
        ui->cameraShowLabel->setPixmap(pixmap.scaled(ui->cameraShowLabel->size(),Qt::IgnoreAspectRatio));//Qt::SmoothTransformation 保持比例
        ui->video_0->setPixmap(pixmap.scaled(ui->video_0->size(),Qt::IgnoreAspectRatio));//Qt::SmoothTransformation 保持比例
    }
}

Mat SmartHome::FaceRecognition(const Mat &mat)
{
    vector<Rect> faces;  //创建一个容器保存检测出来的脸
    Mat img1, gray;

    img1 = mat;
//    cv::resize(img1,img1,Size(320, 240));

    cvtColor(img1, gray, COLOR_BGR2GRAY); //转换成灰度图，因为harr特征从灰度图中提取
    equalizeHist(gray,gray);  //直方图均衡行
    ccf.detectMultiScale(gray,faces,1.1,3,0,Size(50,50),Size(200,200)); //检测人脸
    for(vector<Rect>::const_iterator iter=faces.begin();iter!=faces.end();iter++)
    {
        Mat img2, m;
        img1.copyTo(img2);

        rectangle(img1,*iter,Scalar(0,0,255),2,10);//画出脸部矩形
        if(iter->x-5 > 0 && iter->y-5 >0){
            cv::Rect area(iter->x-5, iter->y-5, iter->width+5, iter->height+5);//需要裁减的矩形区域
            m = img2(area);//img2(*iter).copyTo(m);
        }

        QImage qimg = this->Mat2QImage(m);
        QPixmap pixmap = QPixmap::fromImage(qimg);
        if(!pixmap.isNull()){
            ui->video_1->setPixmap(pixmap.scaled(ui->video_1->size(),Qt::IgnoreAspectRatio));//Qt::SmoothTransformation 保持比例
            if(play_flag ==0){
                QSound *success = new QSound("./mp3/guests_visit.wav", this);
                success->play();
                play_flag = 1;
                QTimer::singleShot(3000,this,SLOT(SmartHome_Play()));//延时3秒
            }
        }
    }
    return img1;
}
void SmartHome::SmartHome_Play()
{
    play_flag = 0;
}

void SmartHome::on_speech_pressed()
{
    ui->speech->setText("松开识别");
    //开始录音-1.USB麦克或自带的麦克
//    audio = new Audio;
//    audio->startAudio("file_16k.pcm");

    //开始录音-2.使用科大讯飞麦克阵列录音
    hidmic->hidmic_start_record();
}

void SmartHome::on_speech_released()
{
    //停止录音-1.USB麦克或自带的麦克
    //audio->stopAudio();

    //停止录音-2.使用科大讯飞麦克阵列
    hidmic->hidmic_stop_record();
    //修改按钮文字
    ui->speech->setText("开始识别");

    //开始识别
    Speech m_speech;
    //QString text = m_speech.speechIdentify("file_16k.pcm");//自带的麦克
    QString text = m_speech.speechIdentify("./audio/mic_demo_vvui_deno.pcm");

    if(text == "开灯。"){
        on_LED_ON_clicked();
    }else if(text == "关灯。"){
        on_LED_OFF_clicked();
    }else if(text == "开启报警。"){
        on_BUZZER_ON_clicked();
    }else if(text == "关闭报警。"){
        on_BUZZER_OFF_clicked();
    }else if(text == "打开电视。"){
        on_OCR_ON_clicked();
    }else if(text == "关闭电视。"){
        on_OCR_OFF_clicked();
    }else if(text == "打开安防。"){
        ui->PIR_data->setChecked(true);
        on_PIR_data_clicked();
    }else if(text == "关闭安防。"){
        ui->PIR_data->setChecked(false);
        on_PIR_data_clicked();
    }
    ui->textEdit->setText(text);

    ui->speech->setText("按住说话");
}

void SmartHome::on_reTrainFaceBtn_clicked()
{
    process->start("python3 data/create_csv.py data/resources/att_faces/ data/resources/at.txt");
    process->waitForStarted();

    if (process->waitForFinished()){
        this->faceUtils->startTrainFace();
    }
}

void SmartHome::on_captureBtn_clicked()
{
    if(image_tmp.empty())
    {
        //QMessageBox::information(this, "警告", "请打开摄像头后重试");
        ui->prompt->setText("没有检测到人脸");
        ui->imgShowLabel->clear();
    }
    else
    {
        QString member = ui->comboBox->currentText();

        //正则只取数字
        QRegExp rx("\\d+");
        QString wndIndex;
        rx.indexIn(member,0);
        wndIndex = rx.cap(0);

        QString imgSavePath = QCoreApplication::applicationDirPath() + "/data/resources/att_faces" + "/s" +wndIndex;

        if(QFileInfo(imgSavePath).exists())
        {
            uint currentTime = QDateTime::currentDateTime().toTime_t();
            if(!imgSavePath.endsWith("/"))
                imgSavePath += "/";
            if(!imgSavePath.endsWith(".pgm"))
                imgSavePath += QString("%1.pgm").arg(currentTime);

            vector<Rect> faces = faceUtils->faceDetection(image_tmp);
            if(faces.empty()){
                ui->prompt->setText("没有检测到人脸");
                ui->imgShowLabel->clear();
            }
            foreach (Rect face, faces)
            {
                Mat imageRIO = image_tmp(face);
                cv::resize(imageRIO, imageRIO, Size(92, 112));

                Mat gray;
                cvtColor(imageRIO,gray, COLOR_BGR2GRAY);

                string str = imgSavePath.toLocal8Bit().toStdString();
                cv::imwrite(str,gray);

                QImage qImage = this->Mat2QImage(imageRIO);
                ui->imgShowLabel->setPixmap(QPixmap::fromImage(qImage.scaled(ui->imgShowLabel->size(),Qt::KeepAspectRatio)));// 保持原图片的长宽比，且不超过矩形框的大小
                ui->prompt->setText("录入成员" + wndIndex + "成功!");
            }
        }
        else
        {
            QMessageBox::information(this, "警告", "路径不存在，截图保存失败");
        }
    }
}

void SmartHome::on_openImgBtn_clicked()
{
    QString path;
    path = QCoreApplication::applicationDirPath() + "/data/resources/att_faces";
    if(QFileInfo(path).exists())
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    else
        QMessageBox::information(this, "警告", "当前路径不存在，请检查路径后重试");
}
void SmartHome::startTrainSlot()
{
    ui->reTrainFaceBtn->setEnabled(false);
}

void SmartHome::finishTrainSlot(bool isSuccess)
{
    ui->reTrainFaceBtn->setEnabled(true);
    if(!isSuccess)
        QMessageBox::information(this, "警告", "样本训练失败");
}

void SmartHome::on_captureDel_clicked()
{
    QString member = ui->comboBox->currentText();
    //正则只取数字
    QRegExp rx("\\d+");
    QString wndIndex;
    rx.indexIn(member,0);
    wndIndex = rx.cap(0);
    QString file_path=  QCoreApplication::applicationDirPath() + "/data/resources/att_faces" + "/s"+ wndIndex;
    if(removeFolderContent(file_path)){
        ui->prompt->setText("成员" + wndIndex + "删除成功!");
    }else{
        ui->prompt->setText("成员" + wndIndex + "不存在或已被删除");
    }
    ui->imgShowLabel->clear();
}
bool SmartHome::removeFolderContent(const QString &folderDir)
{
    QDir dir(folderDir);
    QFileInfoList fileList;
    QFileInfo curFile;
    if(!dir.exists())  {return false;}//文件不存，则返回false
    fileList=dir.entryInfoList(QDir::Dirs|QDir::Files
                               |QDir::Readable|QDir::Writable
                               |QDir::Hidden|QDir::NoDotAndDotDot
                               ,QDir::Name);

    if(fileList.size()==0)
        return false;
    while(fileList.size()>0)
    {
        int infoNum=fileList.size();
        for(int i=infoNum-1;i>=0;i--)
        {
            curFile=fileList[i];
            if(curFile.isFile())//如果是文件，删除文件
            {
                QFile fileTemp(curFile.filePath());
                fileTemp.remove();
                fileList.removeAt(i);
            }
            /*if(curFile.isDir())//如果是文件夹
            {
                QDir dirTemp(curFile.filePath());
                QFileInfoList fileList1=dirTemp.entryInfoList(QDir::Dirs|QDir::Files
                                                              |QDir::Readable|QDir::Writable
                                                              |QDir::Hidden|QDir::NoDotAndDotDot
                                                              ,QDir::Name);
                if(fileList1.size()==0)//下层没有文件或文件夹
                {
                    dirTemp.rmdir(".");
                    fileList.removeAt(i);
                }
                else//下层有文件夹或文件
                {
                    for(int j=0;j<fileList1.size();j++)
                    {
                        if(!(fileList.contains(fileList1[j])))
                            fileList.append(fileList1[j]);
                    }
                }
            }*/
        }
    }
//    dir.removeRecursively();
    return true;
}
void SmartHome::save_ip()
{
    QString host_data = ui->host_DHT11_data->text();
    QString port_data = ui->port_DHT11_data->text();
    QString user_data = ui->user_DHT11_data->text();
    QString password_data = ui->password_DHT11_data->text();
    QString subtopic_data = ui->SubTopic_DHT11_data->text();
    QString pubtopic_data = ui->PubTopic_DHT11_data->text();

    QFile *myFile;
    QTextStream *outFile;
    QString filename="./data/baidu_mqtt.sh";
    myFile=new QFile(filename);
    if(myFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        outFile=new QTextStream(myFile);
        *outFile<<host_data<<endl;
        *outFile<<port_data<<endl;
        *outFile<<user_data<<endl;
        *outFile<<password_data<<endl;
        *outFile<<subtopic_data<<endl;
        *outFile<<pubtopic_data<<endl;
        myFile->close();
    }
}
void SmartHome::get_ip()
{
    // QFile 构造函数中打开文件
    QFile *myFile;
    QString filename="./data/baidu_mqtt.sh";
    myFile=new QFile(filename);

    // 只读打开文件
    if(myFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString buffer;
        // 返回字节数，
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
            ui->host_DHT11_data->setText(buffer.trimmed());
        }
        buffer  = myFile->readLine();

        if (!buffer.isNull())
        {
            ui->port_DHT11_data->setValue(buffer.toInt());
        }
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
            ui->user_DHT11_data->setText(buffer.trimmed());
        }
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
            ui->password_DHT11_data->setText(buffer.trimmed());
        }
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
            ui->SubTopic_DHT11_data->setText(buffer.trimmed());
        }
        buffer  = myFile->readLine();
        if (!buffer.isNull())
        {
            ui->PubTopic_DHT11_data->setText(buffer.trimmed());
        }
        myFile->close();
    }
}

void SmartHome::on_face_button_clicked()
{
    if(ui->face_button->text() == "提示音关"){
        qsound_play_flag = 1;
        ui->face_button->setText(tr("提示音开"));
    }else{
        qsound_play_flag = 0;
        ui->face_button->setText(tr("提示音关"));
    }
}

void SmartHome::on_Publish_DHT11_clicked()
{
    if (m_client_dht11->publish(ui->PubTopic_DHT11_data->text(), ui->message_DHT11_data->text().toUtf8()) == -1)
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
}
