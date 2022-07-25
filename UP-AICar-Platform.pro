#-------------------------------------------------
#
# Project created by QtCreator 2022-03-08T14:39:36
#
#-------------------------------------------------

QT       += core gui serialbus multimedia serialport mqtt webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

target.path = /home/uptech/cjf
INSTALLS += target

INCLUDEPATH += . /usr/include/opencv4/opencv /usr/include/opencv4

TARGET = UP-AICar-Platform
TEMPLATE = app

SOURCES += main.cpp\
        AICarDemo/aicardemo.cpp \
        AICarDemo/car/car.cpp \
        AICarDemo/rplidar/rplidar.cpp \
        CameraDemo/camerademo.cpp \
        GPSDemo/gps_bd/gps.c \
        KeyboardDemo/keyboarddemo.cpp \
        MICDemo/hidmicdemo/hidmicdemo.cpp \
        SensorDemo/sensordemo.cpp \
        SmartHome/BaiduSpeech/audio.cpp \
        SmartHome/BaiduSpeech/http.cpp \
        SmartHome/BaiduSpeech/speech.cpp \
        SmartHome/cjson/cJSON.c \
        SpeakerDemo/speakerdemo.cpp \
        lpr/cvUniText.cpp \
        lpr/lpr.cpp \
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
    AICarDemo/rplidar/hal/types.h \
    AICarDemo/rplidar/rplidar.h \
    AICarDemo/rplidar/rplidar_cmd.h \
    AICarDemo/rplidar/rplidar_driver.h \
    AICarDemo/rplidar/rplidar_protocol.h \
    AICarDemo/rplidar/rptypes.h \
    CameraDemo/camerademo.h \
    GPSDemo/gps_bd/gps.h \
    KeyboardDemo/keyboarddemo.h \
    MICDemo/hidmicdemo/hidapi.h \
    MICDemo/hidmicdemo/hidmicdemo.h \
    MICDemo/hidmicdemo/protocol_proc_unit.h \
    MICDemo/hidmicdemo/queue_simple.h \
    SensorDemo/sensordemo.h \
    SmartHome/BaiduSpeech/audio.h \
    SmartHome/BaiduSpeech/http.h \
    SmartHome/BaiduSpeech/speech.h \
    SmartHome/cjson/cJSON.h \
    SpeakerDemo/speakerdemo.h \
    lpr/cvUniText.hpp \
    lpr/include/detector_creator.h \
    lpr/include/lpc_recognizer.h \
    lpr/include/lpr_recognizer.h \
    lpr/include/plate_detectors.h \
    lpr/include/plate_info.h \
    lpr/include/plate_petector.h \
    lpr/include/plate_recognizers.h \
    lpr/lpr.h \
    lpr/timing.h \
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
    KeyboardDemo/keyboarddemo.ui \
    LCDCheck/lcdcheck.ui \
    MICDemo/micdemo.ui \
    SensorDemo/sensordemo.ui \
    SmartHome/smarthome.ui \
    SpeakerDemo/speakerdemo.ui

RESOURCES += \
    resource.qrc

DISTFILES += \
    data/haarcascade_frontalface_default.xml \

CONFIG += c++11

INCLUDEPATH += \
    /usr/include/opencv4 \
#    /usr/include/ncnn \
    /usr/include/freetype2

LIBS += $(shell pkg-config --libs opencv4) \
    -fopenmp \
    -lfreetype \
    -lgomp  \
    -lstdc++ \
    -lm \
    -lpthread
#------A311D
#-lvulkan -lglslang -lSPIRV -lMachineIndependent -lOGLCompiler -lOSDependent -lGenericCodeGen
# or LIBS += ./lpr/deps/ncnn/lib/libncnn_imx8.a -lncnn
#------end


#A311D
contains(QT_ARCH, arm64){
#车牌识别库
unix:!macx: LIBS += -L$$PWD/lib/lpr/arm64/ -lmlpr -lncnn 
INCLUDEPATH += $$PWD/lpr/include/ncnn
DEPENDPATH += $$PWD/lib/lpr/arm64
unix:!macx: PRE_TARGETDEPS += $$PWD/lib/lpr/arm64/
#--科大讯飞
INCLUDEPATH += \
    /usr/include/libusb-1.0
unix:!macx: LIBS += -L$$PWD/lib/iflytek/arm64/ -lhid_lib
INCLUDEPATH += $$PWD/lib/iflytek/arm64
DEPENDPATH += $$PWD/lib/iflytek/arm64
#--end
LIBS += -lvulkan -lglslang -lSPIRV -lMachineIndependent -lOGLCompiler -lOSDependent -lGenericCodeGen

}else{

#--科大讯飞
INCLUDEPATH += \
    /usr/include/libusb-1.0
unix:!macx: LIBS += -L$$PWD/lib/iflytek/x64/ -lhid_lib
INCLUDEPATH += $$PWD/lib/iflytek/x64
DEPENDPATH += $$PWD/lib/iflytek/x64
#--end

#车牌识别库
unix:!macx: LIBS += -L$$PWD/lib/lpr/x64/ -lmlpr -lncnn
INCLUDEPATH += $$PWD/lpr/include/ncnn
DEPENDPATH += $$PWD/lib/lpr/x64
unix:!macx: PRE_TARGETDEPS += $$PWD/lib/lpr/x64/
#激光雷达库
unix:!macx: LIBS += -L$$PWD/lib/rplidar/x64/ -lrplidar_sdk
INCLUDEPATH += $$PWD/lib/rplidar/x64
DEPENDPATH += $$PWD/lib/rplidar/x64
unix:!macx: PRE_TARGETDEPS += $$PWD/lib/rplidar/x64/
}

