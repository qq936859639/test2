#ifndef LCDCHECK_H
#define LCDCHECK_H

#include <QWidget>

namespace Ui {
class LCDCheck;
}

class LCDCheck : public QWidget
{
    Q_OBJECT

public:
    explicit LCDCheck(QWidget *parent = nullptr);
    ~LCDCheck();
    int status;

signals:
    void clicked();

public slots:
    void mouseClicked();

protected:
    void mouseReleaseEvent(QMouseEvent *ev);

private:
    Ui::LCDCheck *ui;
};

#endif // LCDCHECK_H
