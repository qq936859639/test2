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
