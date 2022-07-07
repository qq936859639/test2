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

void CoreModule::on_quit_clicked()
{
//    this->close();
    CoreModule::deleteLater();//关闭当前窗口
}
