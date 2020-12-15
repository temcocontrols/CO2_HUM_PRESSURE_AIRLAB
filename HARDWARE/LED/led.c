#include "led.h"
#include "modbus.h"
#include "controls.h" 

//u16 led_bank1 = 0 ;
//u16 led_bank2 = 0 ;
//u8  heart_beat_led =  0; 
u8	tx_count = 0 ;
u8  rx_count = 0 ;
//u8  net_rx_count = 0 ;
//u8  net_tx_count = 0 ;
//u8 dim_timer_setting[28];
//void LED_Init(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE , ENABLE);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOE, &GPIO_InitStructure);
//	GPIO_ResetBits(GPIOE, GPIO_InitStructure.GPIO_Pin);	
//}
// 

//void refresh_led(void)
//{
//	static u8 led_switch = 0 ;
//	led_switch = !led_switch ;
//	if(led_switch)
//	{
//		led_bank1 &= ~(1<<14);
//		led_bank1 |= (1<<15);
//		GPIO_Write(GPIOE, led_bank1) ;
//	}
//	else
//	{
//		
//		led_bank2 |= (1<<14);
//		led_bank2 &= ~(1<<15);
//		GPIO_Write(GPIOE, led_bank2) ;
//	}
//}


