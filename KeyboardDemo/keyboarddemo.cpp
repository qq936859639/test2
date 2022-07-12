#include "keyboarddemo.h"
#include "ui_keyboarddemo.h"

KeyboardDemo::KeyboardDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KeyboardDemo)
{
    ui->setupUi(this);
}

KeyboardDemo::~KeyboardDemo()
{
    delete ui;
}

void KeyboardDemo::on_quit_clicked()
{
    KeyboardDemo::deleteLater();//关闭当前窗口
}
