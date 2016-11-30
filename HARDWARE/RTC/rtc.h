#ifndef __RTC_H
#define __RTC_H	    

#include "stm32f10x.h"

//时间结构体
#pragma pack(push)
#pragma pack(1)
typedef struct 
{
	vu8 hour;
	vu8 min;
	vu8 sec;			
	//公历日月年周
	vu16 w_year;
	vu8 w_month;
	vu8 w_date;
	vu8 week;		 
}_calendar_obj;	
#pragma pack(pop)
extern _calendar_obj calendar;					//日历结构体
//extern _calendar_obj calendar_ghost;					//日历结构体
extern u8 calendar_ghost_enable;
extern u8 Rtc_Sec_It;

u8 RTC_Init(void);      						//平年,闰年判断
u8 Time_Adjust(void);	//设置时间	


#endif
