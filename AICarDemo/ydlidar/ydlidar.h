#ifndef YDLIDAR_H
#define YDLIDAR_H

#include "CYdLidar.h"

class YDLIDAR {
public:
    int rplidar_open();
    int rplidar_read();
    int rplidar_close();
    void rplidar_startMotor();
    void rplidar_stopMotor();
    uint8_t rplidar_ranges_flag;
private:
    CYdLidar laser;
    float rplidar_theta;
    float rplidar_dist;
};
#endif // YDLIDAR_H
