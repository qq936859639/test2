#ifndef GPSDEMO_H
#define GPSDEMO_H

#include <QWidget>

namespace Ui {
class GPSDemo;
}

class GPSDemo : public QWidget
{
    Q_OBJECT

public:
    explicit GPSDemo(QWidget *parent = nullptr);
    ~GPSDemo();

private:
    Ui::GPSDemo *ui;
};

#endif // GPSDEMO_H
