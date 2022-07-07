#include "speakerdemo.h"
#include "ui_speakerdemo.h"
#include <QDebug>
SpeakerDemo::SpeakerDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpeakerDemo)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    qsound = new QSound("./mp3/speaker_play.wav", this);
}

SpeakerDemo::~SpeakerDemo()
{
    delete ui;
}

void SpeakerDemo::on_speaker_play_clicked()
{
    qsound->play();
}

void SpeakerDemo::on_speaker_stop_clicked()
{
    qsound->stop();
}

void SpeakerDemo::on_quit_clicked()
{
    SpeakerDemo::deleteLater();//关闭当前窗口
}
