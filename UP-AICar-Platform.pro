#-------------------------------------------------
#
# Project created by QtCreator 2022-03-08T14:39:36
#
#-------------------------------------------------

QT       += core gui serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

target.path = /home/uptech/cjf
INSTALLS += target

TARGET = UP-AICar-Platform
TEMPLATE = app


SOURCES += main.cpp\
        AICarDemo/aicardemo.cpp \
        AICarDemo/car/car.cpp \
        CameraDemo/camerademo.cpp \
        services/camerathread.cpp \
        CoreModule/coremodule.cpp \
        GPSDemo/gpsdemo.cpp \
        LCDCheck/lcdcheck.cpp \
        MICDemo/micdemo.cpp \
        SmartHome/smarthome.cpp \
        mainwindow.cpp \
        services/modbusthread.cpp \
        services/v4l.cpp

HEADERS  += mainwindow.h \
    AICarDemo/aicardemo.h \
    AICarDemo/car/car.h \
    CameraDemo/camerademo.h \
    services/camerathread.h \
    CoreModule/coremodule.h \
    GPSDemo/gpsdemo.h \
    LCDCheck/lcdcheck.h \
    MICDemo/micdemo.h \
    SmartHome/smarthome.h \
    services/modbusthread.h \
    services/v4l.h

FORMS    += mainwindow.ui \
    AICarDemo/aicardemo.ui \
    CameraDemo/camerademo.ui \
    CoreModule/coremodule.ui \
    GPSDemo/gpsdemo.ui \
    LCDCheck/lcdcheck.ui \
    MICDemo/micdemo.ui \
    SmartHome/smarthome.ui

RESOURCES += \
    resource.qrc
