#ifndef __KEY_H
#define __KEY_H
 
#include "config.h" 


#define KEY_NON		0X00
#define	KEY_1		0X1000	//PC0
#define	KEY_2		0X2000	//PC1
#define	KEY_3		0X4000	//PC2
#define	KEY_4		0X8000	//PC3 


#define	KEY_LEFT	(1 << 12)
#define	KEY_RIGHT	(1 << 13)
#define	KEY_UP		(1 << 14)
#define	KEY_DOWN	(1 << 15)

#define	KEY_UP_MASK			(1 << 0)
#define	KEY_DOWN_MASK		(1 << 1)
#define	KEY_LEFT_MASK		(1 << 2)
#define	KEY_RIGHT_MASK		(1 << 3)
							
 
#define KEY_SPEED_1			(0x0000)
#define KEY_SPEED_10		(0x0100)
#define KEY_SPEED_50		(0x0200)
#define KEY_SPEED_100		(0x0300)
#define KEY_SPEED_MASK		(0x00ff)
#define KEY_FUNCTION_MASK	(0xff00)

#define	LONG_PRESS_TIMER_SPEED_100	70
#define	LONG_PRESS_TIMER_SPEED_50	50
#define	LONG_PRESS_TIMER_SPEED_10	30
#define	LONG_PRESS_TIMER_SPEED_1	10

#define SPEED_100		111 
#define SPEED_50		55
#define SPEED_10		11
#define SPEED_1			1


extern xQueueHandle qKey;
extern u8 global_key;
extern u16 pre_key;
void vKEYTask( void *pvParameters );
					    
#endif
