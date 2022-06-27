#ifndef MICDEMO_H
#define MICDEMO_H

#include <QWidget>
#include "hidmicdemo/hidmicdemo.h"
//#include <QProcess>
namespace Ui {
class MICDemo;
}

class MICDemo : public QWidget
{
    Q_OBJECT

public:
    explicit MICDemo(QWidget *parent = nullptr);
    ~MICDemo();
//    QProcess process;
private slots:
    void on_speech_config_clicked();
    void on_speech_pressed();
    void on_speech_released();
    void on_speech_play_clicked();

    void on_quit_clicked();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MICDemo *ui;
    HIDMICDEMO *hidmic;
};

#endif // MICDEMO_H
