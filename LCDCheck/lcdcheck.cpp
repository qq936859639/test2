#include "lcdcheck.h"
#include "ui_lcdcheck.h"
#include <QGraphicsScene>
#include <QDebug>

LCDCheck::LCDCheck(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LCDCheck)
{
    ui->setupUi(this);
    status = 0;
    connect(this,SIGNAL(clicked()),this,SLOT(mouseClicked()));
}
LCDCheck::~LCDCheck()
{
    delete ui;
}

void LCDCheck::mouseClicked(){
    switch (this->status % 7) {
    case 0:
        this->setStyleSheet("background-color: rgb(255, 0, 0);");
        break;
    case 1:
        this->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    case 2:
        this->setStyleSheet("background-color: rgb(0, 0, 255);");
        break;
    case 3:
        this->setStyleSheet("background-color: rgb(255, 255, 0);");
        break;
    case 4:
        this->setStyleSheet("background-color: rgb(0, 255, 255);");
        break;
    case 5:
        this->setStyleSheet("background-color: rgb(255, 0, 255);");
        break;
    case 6:
        this->setStyleSheet("background-color: rgb(255, 255, 255);");
        this->status = 0;
        break;
    }
    this->status ++;
}

void LCDCheck::mouseReleaseEvent(QMouseEvent *ev){
    Q_UNUSED(ev);
    emit clicked();
}



void LCDCheck::on_quit_clicked()
{
    LCDCheck::deleteLater();//关闭当前窗口
}
