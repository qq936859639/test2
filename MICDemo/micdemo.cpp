#include "micdemo.h"
#include "ui_micdemo.h"

MICDemo::MICDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MICDemo)
{
    ui->setupUi(this);
}

MICDemo::~MICDemo()
{
    delete ui;
}
