
#ifndef	__LCD_H__ 
#define	__LCD_H__

#include "bitmap.h"
#include "types.h"
#include "define.h"
#include "stdbool.h"



#ifndef	TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

extern bool isColorScreen;

#if 1//defined (DOT_MATRIX_SCREEN)
#define LCD_CS		PEout(11)
#define LCD_AO		PEout(13)
#define LCD_RST		PEout(12)
#define LCD_CLK		PEout(14)
#define LCD_DATA_W	PEout(15)
#define LCD_DATA_R	PEin(15)


#define LCD_BL		PEout(10)
#define BL_ON()		(LCD_BL = 0)
#define BL_OFF()	(LCD_BL = 1)

#define LCD_SCL		PEout(14)
#define LCD_SDA		PEout(15)
#define LCD_RES		PEout(12)// PD15
 
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
extern bool alarmEnable;
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
//void Lcd_Show_String(uint8 pos_row, uint8 pos_line, uint8 disp_mode, uint8 *str);	// pos_x: 0 - 4 , pos_y: 0 - 15
uint8 Lcd_Show_Data(uint8 pos_row, uint8 pos_line, uint16 number, uint8 dot, uint8 align, uint8 mode);
void Lcd_Full_Screen(uint8 full);
//void Lcd_Show_two_digitals(uint8 pos_row, uint8 pos_line, uint8 dat);
//void Lcd_Clear_Row(uint8 row);
void update_cursor(void);
void cursor_on(uint8 row, uint8 line, uint8 onByte, uint8 offByte);
void cursor_off(void);
	 
void scrolling_message(void);

void update_message_context(void);
	 
//void start_back_light(uint8 timer);
void poll_back_light(void);

void start_scrolling(void);
void stop_scrolling(void);

//void get_time_text(void);
//void itoa(uint16 num, uint8 *p, uint8 dot);

extern uint8 main_net_status_ctr; 
void reset_main_net_status_counter(void);
//void poll_main_net_status(void);

#endif   // (DOT_MATRIX_SCREEN)









#if 1//defined (COLOR_SCREEN)




#define FORM32X64 		0
#define FORM15X30			1

#define TSTAT8_CH_COLOR   	0xffff
#define TSTAT8_MENU_COLOR   0x7e17

#define SCH_COLOR  0xffff//0XB73F
#define SCH_BACK_COLOR  0x3bef//0x43f2//0x14E9

#define TSTAT8_BACK_COLOR1  0x7E19
#define TSTAT8_BACK_COLOR   0x7E19//
//#define TSTAT8_MENU_COLOR2  0x7e17
#define TSTAT8_MENU_COLOR2  0x7e10
#define TANGLE_COLOR        0xbe9c

#define SCOROLL_BUFF_LEN		15
extern uint8 *scroll;
extern uint8 scroll_ram[20][SCOROLL_BUFF_LEN];
extern uint8 fan_flag;
extern uint8 display_flag;
extern uint8 schedule_hour_minute; //indicate current display item is "hour" or "minute"
extern uint8 blink_parameter;
extern uint8 clock_blink_flag;
void LCDtest(void);
extern uint8 const chlib[];
extern uint8 const chlibsmall[];
extern uint8 const chlibmiddle[];
extern uint16 const athome[];
extern uint16 const offhome[];
extern uint16 const sunicon[];
extern uint16 const moonicon[];
//extern uint16 const heaticon[]; 
//extern uint16 const coolicon[];
extern uint16 const aqiicon[];
extern uint16 const pm25icon[];
extern uint16 const pressicon[];

extern uint16 const degree_o[];
//extern uint16 const therm_meter[];
extern uint16 const leftup[];
extern uint16 const leftdown[];
extern uint16 const rightdown[];
extern uint16 const rightup[];
extern uint16 const cmnct_send[]; 
extern uint16 const cmnct_rcv[]; 
extern uint16 const wifinocnnct[];
extern uint16 const wificonnect[];

extern U8_T const   network_status_text[];
extern U8_T const   net_status_ok_text[];
extern U8_T const   net_status_dead_text[];
extern U8_T const   net_offline_text[];
extern U8_T const   alarm_text[];
extern U8_T const   alarm_status_text[4][9];
extern U8_T date[];
extern U8_T color_time[];

u8 READ_LINK(void);
u8 READ_READY(void);

extern uint8_t screenArea1, screenArea2, screenArea3;
extern bool enableScroll;

typedef struct   
{
 uint8 unit;
 uint8 setpoint;
 uint8 fan;
 uint8 sysmode;
 uint8 occ_unocc;
 uint8 heatcool;
 uint8 fanspeed;
 uint8 cmnct_send;
 uint8 cmnct_rcv; 	
} DISP_CHANGE;

extern DISP_CHANGE icon;
//extern uint16 const angle[];
void draw_tangle(uint8 xpos, uint16 ypos);
void ClearScreen(unsigned int bColor);
void disp_ch(uint8 form, uint16 x, uint16 y,uint8 value,uint16 dcolor,uint16 bgcolor);		
void disp_icon(uint16 cp, uint16 pp, uint16 const *icon_name, uint16 x,uint16 y,uint16 dcolor, uint16 bgcolor);
void disp_null_icon(uint16 cp, uint16 pp, uint16 const *icon_name, uint16 x,uint16 y,uint16 dcolor, uint16 bgcolor);
void disp_str(uint8 form, uint16 x,uint16 y,uint8 *str,uint16 dcolor,uint16 bgcolor);	
void display_SP(int16 setpoint);
void display_fanspeed(int16 speed);
void display_mode(void);
void display_fan(void);
void display_icon(void);
void display_value(uint16 pos,int16 disp_value, uint8 disp_unit);
//void display_menu(uint16 pos, uint8 *item);
void display_menu (uint8 *item1, uint8 *item2);
void clear_line(uint8 linenum);
void clear_lines(void);
//void display_clock_date(int8 item, int16 value);
//void display_clock_time(int8 item, int16 value);
void display_scroll(void);
void scroll_warning(void);
void display_schedule_time(int8 schedule_time_sel, uint8 hour_minute);
void Top_area_display(uint8 item, int16 value, uint8 unit);
extern void display_pm25( int16 value);
#define CH_HEIGHT													32

#define THERM_METER_POS										0
#define HUM_POS												THERM_METER_POS+80
#define CO2_POS												THERM_METER_POS+120+5
#define CO2_POSY_OFFSET										20
#define MED_DIS_OFFSET                    -3
#define SETPOINT_YPOS											220
#define SETPOINT_XPOS											80+MED_DIS_OFFSET
#define FAN_MODE_XPOS											SETPOINT_XPOS+CH_HEIGHT+12
#define SYS_MODE_XPOS											FAN_MODE_XPOS+CH_HEIGHT+12

#define MENU_FIRST_XPOS									10
#define MENU_SECOND_XPOS								34
#define MENU_THIRD_XPOS									58
#define MENU_FORTH_XPOS									82
#define MENU_FIFTH_XPOS									106

#define MENU_ITEM1      SETPOINT_XPOS+0
#define MENU_ITEM2      FAN_MODE_XPOS+0


#define TIME_POS										      0//SYS_MODE_POS + CH_HEIGHT + 7
#define MENU_ITEM_POS											130
#define MENU_VALUE_POS										SYS_MODE_XPOS//MENU_ITEM_POS + CH_HEIGHT
#define ICON_POS													254
#define ICON_XPOS 												2

#define FIRST_ICON_POS									  ICON_XPOS
#define SECOND_ICON_POS                   FIRST_ICON_POS + 50
#define THIRD_ICON_POS                    SECOND_ICON_POS + 50
#define FOURTH_ICON_POS                   THIRD_ICON_POS + 50
#define FIFTH_ICON_POS                    FOURTH_ICON_POS//FOURTH_ICON_POS + 40

#define ICON_XDOTS						45//55
#define ICON_YDOTS						55//45

#define FANBLADE_XDOTS						45//40
#define FANBLADE_YDOTS						40//45

#define FANSPEED_XDOTS						45//15
#define FANSPEED_YDOTS						15//45

#define THERM_METER_XPOS									20
#define TEMP_FIRST_BLANK						      0//30  //+= blank width
#define FIRST_CH_POS											200//TEMP_FIRST_BLANK + THERM_METER_XPOS
#define SECOND_CH_POS											FIRST_CH_POS-40   //160
#define THIRD_CH_POS											SECOND_CH_POS-40-16   //144
#define UNIT_POS													THIRD_CH_POS - 14 - 24    //106
#define BUTTON_DARK_COLOR   							0X0BA7
#define BTN_OFFSET												CH_HEIGHT+7


#define TOP_AREA_DISP_ITEM_TEMPERATURE   	0
#define TOP_AREA_DISP_ITEM_HUM					 	1
#define TOP_AREA_DISP_ITEM_CO2				   	2
#define TOP_AREA_DISP_ITEM_PM25						3

#define TOP_AREA_DISP_UNIT_C   					 	0
#define TOP_AREA_DISP_UNIT_F					 	 	1
#define TOP_AREA_DISP_UNIT_PPM				   	2
#define TOP_AREA_DISP_UNIT_PERCENT			 	3



#define SCREEN_AREA_TEMP   0
#define SCREEN_AREA_HUMI	1
#define SCREEN_AREA_CO2		2
#define SCREEN_AREA_NONE	3
#define SCREEN_AREA_PM25	4
#define SCREEN_AREA_PRESSURE	5


//extern uint8 const   AQI_LEVEL[6][15];
//extern struct _CURSOR_ cursor;
void Lcd_Write_Byte(uint8 num); //from high to low
void Lcd_Write_Command(uint8 command); //from high to low
//void Lcd_Write_Char(uint8 row, uint8 line, uint8 num, uint8 disp_nor_or_inv); // row: 0 - 4 , line: 0 - 15
void Lcd_Set_Fuction(uint8 mode);
void Lcd_Set_Y_Addr(uint8 page);// 0 1 0 0        page 0 - 9
void Lcd_Set_X_Addr(uint8 line);// 1 1 1 0/1        page 0 - 129
void Lcd_Initial(void);
void Lcd_Show_String(uint8 pos_row, uint8 pos_line, uint8 disp_mode, uint8 *str);	// pos_x: 0 - 4 , pos_y: 0 - 15
//uint8 Lcd_Show_Data(uint8 pos_row, uint8 pos_line, uint16 number, uint8 dot, uint8 align, uint8 mode);
//void Lcd_Full_Screen(uint8 full);
//void Lcd_Show_two_digitals(uint8 pos_row, uint8 pos_line, uint8 dat);
void Lcd_Clear_Row(uint8 row);
void update_cursor(void);
void cursor_on(uint8 row, uint8 line, uint8 onByte, uint8 offByte);
void cursor_off(void);
	 
void scrolling_message(void);

void update_message_context(void);
	 
extern void start_back_light(uint8 timer);
extern void poll_back_light(void);

void start_scrolling(void);
void stop_scrolling(void);

void get_time_text(void);
void itoa(uint16 num, uint8 *p, uint8 dot);

//extern uint8 main_net_status_ctr; 
void reset_main_net_status_counter(void);
void poll_main_net_status(void);

//void itoa(uint16 num, uint8 *p, uint8 dot);

void clear_line(uint8 linenum);
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

#endif  // (COLOR_SCREEN)


#endif   // LCD_H 
