#ifndef AICARDEMO_H
#define AICARDEMO_H

#include <QWidget>
#include <QGraphicsScene>
#include <QPainter>
#include "AICarDemo/car/car.h"

namespace Ui {
class AICarDemo;
}

class AICarDemo : public QWidget
{
    Q_OBJECT

public:
    explicit AICarDemo(QWidget *parent = nullptr);
    ~AICarDemo();
    QGraphicsScene *scene;
    QGraphicsView *view;
    Car *car ;

private:
    Ui::AICarDemo *ui;
};

#endif // AICARDEMO_H
