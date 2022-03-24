#ifndef MICDEMO_H
#define MICDEMO_H

#include <QWidget>

namespace Ui {
class MICDemo;
}

class MICDemo : public QWidget
{
    Q_OBJECT

public:
    explicit MICDemo(QWidget *parent = nullptr);
    ~MICDemo();

private:
    Ui::MICDemo *ui;
};

#endif // MICDEMO_H
