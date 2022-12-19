
#ifndef __MENU_IDLE_H__

#define	__MENU_IDLE_H__
#include "types.h"
#include "define.h"

void Idle_init(void);
void Idle_display(void);
void Idle_keycope(uint16 key_value);
uint8_t check_idle_setp(void);
void check_setpoint_count(void);
void intial_setpoint(void);

extern uint8_t max_setpoint;
extern int8_t index_setpoint;
extern uint8_t icon_setpoint[3];
#endif

