#ifndef KEYBOARDDEMO_H
#define KEYBOARDDEMO_H

#include <QWidget>

namespace Ui {
class KeyboardDemo;
}

class KeyboardDemo : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardDemo(QWidget *parent = nullptr);
    ~KeyboardDemo();

private slots:
    void on_quit_clicked();

private:
    Ui::KeyboardDemo *ui;
};

#endif // KEYBOARDDEMO_H
