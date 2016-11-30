#ifndef _ANALOG_INPUTS_H_
#define _ANALOG_INPUTS_H_

#include "types.h"
#include "define.h" 
#include "bitmap.h"    
 
typedef enum
{
	HUM_VOL_FB = 0,
	TEMP_VOL_FB,
	CO2_VOL_FB,	
	HUM_CUR_FB, 
	TEMP_CUR_FB,	 
	CO2_CUR_FB,		 
  	PRE_AD,
	TEMP_AD,
	AD_MAX_CHANNEL,
}AD_CHANNEL_SEL;



void input_initial(void);
uint16 get_ad_val(uint8 channel); 
 



#endif































