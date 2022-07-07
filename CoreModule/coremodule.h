#ifndef COREMODULE_H
#define COREMODULE_H

#include <QWidget>

namespace Ui {
class CoreModule;
}

class CoreModule : public QWidget
{
    Q_OBJECT

public:
    explicit CoreModule(QWidget *parent = nullptr);
    ~CoreModule();

private slots:
    void on_quit_clicked();

private:
    Ui::CoreModule *ui;
};

#endif // COREMODULE_H
