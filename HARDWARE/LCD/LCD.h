
#ifndef	__LCD_H__ 
#define	__LCD_H__

#include "bitmap.h"
#include "types.h"
#include "define.h"



#ifndef	TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define LCD_CS		PEout(11)
#define LCD_AO		PEout(13)
#define LCD_BL		PEout(10)
#define LCD_RST		PEout(12)
#define LCD_CLK		PEout(14)
#define LCD_DATA_W	PEout(15)
#define LCD_DATA_R	PEin(15)

#define BL_ON()		(LCD_BL = 0)
#define BL_OFF()	(LCD_BL = 1)

 
#define	MAX_ROW		5
#define	MAX_LINE	21

#define MAX_INT		32767
#define MAX_BYTE	255
#define MAX_UINT	65535

#define LOWER		0
#define UPPER		1

#define	DISP_NOR	0
#define DISP_INV	1

#define ALIGN_LEFT	0
#define	ALIGN_RIGHT	1

#define	CURSOR_ON	1
#define	CURSOR_OFF	0

#define LCD_BL_ON_TIME	10
struct _CURSOR_
{
	uint8 row;
	uint8 line;
	uint8 on_byte;
	uint8 off_byte;
	uint8 status;
};
extern struct _CURSOR_ cursor;

extern bit scrolling_flag;

extern uint8 time[];
extern uint8 const   AQI_LEVEL[6][15];
// A0 = 0
//[H0 :H1] = [0 : 0]
#define CMD_SET_V0Range_LOW     0x04
#define CMD_SET_V0Range_HIGH    0x05
#define CMC_END					0X06
#define CMD_DISPLAT_ALLOFF      0X08
#define CMD_DISPLAT_NORMAL      0X0C
#define CMD_DISPLAT_ALLON       0X09
#define CMD_DISPLAY_INVERSE     0X0D


//[H0 :H1] = [0 : 1]
#define CMD_DISPLAY_CONFIG      0X08// DO = 1
#define CMD_SET_BIAS            0x11  //  1/10
#define CMD_SET_V0              0xae  //Set VOP value:AB=Vop=10.8V  =0aeh


//[H0 :H1] = [1 : 0]
#define CMD_SET_SHOWMODE        0x04   //full display 1/80 duty
//#define CMD_SET_SHOWPART 
//#define CMC_SET_STARTLINE

//[H0 :H1] = [1 : 1]
#define CMD_RESET               0x03
#define CMD_HIGH_POWER          0xb4
#define CMD_FRE_50              0x08
#define CMD_FRE_68				0x09
#define CMD_FRE_70				0x0a
#define CMD_FRE_73				0x0b
#define CMD_FRE_75				0x0c
#define CMD_FRE_78				0x0d
#define CMD_FRE_81				0x0e
#define CMD_FRE_150				0x0f

//void delay(uint8 num);
void Lcd_Write_Byte(uint8 num); //from high to low
void Lcd_Write_Command(uint8 command); //from high to low
void Lcd_Write_Char(uint8 row, uint8 line, uint8 num, uint8 disp_nor_or_inv); // row: 0 - 4 , line: 0 - 15
void Lcd_Set_Fuction(uint8 mode);
void Lcd_Set_Y_Addr(uint8 page);// 0 1 0 0        page 0 - 9
void Lcd_Set_X_Addr(uint8 line);// 1 1 1 0/1        page 0 - 129
void Lcd_Initial(void);
void Lcd_Show_String(uint8 pos_row, uint8 pos_line, uint8 disp_mode, uint8 *str);	// pos_x: 0 - 4 , pos_y: 0 - 15
uint8 Lcd_Show_Data(uint8 pos_row, uint8 pos_line, uint16 number, uint8 dot, uint8 align, uint8 mode);
void Lcd_Full_Screen(uint8 full);
//void Lcd_Show_two_digitals(uint8 pos_row, uint8 pos_line, uint8 dat);
void Lcd_Clear_Row(uint8 row);
void update_cursor(void);
void cursor_on(uint8 row, uint8 line, uint8 onByte, uint8 offByte);
void cursor_off(void);
	 
void scrolling_message(void);

void update_message_context(void);
	 
void start_back_light(uint8 timer);
void poll_back_light(void);

void start_scrolling(void);
void stop_scrolling(void);

void get_time_text(void);
void itoa(uint16 num, uint8 *p, uint8 dot);

extern uint8 main_net_status_ctr; 
void reset_main_net_status_counter(void);
void poll_main_net_status(void);
#endif
