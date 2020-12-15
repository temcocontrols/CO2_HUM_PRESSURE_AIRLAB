#ifndef __MODBUS_H
#define	__MODBUS_H
#include <string.h>
#include "stm32f10x.h"
//IO操作函数	 
#include "bitmap.h"
#include "crc.h"
#include "define.h"
#include "filter.h"
#include "delay.h"

#ifndef FALSE
#define FALSE 0 
#endif 

#ifndef TRUE
#define TRUE 1 
#endif 

#define TXEN		PAout(8)


#define SEND			1			//1
#define	RECEIVE		0

#define	READ_VARIABLES				0x03
#define	WRITE_VARIABLES				0x06
#define	MULTIPLE_WRITE				0x10
#define	CHECKONLINE					0x19

#define DATABUFLEN					200
#define DATABUFLEN_SCAN				12
#define SENDPOOLLEN         		8








#define SERIAL_COM_IDLE				0
#define INVALID_PACKET				1
#define VALID_PACKET				2

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define USART_SEND_LEN			490//255

#define RESPONSERANDVALUE	1


#define NONE_PARITY					0
#define ODD_PARITY					1
#define EVEN_PARITY					2


extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
extern vu8 revce_count ;
extern u8 dealwithTag ;

void serial_restart(void);
void modbus_data_cope(u8 XDATA* pData, u16 length, u8 conn_id) ;
void modbus_init(void) ;
void SoftReset(void) ;
typedef union {
         unsigned long  word;
         unsigned int   half[2];
         unsigned char  quarter[4];
      }double_read;


typedef struct 
{
	u8 serial_Num[4];
//	u16 software ;
	u8 address ;
	u32 baudrate ;
	u8	baud ;
	u8 update ;
	u8 product ;
	u8 hardware_Rev;
	u8 SNWriteflag ;
	u8 com_config;
	u8 protocal ;
	u8 reset ;
	u8 mac_addr[6] ;
	u8 ip_addr[4]  ;
	u8 ip_mode     ;
	u8 mask_addr[4] ;
	u8 gate_addr[4] ;
	u8 tcp_server ;
	u16 listen_port ;
	u8 mac_enable ;
	u8 ghost_ip_addr[4]  ;
	u8 ghost_ip_mode     ;
	u8 ghost_mask_addr[4] ;
	u8 ghost_gate_addr[4] ;
	u8 ghost_tcp_server ;
	u16 ghost_listen_port ;
	
	u8 write_ghost_system  ;
//	#ifdef T322AI
////	u16 customer_range_hi[MAX_AI_CHANNEL];
////	u16 customer_range_lo[MAX_AI_CHANNEL];
////	u16	raw_data[MAX_AI_CHANNEL];
//	u16 input[MAX_AI_CHANNEL] ;
//	u8  filter_value[MAX_AI_CHANNEL];
//	u8  range [MAX_AI_CHANNEL];
//	u16  offset[MAX_AI_CHANNEL];
////	u8   customer_enable[MAX_AI_CHANNEL] ;
//	double_read pulse[MAX_AI_CHANNEL] ;
//	#endif
//	#ifdef T38AI8AO6DO
////	u16 customer_range_hi[MAX_AI_CHANNEL];
////	u16 customer_range_lo[MAX_AI_CHANNEL];
////	u8   customer_enable[MAX_AI_CHANNEL] ;
////	u16 input[MAX_AI_CHANNEL] ;
////	u8  filter_value[MAX_AI_CHANNEL];
////	u8  range [MAX_AI_CHANNEL];
////	u16  offset[MAX_AI_CHANNEL];
//	u16	 switch_gourp[2] ;
////	u8 	digit_output[MAX_DO] ;
////	u16 output[MAX_AO+MAX_DO] ;
//	double_read pulse[MAX_AI_CHANNEL] ;
//	#endif 
 
//	u16 customer_table1_vol[11];
//	u16 customer_table1_val[11];
//	u16 customer_table2_vol[11];
//	u16 customer_table2_val[11];
//	u16 customer_table3_vol[11];
//	u16 customer_table3_val[11];
//	u16 customer_table4_vol[11];
//	u16 customer_table4_val[11];
//	u16 customer_table5_vol[11];
//	u16 customer_table5_val[11];
//	u8  table_point[5] ;
 

	
//	#ifdef T3PT12

//	u16 input[MAX_AI_CHANNEL] ;
//	u8  filter_value[MAX_AI_CHANNEL];
//	u8  range [MAX_AI_CHANNEL];
//	u16  offset[MAX_AI_CHANNEL];
//	u8 	  resole_bit ;	
//	#endif 	

	u8 protocal_timer_enable;
	
}STR_MODBUS ;

typedef struct
{
	u8 rx_count;
	u8 tx_count;
}STR_UART;
extern STR_UART uart;
extern signed int short ctest[20];
extern STR_MODBUS modbus ;
extern vu8 serial_receive_timeout_count ;
 void dealwithData(void) ;
//extern u8 i2c_test[10] ; 
void send_byte(u8 ch, u8 crc) ;
void responseCmd(u8 type, u8* pData); 
void internalDeal(u8 type,  u8 *pData) ;
void USART_SendDataString(u16 num) ;
extern u8 uart_send[USART_SEND_LEN] ;
extern u8 SERIAL_RECEIVE_TIMEOUT ;
extern u8  	Station_NUM;
extern u8 dis_hum_info;
extern u8 reply_delay_time;
extern u8 receive_delay_time;
extern u8 reply_done;
extern u8 uart1_parity;

void Inital_Bacnet_Server(void);
void EEP_Dat_Init(void) ;

u16 swap_int16( u16 value) ;
u32 swap_int32( u32 value) ;
void stack_detect(u16 *p);
void Data_Deal(u16 StartAdd,u8 Data_H,u8 Data_L);


#endif
