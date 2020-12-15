#ifndef _AQI_H_
#define _AQI_H_

#include "types.h"
#include "define.h" 
#include "bitmap.h" 


#define AQI_INDEX_RNG				500 

#define LEVEL0			50		//AQI:   0 ~ 50
#define LEVEL1			99		//AQI:  50 ~ 99
#define LEVEL2			149		//AQI:  99 ~ 149	
#define LEVEL3			200		//AQI: 149 ~ 200
#define LEVEL4			300		//AQI: 200 ~ 300
#define LEVEL5			500		//AQI: 300 ~ 500

enum
{
	GOOD = 0,
	MODERATE,
	POOL_FOR_SOME,
	UNHEALTHY,
	MORE_UNHEALTHY,
	HAZARDOUS,
};
 
extern uint16 aqi_table_customer[];
void get_aqi_value(uint16 PM_val, uint16 *AQI_val,uint8 *AQI_level);



#endif

