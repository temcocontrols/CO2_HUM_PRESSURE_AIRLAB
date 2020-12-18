#ifndef __LED_H
#define __LED_H
 
#include "bitmap.h"

#define LED_ON	0
#define LED_OFF	1	

 
 
#define LED_NET_BEAT    PEout(0) // blue led
#define LED_CO2_GOOD    PEout(1) // green led
#define LED_CO2_FAIR    PEout(2) // yellow led
#define LED_CO2_POOR    PEout(3) // red led 
#define CO2_RESET				PEout(8)

//#define LED_HEARTBEAT   PDout(6) // blue led

void LED_Init(void);
void refresh_alarm_status_led(char Led_Status);
void refresh_net_status_led(char Led_Status);
#endif

















