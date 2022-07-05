#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/signal.h>
#include "gps.h"

#define USE_BEIJING_TIMEZONE

#define DEBUG
#ifdef DEBUG
#define DPRINTF(x...) {printf(x);}
#else
#define DPRINTF(x...) (void)(0)
#endif

static int GetComma(int num,char* str);
static void UTC2BTC(date_time *GPS);
static double get_double_number(char *s);
static float Get_Float_Number(char *s);
static float Str_To_Float(char *buf);
static int Get_Int_Number(char *s);

void show_gps(GPS_INFO *GPS)
{ 	
//	printf("satellite      :	\033[0;31m %d \033[0m \n",GPS->satellite);
//	printf("latitude       :	\033[0;32m %d %d %d \033[0m \n",GPS->latitude_Degree,GPS->latitude_Cent,GPS->latitude_Second);
//	printf("longitude      :	\033[0;32m %d %d %d \033[0m \n",GPS->longitude_Degree,GPS->longitude_Cent,GPS->longitude_Second);
	printf("DATE           :	\033[0;32m %d%02d%02d \033[0m \n",GPS->D.year,GPS->D.month,GPS->D.day);
	printf("TIME           :	\033[0;32m %02d\033[5m:\033[0m\033[0;32m%02d\033[5m:\033[0m\033[0;32m%02d \033[0m  \n",GPS->D.hour,GPS->D.minute,GPS->D.second);	
	printf("HIGH           :	\033[0;35m %10.4f \033[0m  \n",GPS->high);	
	printf("SPEED          :	\033[0;35m %10.4f \033[0m  \n",GPS->speed );
	printf("DIRECTION      :	\033[0;35m %10.4f \033[0m  \n",GPS->direction );
	printf("LATITUDE-NS    :	\033[0;35m %10.4f %c \033[0m \n",GPS->latitude/100,GPS->NS);
        printf("LONGTITUDE-EW  :	\033[0;35m %10.4f %c \033[0m \n",GPS->longitude/100,GPS->EW);	
	printf("STATUS         :	\033[1;33m %c \033[0m \n\n",GPS->status );
}

unsigned char Data_CalcFCS(char  *msg_ptr, unsigned char  len)
{
      unsigned char  x;
      unsigned char  xorResult;

      xorResult = 0;

      for ( x = 0; x < len; x++, msg_ptr++ )
        xorResult = xorResult ^ *msg_ptr;

      return ( xorResult );
}

void gps_parse(char *line,GPS_INFO *GPS)
{
	int tmp;
	unsigned char status, fcs = 0;
	char *ptr = NULL;
	char *buf=line;
	float lati_cent_tmp, lati_second_tmp;
	float long_cent_tmp, long_second_tmp;
	float speed_tmp;
	

	if(strncmp(buf+3,"RMC",3)==0||strncmp(buf+3,"GGA",3)==0)
	{   	
		
		printf("GPRMC founded!\n");			
		if(strncmp(buf+3,"RMC",3)==0)
		{
			status   =buf[GetComma(2,buf)];
			if(status == 'A')
			{
				GPS->D.hour   =(buf[ 7]-'0')*10+(buf[ 8]-'0');
				GPS->D.minute =(buf[ 9]-'0')*10+(buf[10]-'0');
				GPS->D.second =(buf[11]-'0')*10+(buf[12]-'0');
				tmp = GetComma(9,buf);
				GPS->D.day    =(buf[tmp+0]-'0')*10+(buf[tmp+1]-'0');
				GPS->D.month  =(buf[tmp+2]-'0')*10+(buf[tmp+3]-'0');
				GPS->D.year   =(buf[tmp+4]-'0')*10+(buf[tmp+5]-'0')+2000;
	
				GPS->status   =buf[GetComma(2,buf)];
				GPS->latitude =get_double_number(&buf[GetComma(3,buf)]);
				GPS->NS       =buf[GetComma(4,buf)];
				GPS->longitude=get_double_number(&buf[GetComma(5,buf)]);
				GPS->EW       =buf[GetComma(6,buf)];
			GPS->latitude_Degree = (int)GPS->latitude / 100; //分离纬度
		            lati_cent_tmp = (GPS->latitude - GPS->latitude_Degree * 100);
		                GPS->latitude_Cent = (int)lati_cent_tmp;
		                lati_second_tmp = (lati_cent_tmp - GPS->latitude_Cent) * 60;
		            GPS->latitude_Second = (int)lati_second_tmp;

	            GPS->longitude_Degree = (int)GPS->longitude / 100;    //分离经度
            long_cent_tmp = (GPS->longitude - GPS->longitude_Degree * 100);
            GPS->longitude_Cent = (int)long_cent_tmp; 
            long_second_tmp = (long_cent_tmp - GPS->longitude_Cent) * 60;
            GPS->longitude_Second = (int)long_second_tmp;

            speed_tmp = Get_Float_Number(&buf[GetComma(7, buf)]); //速度(单位：海里/时)
            GPS->speed = speed_tmp * 1.85; //1海里=1.85公里
            GPS->direction = Get_Float_Number(&buf[GetComma(8, buf)]); //角度

			}
		}

		if(strncmp(buf+3,"GGA",3)==0)
		{ 
			ptr = strchr(buf,'*');
			if(ptr!=NULL){
				fcs = strtoul(ptr + 1,NULL,16);
				if(fcs == Data_CalcFCS(buf+1,(unsigned char)(ptr-buf-1)))
				{	
					GPS->D.hour   =(buf[ 7]-'0')*10+(buf[ 8]-'0');
                			GPS->D.minute =(buf[ 9]-'0')*10+(buf[10]-'0');
                			GPS->D.second =(buf[11]-'0')*10+(buf[12]-'0');
                
                			GPS->latitude =get_double_number(&buf[GetComma(2,buf)]);
                			GPS->NS       =buf[GetComma(3,buf)];
                			GPS->longitude=get_double_number(&buf[GetComma(4,buf)]);
                			GPS->EW       =buf[GetComma(5,buf)];
					GPS->high     = get_double_number(&buf[GetComma(9,buf)]);
					GPS->satellite = Get_Int_Number(&buf[GetComma(7, buf)]);
				}
			}
		}
	#ifdef USE_BEIJING_TIMEZONE
                UTC2BTC(&GPS->D);
	#endif
	}
}

static double get_double_number(char *s)
{
	char buf[128];
	int i;
	double rev;
	i=GetComma(1,s);
	strncpy(buf,s,i);
	buf[i]=0;
	rev=atof(buf);
	//printf("s=%s ,buf= %s,val= %f\n",s,buf,rev);
	return rev;
	
}
static int Get_Int_Number(char *s)
{
	char buf[128];
	int i;
	int rev;
	i=GetComma(1,s);
	strncpy(buf,s,i);
	buf[i]=0;
	rev=atoi(buf);
	//printf("s=%s ,buf= %s,val= %d\n",s,buf,rev);
	return rev;
	
}

static int GetComma(int num,char *str)
{
	int i,j=0;
	int len=strlen(str);
	for(i=0;i<len;i++)
	{
		if(str[i]==',')
		j++;
		if(j==num)return i+1;	
	}
	return 0;	
}

static float Get_Float_Number(char *s)
{

	char buf[10];
	unsigned char i;
	float rev;
	i=GetComma(1,s);
	i= i - 1;
	strncpy(buf,s, i);
	buf[i]= 0;
	rev=Str_To_Float(buf);
	return rev;     
}
static float Str_To_Float(char *buf)

{
	float rev = 0;
	float dat = 0;
	int integer = 1;
	char *str = buf;
	int i;
	while(*str!= '\0')
	{
		switch(*str)
		{
			case'0':
				dat= 0;
				break;
			case'1':
				dat= 1;
				break;
			case'2':
				dat= 2;
				break;           
			case'3':
				dat= 3;
				break;
			case'4':
				dat= 4;
				break;
			case'5':
				dat= 5;
				break;
			case'6':
				dat= 6;
				break;
			case'7':
				dat= 7;
				break;
			case'8':
				dat= 8;
				break;
			case'9':
				dat= 9;
				break;
			case'.':
				dat= '.';
				break;
		}
		if(dat== '.')
		{
			integer= 0;
			i= 1;
			str++;
			continue;
		}
		if(integer == 1 )
		{
			rev= rev * 10 + dat;
		}
		else
		{
			rev= rev + dat / (10 * i);
			i= i * 10 ;
		}
		str++;
	}
	return rev;
}
static void UTC2BTC(date_time *GPS)
{

	GPS->second++;
      if(GPS->second>59){
			GPS->second=0;
			GPS->minute++;
			if(GPS->minute>59){
				GPS->minute=0;
				GPS->hour++;
			}
		}	

     GPS->hour+=8;
     if(GPS->hour>23)
		{
			GPS->hour-=24;
			GPS->day+=1;
			if(GPS->month==2 ||
			   		GPS->month==4 ||
			   		GPS->month==6 ||
			   		GPS->month==9 ||
			   		GPS->month==11 ){
				if(GPS->day>30){
			   		GPS->day=1;
					GPS->month++;
				}
			}
			else{
				if(GPS->day>31){
			   		GPS->day=1;
					GPS->month++;
				}
			}
			if(GPS->year % 4 == 0 ){
		   		if(GPS->day > 29 && GPS->month ==2){
		   			GPS->day=1;
					GPS->month++;
				}
			}
			else{
		   		if(GPS->day>28 &&GPS->month ==2){
		   			GPS->day=1;
					GPS->month++;
				}
			}
			if(GPS->month>12){
				GPS->month-=12;
				GPS->year++;
			}		
		}
}
