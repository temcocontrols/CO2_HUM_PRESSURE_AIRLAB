
#ifndef __HUMIDITY_H__

#define	__HUMIDITY_H__

#include "types.h"
#include "define.h" 
#include "bitmap.h"   



#define HEAT_START     			1
#define HEAT_STOP      			0


#define	CMD_READ_PARAMS				0xc1 //
#define	CMD_READ_VERSION			0xc2 //
#define CMD_LIGHT_READ              0xc4 
#define	CMD_OVERRIDE_CAL			0xd3 //
#define	CMD_READ_CAL_PT				0xd4
#define	CMD_HEATING_CONTROL			0xd5 //
#define	CMD_CLEAR_TABLE				0xe4 //
#define	CMD_TEMP_CAL				0xe5 //
#define	CMD_HUM_CAL					0xe6 //

#define LIGHT_SENSOR_ADDR_W		0x52
#define LIGHT_SENSOR_ADDR_R		0x53

#define TEMP_CALIBRATION	0x01
#define HUM_CALIBRATION		0x02
#define HUM_HEATER			0x04

typedef struct
{
	int16 ad[2];
	uint16 humidity;
	float pre_humidity;
	int16 temperature_c;
	float pre_temperature_c;
	int16 temperature_f;
	uint16 frequency;
	uint8 counter;
	uint8 T_Filter;
	uint8 H_Filter;
 
	uint16 sn;
	int16 offset_t;
	int16 offset_h;
	int16 offset_h_default;
	int16 dew_pt;
	int16 dew_pt_F;
	uint16 Pws;
	uint16 Mix_Ratio;
	uint16 Enthalpy;  
	int16 compensation_val;
	int16 org_hum;
 
}STR_HUMIDITY;

extern STR_HUMIDITY HumSensor;
extern uint8 humidity_version; 
extern uint8 hum_heat_status;

extern int16 external_operation_value;
extern uint8 external_operation_flag;



#define   MAX_CAL_POINT                10
#define   FAC_TABLE                    0
#define   USER_TABLE                   1

#define   IIC_ADDR                     0x5a

#define   CMD_READ_PARAMS              0xc1
#define   CMD_READ_VERSION             0xc2
#define   CMD_CONFIG_READ              0xc3

#define   CMD_OVERRIDE_CAL             0xd3
#define   CMD_READ_CAL_PT              0xd4
#define   CMD_HEATING_CONTROL          0xd5

#define   CMD_CLEAR_TABLE              0xe4
#define   CMD_TEMP_CAL                 0xe5
#define   CMD_HUM_CAL                  0xe6
#define   CMD_SN_WRITE                 0xee
#define   CMD_SN_READ                  0xef

#define   CMD_TAB_SEL                  0xa0
#define   CMD_USER_TAB_PT_SET          0xa1 

#define   LIGHT_SENSOR 				   28
  
	bit read_sensor(void); 
	bit write_sensor_temp(int16 temp); 
	bit write_sensor_humtable(uint16 hum, uint16 freq);
	bit read_sensor_humtab(uint8 pt, uint16 *hum, uint16 *freq);
	bit clear_sensor_humtab(void); 
	bit write_sensor_humidity_humidity(uint16 hum); 

	bit pic_read_sn(uint16 *sn);
	// bit pic_write_sn(uint16 sn);
	void update_humidity_display(uint8 ForceUpdate);
	void external_operation(void); 
	void humidity_check_online(void);

	bit read_humidity_sensor_version(void);
	bit read_light_sensor_version(void);
	void start_light_sensor_mearsure(void);
	uint8 read_light_sensors_time(void);
	uint8 read_light_sensors_gain(void);
	uint16 read_light_sensors_data0(void);
	uint16 read_light_sensors_data1(void);
	
	
	void auto_heating(void);
	bit pic_read_light_val(uint16 *val);
	extern uint8 point_num; 
#endif

