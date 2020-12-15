#ifndef _SUB_NET_H_
#define _SUB_NET_H_
#include "config.h"
 
#define	SUB_BUF_LEN			20
#define	UART3_BUF_LEN			20



enum
{  
	REG_CO2_VALUE = 109,
};


extern U8_T subnet_response_buf[ ];
extern U8_T subnet_rec_package_size;
extern xQueueHandle qSubSerial;
//extern U8_T subnet_idle;

 
void set_sub_serial_baudrate(U32_T BR);
U8_T wait_subnet_response(U8_T nDoubleTick);
void sub_send_string(U8_T *p, U8_T length);
void set_subnet_parameters(U8_T io, U8_T length);
void read_from_slave(uint16 addr);
int8 DEAL_TEMCO_SENSOR(void);
void sub_net_init(void);
uint8 get_uart2_length(void);
void serial1_restart(void);
#endif





