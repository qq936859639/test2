#include "smarthome.h"
#include "ui_smarthome.h"

SmartHome::SmartHome(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SmartHome)
{
    ui->setupUi(this);
}

SmartHome::~SmartHome()
{
    delete ui;
}
