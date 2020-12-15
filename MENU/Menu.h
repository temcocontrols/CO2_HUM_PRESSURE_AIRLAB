
#ifndef __MENU_H__

#define	__MENU_H__
#include "config.h"

#define MENU_BLOCK_SECONDS_DEFAULT		60
#define BACKLIGHT_KEEP_SECONDS_DEFAULT	30
#define SWTIMER_COUNT_SECOND	 configTICK_RATE_HZ   

struct _MENU_STATE_
{
	uint8 Index;
	void (*KeyCope)(U16_T);								
	void (*InitAction)(void);
	void (*DisplayPeriod)(void);
	uint8 BlockTime;
};

extern struct _MENU_STATE_ CurrentState;

extern bit menu_system_start;
extern bit in_sub_menu;
extern bit value_change;
extern bit previous_internal_co2_exist;
extern uint8 pre_db_ctr;

#define MAX_PASSWORD_DIGITALS	4
extern uint8 menu_password;
extern uint8 use_password;
extern uint8 password_index;
extern uint8 password_buffer[4];
extern uint8 user_password[4];

extern uint8 text[50];
extern uint8 int_text[21];
//extern uint16 set_value;

extern uint8 const   internal_text[];
extern uint8 const   external_text[];
extern uint8 const   ppm_text[];
//extern uint8 const code co2_text[];
extern uint8 const   int_space[];
//extern uint8 const code online_text[];
//extern uint8 const code offline_text[];
//extern uint8 warming_text[13];

extern uint8 menu_block_seconds;
extern uint8 backlight_keep_seconds; 
extern xQueueHandle xMutex,IicMutex;

 enum
{
	MenuIdle = 0,
		MenuMain,
			MenuCo2_Conf,
				MenuCo2Option,
			MenuTemp_Conf,
			MenuHum_Conf,
			MenuMisc,
 			MenuPresConf,
	MenuEnd,
};


void show_system_info(void);
void update_menu_state(uint8 MenuId);
void exit_request_password(void);
void vStartMenuTask(unsigned char uxPriority);
//void vStartScrollingTask(unsigned char uxPriority);
void print(char *p);

#endif

