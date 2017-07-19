#ifndef _PM25_H_
#define _PM25_H_
#include "types.h"
#include "define.h" 
#include "bitmap.h"  

#define MAX_WORK_PERIOD	30

#define CMD_LENTH		19
#define PM25_PEL_LENTH	9
typedef struct
{
	u8 menu_set;
	u8 scroll_set;
	u8 seconds;
	u8 display;
}str_menu;
typedef struct
{
	u16 id;
	int16 pm25;
	int16 pm10;
	int16 pre_pm25;
	int16 pre_pm10;
	u16 AQI;
	u8 level;
	u8 mode;
	u8 period;
	u8 status;
	u8 cmd_status; 
	u8 period_spare;
	u8 test;
	str_menu menu;
	int16 pm25_offset;
	int16 pm10_offset;
	u8 PM25_filter;
	u8 PM10_filter;
	u8 auto_manual;
}STR_PM25;

extern STR_PM25 pm25_sensor;


void PM25_task(void *pvParameters);
void pm25_reset_factory(void);
#endif

