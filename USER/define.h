#ifndef _DEFINE_H
#define _DEFINE_H
#include "ProductModel.h"
#define STM32F10X
//#define NO_HUM
extern unsigned char PRODUCT_ID;
 
//#if (PRODUCT_ID == STM32_CO2_NET)
//		#define   	    CO2_SENSOR
//		#define			NET_ENABLE
//#elif (PRODUCT_ID == STM32_CO2_RS485) 
//		#define   	    CO2_SENSOR
//		
//#elif (PRODUCT_ID == STM32_HUM_NET)
//		#define   	    HUM_SENSOR
// 		#define			NET_ENABLE
//#elif (PRODUCT_ID == STM32_HUM_RS485) 
//		#define   	    HUM_SENSOR
//		
//#elif (PRODUCT_ID == STM32_PRESSURE_NET)
//		#define   	    PRESSURE_SENSOR
//		#define			NET_ENABLE
//#elif (PRODUCT_ID == STM32_PRESSURE_RS485) 
//		#define   	    PRESSURE_SENSOR

//#endif

//                	#define 		CO2_SENSOR
//    	         	#define 		PRESSURE_SENSOR
//                 	#define   	    HUM_SENSOR	

/**********************************************
BIT:	 0		 1		 2		   3   		4 
		CO2		HUM		TEMP	PRESSURE
***********************************************/
//#ifdef CO2_SENSOR
 	#define  SENSOR_TYPE1	0x07
//#elif defined PRESSURE_SENSOR	
 	#define  SENSOR_TYPE2	0x08
//#elif defined HUM_SENSOR	
 	#define  SENSOR_TYPE3	0x06
//#else
//	#define  SENSOR_TYPE	0x00
//#endif

//typedef enum{
//	DOT_MATRIX_SCREEN,
//	COLOR_SCREEN,
//}eScreenType;

//#define DOT_MATRIX_SCREEN   //COLOR_SCREEN  //

#ifdef STM32F10X

typedef enum{
  NO_ERROR       = 0x00, // no error
  ACK_ERROR      = 0x01, // no acknowledgment error
  CHECKSUM_ERROR = 0x02, // checksum mismatch error
  TIMEOUT_ERROR  = 0x04, // timeout error
  PARM_ERROR     = 0x80, // parameter out of range error
}etError;


#define far 
#define xdata
#define idata
#endif

#define READ_WRITE_PROPERTY 1


#define SOFTREV    58
 

#define DEFAULT_FILTER  5 
#define T38AI8AO6DO   	44
#define PM_CO2_NET   	32 


#define	BAUDRATE_9600			9600			//0
#define	BAUDRATE_19200			19200			//1
#define	BAUDRATE_38400			38400			//2
#define	BAUDRATE_57600			57600			//3
#define	BAUDRATE_115200			115200		//4
#define	BAUDRATE_76800			76800		//5


#define EN_OUT 0 
#define EN_IN  1
#define EN_VAR 2

 


 

 
#define INPUT_CONTROL
#define OUTPUT_CONTROL



 

 
#define SWITCH_NUM 14

//#define PRODUCT_ID 				PM_CO2_NET
#define HW_VER					8
typedef enum{ 
	T_CHANNEL = 0,
	H_CHANNEL,
	C_CHANNEL,
	P_CHANNEL,
	MAX_AI_CHANNEL = 4,
}MAP_INPUT;
 
//#define MAX_AIS         		0
#define MAX_INS  				7 		//temp,hum,co2,pressure,pm2.5,pm10,light,

 
#define MAX_AOS					3		//output1,output2,output3
#define MAX_DO					0
#define MAX_OUTS				(MAX_AOS + MAX_DO)

 
#define MAX_AVS  				 36
//#define MAX_VARS				4 
#define MAX_CONS				3

#define  SW_OFF  0
#define  SW_HAND 2
#define  SW_AUTO 1

////IO操作函数	 
//#define CHA_SEL0				PAout(0)	//SCL
//#define CHA_SEL1				PAout(1)	//SDA	 
//#define CHA_SEL2				PAout(2)		//输入SDA
//#define CHA_SEL3				PAout(3)		//输入SDA
//#define CHA_SEL4				PAout(4)

//#define RANGE_SET0			PCout(8)
//#define RANGE_SET1			PCout(9)



//#define PRODUCT_ID 		T38AI8AO6DO
//#define HW_VER			8
//#define MAX_AI_CHANNEL	 8
//#define MAX_AIS         MAX_AI_CHANNEL
//#define MAX_INS  MAX_AIS

//#define MAX_AOS					8
//#define MAX_DOS					6 
//#define MAX_OUTS				(MAX_AOS+MAX_DOS)
 
//IO操作函数	 
//#define CHA_SEL0				PCout(0)		//		PIN15
//#define CHA_SEL1				PCout(1)	//	 		PIN16
//#define CHA_SEL2				PCout(2)		//	PIN13
 

//#define CHA_SEL4				PAout(7)
//#define RANGE_SET0			PAout(5)
//#define RANGE_SET1			PAout(4)


#define	MAX_EXT_CO2					254
#define	SCAN_DB_SIZE				5
 
 

// #define  printf	 my_print

#endif 

 

