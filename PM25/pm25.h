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

typedef enum{
	PM25_0_100,
	PM25_0_1000
}PM25_RANGE_T;

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
	u8 pm25_range;
	u8 AQI_area;
}STR_PM25;

extern STR_PM25 pm25_sensor;


void PM25_task(void *pvParameters);
void pm25_reset_factory(void);


#define NOVA  		0
#define SENSIRION  1
#define PACKAGE_END  70
#define DATA_OFFSET  5

#define UG_M3   0
#define NUM_CM3   1

extern u8 pm25_sensor_type;
extern u8 pm25_current_cmd;
extern u8 sensirion_rev_cnt;
extern u8 sensirion_rev_end;
extern u8 shift_flag;
extern u8 pm25_unit;

extern uint16 pm25_number_05;
extern uint16 pm25_number_10;
extern uint16 pm25_number_25;
extern uint16 pm25_number_40;
extern uint16 pm25_number_100;
extern uint16 pm25_weight_10;
extern uint16 pm25_weight_25;
extern uint16 pm25_weight_40;
extern uint16 pm25_weight_100;
//extern u8 sensirion_rcv_buf[80];

enum{
SENSIRION_NULL,
SENSIRION_START_MEASUREMENT,
SENSIRION_STOP_MEASUREMENT,	
SENSIRION_READ_MEASUREMENT,
SENSIRION_READ_AUTO_CLEAN,
SENSIRION_DISABLE_AUTO_CLEAN,
SENSIRION_START_FAN_CLEAN,
SENSIRION_RESET	
};

#endif

