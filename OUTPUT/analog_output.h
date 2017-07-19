#ifndef _ANALOG_OUTPUT_H_
#define _ANALOG_OUTPUT_H_

#include "types.h"
#include "define.h"
 
 
#define PIC_I2C_ADDR	0x30

#define	PWM_CHN_HUM		PCA_MODULE2
#define	PWM_CHN_TEMP	PCA_MODULE3
#define	PWM_CHN_CO2		PCA_MODULE4

#define OUT_FAULT	0
//#ifdef CO2_SENSOR
//	#define _4_20MA		1
//	#define	_0_5V		2
//	#define	_0_10V		3
//#else
	#define _4_20MA		3
	#define	_0_5V		2
	#define	_0_10V		1

//#endif

#define	OUT_VOLTMODE	P3_0
#define OUT_AMPSMODE	P3_1

#define CHANNEL_HUM		0
#define CHANNEL_TEMP	1
#define CHANNEL_CO2		2
#define CHANNEL_PRE		3

typedef enum 
{
	REG_PIC_REV = 0,
	REG_FBV_HUM_H,
	REG_FBV_HUM_L,      
	REG_FBV_TEMP_H, 
	REG_FBV_TEMP_L, 
	REG_FBV_CO2_H,
	REG_FBV_CO2_L,
	REG_FBI_HUM_H,
	REG_FBI_HUM_L,
	REG_FBI_TEMP_H, 
	REG_FBI_TEMP_L, 
	REG_FBI_CO2_H,
	REG_FBI_CO2_L,
	REG_THERMOSTAT_H,
	REG_THERMOSTAT_L,
	REG_END,
} PIC_I2C_REGISTER_LIST;	

typedef struct
{
	S16_T min;
	S16_T max;	
} _RANGE_;

extern _RANGE_ output_range_table[3];

extern U16_T feedback[6];
extern U8_T output_mode;
extern U16_T pwm_th_settings[3];
extern U16_T co2_output_value;
extern U16_T theory_ad;
extern U8_T th_offset;
extern U16_T internal_temperature_ad_value;
extern U8_T output_auto_manual;
extern U16_T output_manual_value_co2;
extern S16_T output_manual_value_temp;
extern U16_T output_manual_value_humidity;

//extern S8_T hum_amp_output_offset;
//extern S8_T temp_amp_output_offset;
//extern S8_T co2_amp_output_offset;
//extern S8_T hum_vol_output_offset;
//extern S8_T temp_vol_output_offset;
//extern S8_T co2_vol_output_offset; 


extern uint16 analog_output[3];
extern uint16 analog_input[3];
extern int16 output_offset[2][3];


void vOutPutTask(void *pvParameters);









#endif


