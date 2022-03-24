#ifndef MODBUSTHREAD_H
#define MODBUSTHREAD_H

#include <QThread>
#include <QObject>

class ModbusThread : public QThread
{
    Q_OBJECT
public:
    explicit ModbusThread(QObject *parent = nullptr);
    ~ModbusThread();
};

#endif // MODBUSTHREAD_H
