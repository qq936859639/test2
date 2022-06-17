#ifndef SPEECH_H
#define SPEECH_H

#include <QObject>
#include "http.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostInfo>
#include <QFile>
#include <QMessageBox>

//自己的API Key和Secret Key
const QString client_id = "ulK1LrgAnIeuUKoqdFBzaTbB";
const QString client_secret = "F9WSDgE8jTBPGQFihdkFVX1YKdfuMEeE";

//获取access_token相关
const QString baiduTokenUrl ="https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=%1&client_secret=%2";

//短语音识别标准版
const QString baiduSpeechUrl = "http://vop.baidu.com/server_api?dev_pid=1537&cuid=%1&token=%2";

//短语音识别极速版
//const  QString baiduSpeechUrl = "https://vop.baidu.com/pro_api?dev_pid=80001&cuid=%1&token=%2";/**< 识别方言 本机唯一标识 获取的token*/

class Speech : public QObject
{
    Q_OBJECT
public:
    explicit Speech(QObject *parent = nullptr);

    QString speechIdentify(QString fileName);

    QString getJsonValue(QByteArray ba, QString key);

signals:

};

#endif // SPEECH_H
