#-------------------------------------------------
#
# Project created by QtCreator 2022-03-08T14:39:36
#
#-------------------------------------------------

QT       += core gui serialbus multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

target.path = /home/uptech/cjf
INSTALLS += target

INCLUDEPATH += . /usr/include/opencv4/opencv /usr/include/opencv4

TARGET = UP-AICar-Platform
TEMPLATE = app



SOURCES += main.cpp\
        AICarDemo/aicardemo.cpp \
        AICarDemo/car/car.cpp \
        CameraDemo/camerademo.cpp \
        plr/cvUniText.cpp \
        plr/plr.cpp \
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
    plr/cvUniText.hpp \
    plr/include/detector_creator.h \
    plr/include/lpc_recognizer.h \
    plr/include/lpr_recognizer.h \
    plr/include/plate_detectors.h \
    plr/include/plate_info.h \
    plr/include/plate_petector.h \
    plr/include/plate_recognizers.h \
    plr/plr.h \
    plr/timing.h \
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

DISTFILES += \
    data/haarcascade_frontalface_default.xml \

CONFIG += c++11

INCLUDEPATH += \
    /usr/include/opencv4 \
    /usr/include/ncnn \
    /usr/include/freetype2

LIBS += $(shell pkg-config --libs opencv4) \
    -fopenmp \
    -lfreetype \
    -lgomp  \
    -lstdc++ \
    -lm \
    -lmlpr \
    -lncnn \
    -lpthread
#------A311D
#-lvulkan -lglslang -lSPIRV -lMachineIndependent -lOGLCompiler -lOSDependent -lGenericCodeGen
# or LIBS += ./plr/deps/ncnn/lib/libncnn_imx8.a -lncnn
#------end
