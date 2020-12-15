#include "key.h"  
xQueueHandle qKey;
u8 global_key = KEY_NON;
u16 pre_key = KEY_NON;
//按键初始化函数
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能GPIOC时钟 
  	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPU;
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOA, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
} 

u8 KEY_Scan(void)
{	 
	u16 key_1st, key_2nd;
	u16 key_val = KEY_NON;
	 
	key_1st = ~GPIO_ReadInputData(GPIOA) & 0xf000; // PC0-3
	delay_ms(20);
	key_2nd = ~GPIO_ReadInputData(GPIOA) & 0xf000; // PC0-3
	
	if(key_1st & key_2nd & KEY_1)
		key_val |= KEY_1;
	
	if(key_1st & key_2nd & KEY_2)
		key_val |= KEY_2;
	
	if(key_1st & key_2nd & KEY_3)
		key_val |= KEY_3;
	
	if(key_1st & key_2nd & KEY_4)
		key_val |= KEY_4;
	 
	return  (key_val >> 12);
}
 extern void watchdog(void);
void vKEYTask( void *pvParameters )
{
	u16 key_temp;
	
	static U8_T long_press_key_start = 0;
	
 	KEY_Init();
	print("Key Task\r\n");
	delay_ms(100);
	
	for( ;; )
	{
		if ((PRODUCT_ID != STM32_CO2_NET)&&(PRODUCT_ID != STM32_HUM_NET)&&(PRODUCT_ID != STM32_PRESSURE_NET)&&(PRODUCT_ID != STM32_PM25)) 
		 watchdog();

		if((key_temp = KEY_Scan()) != pre_key)
		{
			xQueueSend(qKey, &key_temp, 0);
			pre_key = key_temp;
			long_press_key_start = 0;
		}
		else
		{
			if(key_temp != KEY_NON)
			{
				if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_100)
					key_temp |= KEY_SPEED_100;
				else if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_50)
					key_temp |= KEY_SPEED_50;
				else if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_10)
					key_temp |= KEY_SPEED_10;
				else if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_1)
					key_temp |= KEY_SPEED_1;

				if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_1)
					xQueueSend(qKey, &key_temp, 0);

				if(long_press_key_start < LONG_PRESS_TIMER_SPEED_100)
					long_press_key_start++;
			}
		} 

		vTaskDelay(100 / portTICK_RATE_MS);
//		stack_detect(&test[8]);
    }
}
 
