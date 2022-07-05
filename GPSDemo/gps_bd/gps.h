#ifndef _GPS_H
	#define _GPS_H
//#include "types.h"

typedef struct{
	int year;  
	int month; 
	int day;
	int hour;
	int minute;
	int second;
}date_time;

typedef struct{
	 date_time D;//时间
	 char status;  		//接收状态
	 int latitude_Degree;    //度
	 int latitude_Cent;   //分
	 int latitude_Second; //秒
	 int longitude_Degree;    //度
	 int longitude_Cent;  //分
	 int longitude_Second; //秒
	 float direction; //航向
	 double	latitude;   //纬度
	 double longitude;  //经度
	 int satellite;
	 char NS;           //南北极
	 char EW;           //东西
	 double speed;      //速度
	 double high;       //高度
	 int DB;
	 int GNSS;
}GPS_INFO;

void gps_parse(char *line,GPS_INFO *GPS);
void show_gps(GPS_INFO *GPS);
#endif
