#include "micdemo.h"
#include "ui_micdemo.h"

#include <QFile>
#include <QAudioFormat>
#include <QAudioOutput>

#include <QThread>
MICDemo::MICDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MICDemo)
{
    ui->setupUi(this);

    hidmic = new HIDMICDEMO;
    hidmic->hidmic_init();
}

MICDemo::~MICDemo()
{
    delete ui;
}

void MICDemo::closeEvent(QCloseEvent *event)
{
    hidmic->hidmic_close();//关闭麦克风
}

void MICDemo::on_speech_config_clicked()
{
    if(0 <= ui->spinBox->value() &&ui->spinBox->value()<6){
        hidmic->hidmic_config(ui->spinBox->value());
        ui->textBrowser->clear();
        ui->textBrowser->setText("设置成功");
    }else{
        ui->textBrowser->clear();
        ui->textBrowser->setText("请设置数值0-5");
    }
}

void MICDemo::on_speech_pressed()
{
    ui->speech->setText("开始录音");
    //开始录音
    hidmic->hidmic_start_record();
    ui->textBrowser->clear();
    ui->textBrowser->setText("开始录音");
}

void MICDemo::on_speech_released()
{
    //结束录音
    hidmic->hidmic_stop_record();
    ui->speech->setText("按住说话");
    ui->textBrowser->clear();
    ui->textBrowser->setText("结束录音");
}

void MICDemo::on_speech_play_clicked()
{
    QAudioFormat fmt;           //音频格式
    fmt.setSampleRate(16000);   //采样率,根据音频实际采样率设置，也可利用该参数实现音频倍速播放
    fmt.setSampleSize(16);      //样本表示大小，使用16Bit表示
    fmt.setChannelCount(1);     //通道数量
    fmt.setCodec("audio/pcm");  //解码器
    fmt.setByteOrder(QAudioFormat::LittleEndian); //字节序，小端模式
    fmt.setSampleType(QAudioFormat::SignedInt);   //样本类型(在linux下显示支持UnSignedInt，但是实际打开会失败，改用SignedInt)
    QAudioOutput *out = new QAudioOutput(fmt);
    QIODevice *io = out->start();   // 开始播放
    ui->textBrowser->clear();
    ui->textBrowser->setText("开始播放");

    int size = out->periodSize();
    char *buf = new char[size]; //
    FILE *fp = fopen("./audio/mic_demo_vvui_deno.pcm", "rb");
    while (!feof(fp))
    {
        if (out->bytesFree() < size)
        {
            QThread::msleep(1); //
            continue;   //读的速度很快，等待播放完成
        }
        int len = fread(buf, 1, size, fp);
        if (len <= 0) break;
        io->write(buf, len);  // 播放
    }
    fclose(fp);
    delete[] buf;

/*
    qDebug()<<"cjf ok";
    process.start("aplay  -r 16000 ./audio/mic_demo_vvui_deno.pcm");
    if (process.waitForStarted()){
        //qDebug() << "PCM player is ready.";
    }
*/
}


void MICDemo::on_quit_clicked()
{
     hidmic->hidmic_close();//关闭麦克风
     MICDemo::deleteLater();//关闭当前窗口
}
