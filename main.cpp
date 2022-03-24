#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("UP-AICar-Platform Demo");
    w.show();
    return a.exec();
}
