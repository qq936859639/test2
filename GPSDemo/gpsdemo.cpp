#include "gpsdemo.h"
#include "ui_gpsdemo.h"

GPSDemo::GPSDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GPSDemo)
{
    ui->setupUi(this);
}

GPSDemo::~GPSDemo()
{
    delete ui;
}
