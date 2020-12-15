
#ifndef __TEMPERATURE_H__

#define	__TEMPERATURE_H__

#include "types.h"
#include "define.h" 
#include "bitmap.h"   

//#define	MIN_TEMP	-500
//#define	MAX_TEMP	3000

#define DEGREE_C	0
#define	DEGREE_F	1

#define	INTERNAL_TEMPERATURE_SENSOR		0
#define	EXTERNAL_TEMPERATURE_SENSOR		1

//extern int16 temperature_f, temperature_c;
extern uint16 hum_read_delay;
extern float tem_org;
extern float hum_org;
extern uint8 deg_c_or_f;
extern uint8 pre_deg_c_or_f;
extern int16 previous_temperature;
extern uint8 temperature_sensor_select;
extern uint8 Temperature_Filter;
extern int16 internal_temperature_c;
extern int16 internal_temperature_f;
extern int16 internal_temperature_offset;
extern uint8 analog_output_sel;
extern uint8 auto_heat_enable;

void update_temperature(void);
void update_temperature_display(uint8 ForceUpdate);
 
		
#define PIC_NORMAL     0 // sensor work well
#define PIC_ON_TO_OFF  1 // sensor had been taken off
#define PIC_OFF_TO_ON  2 // new sensor is connected
#define PIC_NOT_EXIST  3 // never detect sensor
#define PIC_WAIT_OFF_TO_ON       4 // wait for PIC state change
#define PIC_CONNECTING           5 //new sensor is connecting
#define PIC_INITIAL              6
#define PIC_WAITING1             7 //wait for 1st display duty
#define PIC_WAITING2             8 //wait for 1nd display duty
#define PIC_WAITING_END          11 //end of display waiting

#define FACTORY  0
#define USER     1

#define HUM_PERCENT 1
#define HUM_FREQ 0

#define FIRST_TIME   30
void initial_hum_eep(void);
bit refresh_sensor(void);

extern uint8   Run_Timer; 
extern uint16 internal_temp_ad;
extern bit hum_exists ;
extern bit table_sel_enable  ;

extern uint8 xdata display_state ;
extern uint8 xdata table_sel;
extern uint8 xdata hum_size_copy;
extern uint16 xdata hum_table[10][2];
extern int16 xdata humidity_back;

extern float  xdata k_line;
extern float  xdata b_line;
void vUpdate_Temperature_Task( void *pvParameters );

typedef struct
{
	uint16 val;
	uint16 pre_val;
	uint8 filter;
	uint16 ad;
	uint16 k;
	uint16 org;
}STR_LIGHT_SENSOR;

extern STR_LIGHT_SENSOR light;
	 
#endif

