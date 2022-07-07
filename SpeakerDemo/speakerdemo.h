#ifndef SPEAKERDEMO_H
#define SPEAKERDEMO_H

#include <QWidget>
#include <QSound>//声音
#include <QMediaPlayer>
namespace Ui {
class SpeakerDemo;
}

class SpeakerDemo : public QWidget
{
    Q_OBJECT

public:
    explicit SpeakerDemo(QWidget *parent = nullptr);
    ~SpeakerDemo();
QSound *qsound;
private slots:
    void on_speaker_play_clicked();
    void on_speaker_stop_clicked();
    void on_quit_clicked();

private:
    Ui::SpeakerDemo *ui;

};

#endif // SPEAKERDEMO_H
