
#ifndef __MENU_CO2_CONF_H__

#define	__MENU_CO2_CONF_H__
#include "types.h"
#include "define.h"
extern uint8 current_co2_sensor;

void Co2_Conf_init(void);
void Co2_Conf_display(void);
void Co2_Conf_keycope(uint16 key_value);

#endif

