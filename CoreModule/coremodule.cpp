#include "coremodule.h"
#include "ui_coremodule.h"

CoreModule::CoreModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CoreModule)
{
    ui->setupUi(this);
}

CoreModule::~CoreModule()
{
    delete ui;
}
