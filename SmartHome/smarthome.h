#ifndef SMARTHOME_H
#define SMARTHOME_H

#include <QWidget>

namespace Ui {
class SmartHome;
}

class SmartHome : public QWidget
{
    Q_OBJECT

public:
    explicit SmartHome(QWidget *parent = nullptr);
    ~SmartHome();

private:
    Ui::SmartHome *ui;
};

#endif // SMARTHOME_H
