#include "aicardemo.h"
#include "ui_aicardemo.h"

AICarDemo::AICarDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AICarDemo)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-500, -500, 1000, 1000);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    car = new Car();
    scene->addItem(car);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
}

AICarDemo::~AICarDemo()
{
    delete ui;
}
