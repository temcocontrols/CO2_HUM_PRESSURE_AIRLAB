#ifndef _SUBNET_SCAN_H_
#define _SUBNET_SCAN_H_
#include "config.h"

#define UART3_TXEN		PBout(9)



#define UART3_MAX_BUF    20



void uart3_modbus_init(void);
void uart3_send_string(U8_T *p, U8_T length);
void request_external_co2(U8_T index);
void send_test(void);
void set_uart3_parameters(U8_T io, U8_T length);
U8_T wait_uart3_response(U8_T nDoubleTick);
U8_T wait_for_uart3_idle(U8_T nDoubleTick);


extern xQueueHandle qSubSerial3;
extern uint8 uart3_rec_package_size;
extern uint8 uart3_idle;

#endif
