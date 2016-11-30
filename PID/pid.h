#ifndef _PID_H_
#define _PID_H_
#include "types.h"
#include "define.h"
#include "ud_str.h"
///*--------------------pid.h-------------------*/
/*--------------------pid.h-------------------*/
#define TSTAT_ARM

 

//#ifdef PIDTEST
//extern uint16 pidtest1;
//extern uint16 pidtest2;
//extern uint16 pidtest3;
//#endif

#ifndef NO_PID
	#define NO_PID	  		9
#endif

#ifndef	PID_LOOP1
	#define PID_LOOP1	  	0
#endif

#ifndef	PID_LOOP2
	#define PID_LOOP2	  	1
#endif

#ifndef PID_LOOP3
	#define PID_LOOP3	  	2
#endif


#ifndef	PID_TEMPERATURE
	#define PID_TEMPERATURE	  		0
#endif

#ifndef	PID_HUM
	#define PID_HUM	  				1
#endif

#ifndef	PID_CO2
	#define PID_CO2	  				2
#endif

 
#ifndef DAY_MODE
	#define DAY_MODE  			  	1
#endif

#ifndef OFF  
	#define OFF 					0
#endif

#ifndef FAN_OFF  
	#define FAN_OFF 				0
#endif


#define COOL_MODE		1
#define HEAT_MODE		0
#define TRANSMIT_MODE			0
#define PID_MODE				1



extern uint8 init_PID_flag ; 	
#ifdef TSTAT_ARM
  
//extern int16 idata loop_setpoint[3]; 
//extern int32 xdata iterm_sum_1sec[3];
//extern uint8 xdata pid3_input_select;
//extern uint8 xdata pid_setpoint[3] ;
//extern uint8 xdata pid[3];
//extern uint8  xdata universal_pid[3][6];  
//extern uint16 xdata cooling_db;
//extern uint16 xdata heating_db;
//extern uint8 xdata fan_speed_user;

//extern uint8 override_timer;
//extern uint8 occupied; 
//extern int16 xdata mul_analog_input[10] ;
//extern int16 xdata mul_analog_in_buffer[10];
//extern int16 xdata pre_mul_analog_input[10]; //used to filter  readings
//extern uint16 xdata co2_data;
//extern int16 xdata humidity;									
//extern int16 idata temperature ; //global temperature x 10, filtered,

typedef struct 
{
//	uint8 EEP_Mode; 
//	uint8 EEP_PTERM;
//	uint8 EEP_ITERM; 
 
	int16 EEP_SetPoint; 
	uint16 EEP_Input_Value;
	uint8 EEP_Pid; 
	uint8 Set_Flag;
}STR_PID; 
extern STR_PID PID[MAX_CONS];
extern uint8 mode_select;
void vStartPIDTask(void *pvParameters);
extern Str_controller_point controllers[MAX_CONS]; 
#endif


 

#endif
