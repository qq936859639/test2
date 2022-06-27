#ifndef HIDMICDEMO_H
#define HIDMICDEMO_H

#include <QMessageBox>
#include <QDebug>

class HIDMICDEMO {
public:
    int hidmic_init();

    void hidmic_start_record();
    void hidmic_stop_record();
    void hidmic_close();
    void hidmic_config(int mic);
private:

};
#endif // HIDMICDEMO_H
