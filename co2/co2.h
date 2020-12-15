
#ifndef __CO2_H__

#define	__CO2_H__

#include "types.h"
#include "define.h"

#include "bitmap.h"    

#define	EXCEPTION_PPM			99//65535
#define	ISP_PPM					1

#define DEFAULT_ALARM_SP		2500
#define DEFAULT_PRE_ALARM_SP	1500


#define CO2_RESET_PIN		PDout(0) 
#define BEEP_PIN			PDout(3)

 

#define	ALARM_ON()		(BEEP_PIN = 1)
#define	ALARM_OFF()		(BEEP_PIN = 0)
// for debug
#define heart_beat()	(BEEP_PIN = !BEEP_PIN)

#define	ON		1
#define	OFF 	0

#define	STOP_ALARM			0x00
#define PRE_ALARM			0x01
#define	CONTINUOUS_ALARM	0x02
#define NO_SENSOR_ALARM		0x03
#define	ALARM_MANUAL		0x80

#define	INTERNAL_CO2_SENSOR	0
#define	EXTERNAL_CO2_SENSOR	1

#define	INTERNAL_CO2_EXIST_CTR		50
#define	MAX_CO2_REQUIRE_FAIL_CTR	10

#define	ALARM_ON_TIME_MAX			20
#define	ALARM_OFF_TIME_MAX			20

#define	SLOPE_CHECK_TIMES			5
typedef enum
{
  NONE 			=   0	,
  MAYBE_OGM200	=	1	,
  MAYBE_TEMCO_CO2		,
  OGM200				,
  TEMCO_CO2				,
	SCD30,  // 5 
	MH_Z19B,  // 6
  SENSOR_TYPE_ALL,
}CO2_SENSOR_TYPE;

typedef enum
{
	SCD30_CMD_NULL,
	SCD30_CMD_CONTINUE_MEASUREMENT,
	SCD30_CMD_GET_DATA_READY,
	SCD30_CMD_READ_MEASUREMENT,
	SCD30_CMD_READ_ASC,
	SCD30_CMD_ENABLE_ASC,
	SCD30_CMD_DISABLE_ASC,
	SCD30_SET_FRC,
}SCD30_CMD_LIST;

typedef struct _CO2_STRUCT_
{
	uint16 co2_int;
	float pre_co2_int;
	int16 co2_offset;
	uint16 pre_alarm_setpoint;
	uint16 alarm_setpoint;
	uint8 alarm_state;
	uint8 fail_counter;
	uint8 warming_time;
} CO2_STR;

extern CO2_STR far int_co2_str;
extern uint16 co2_asc;
//extern CO2_STR far ext_co2_str[MAX_EXT_CO2];

extern U16_T co2_int;
//extern U16_T previous_co2;
extern  uint16 co2_data_temp;
extern U8_T alarm_state;
extern U8_T alarm_status;
extern U8_T pre_alarm_on_time;
extern U8_T pre_alarm_off_time;
extern U8_T internal_co2_exist;
extern bit co2_refresh_flag;
extern U8_T pre_alarm_level;
extern U8_T alarm_delay_time;

extern U16_T co2_slope_detect_value;
extern U8_T int_co2_filter;

extern U8_T internal_co2_module_type;
extern uint8 internal_co2_bad_comms;
extern u8 read_co2_ctr ; 
extern U8_T scd30_co2_cmd_status;

typedef enum
{
	SLAVE_MODBUS_SERIALNUMBER_LOWORD = 0,
	SLAVE_MODBUS_SERIALNUMBER_HIWORD  = 2,
	SLAVE_MODBUS_VERSION_NUMBER_LO   = 4,
	SLAVE_MODBUS_VERSION_NUMBER_HI,
	SLAVE_MODBUS_ADDRESS,
	SLAVE_MODBUS_PRODUCT_MODEL,
	SLAVE_MODBUS_HARDWARE_REV,	 
	SLAVE_MODBUS_PIC_VERSION,
	SLAVE_MODBUS_ADDRESS_PLUG_N_PLAY,
	SLAVE_MODBUS_CUSTOM_ADDRESS = 14,
	SLAVE_MODBUS_BASE_ADDRESS = 15,
	SLAVE_MODBUS_UPDATE_STATUS = 16,

	SLAVE_MODBUS_CO2 = 100,	// 100
//	SLAVE_MODBUS_PIC_ANALOG1,
//	SLAVE_MODBUS_PIC_ANALOG2,
//	SLAVE_MODBUS_PIC_ANALOG3,
//	SLAVE_MODBUS_PIC_ANALOG4,
//	SLAVE_MODBUS_PIC_OUTPUT,

	SLAVE_MODBUS_CO2_OFFSET  ,
	SLAVE_MODBUS_CO2_ALARM_SETPOINT,
	SLAVE_MODBUS_CO2_PRE_ALARM_SETPOINT,
	
	SLAVE_MODBUS_CO2_SLOPE_DETECT_VALUE,
	SLAVE_MODBUS_CO2_FILTER,

	SLAVE_MODBUS_ANALOG1_FILTER,
	SLAVE_MODBUS_ANALOG2_FILTER,
	SLAVE_MODBUS_ANALOG3_FILTER,
	SLAVE_MODBUS_ANALOG4_FILTER,

	SLAVE_MODBUS_PIC_DISCONNECT,

	SLAVE_MODBUS_REGISTER_END,
} SLAVE_MODBUS_LIST;

void co2_alarm(void);

void receive_internal_co2_ppm(U8_T *p);
void receive_external_co2_ppm(U8_T index, U8_T *p);

//void vStartCo2Task(unsigned char uxPriority);

void Co2_task(void *pvParameters );
void Alarm_task(void *pvParameters );

extern uint16 test[20];
extern uint8 co2_autocal_disable;

#endif


