
#ifndef __SPEC_FONT_H__

#define	__SPEC_FONT_H__


#include "types.h"

void print_big_number(U16_T num, U8_T dot);
void print_online_status(bit flag);
void print_alarm_status(U8_T alarm_level);

void Write_Pres_Char(uint8 row, uint8 line, uint8 c);
void Lcd_Show_Pres(uint8 pos_row, uint8 pos_line,uint8 *str);

#endif



