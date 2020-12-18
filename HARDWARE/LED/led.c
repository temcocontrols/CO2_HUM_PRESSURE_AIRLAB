#include "led.h"

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//LED0/1/2/3/4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_SetBits(GPIOE, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3); 	

//	LED_NET_BEAT  = LED_OFF;
//    LED_CO2_GOOD  = LED_OFF;
//    LED_CO2_FAIR  = LED_OFF;
//    LED_CO2_POOR  = LED_OFF;

//    LED_HEARTBEAT = LED_OFF;

}


void refresh_alarm_status_led(char Led_Status)
{   	
	if(Led_Status &0x02) 
	{
		LED_CO2_GOOD = LED_ON;//STM_EVAL_LEDOn(LED_CO2_GOOD); 
		LED_CO2_FAIR = LED_OFF;//STM_EVAL_LEDOff(LED_CO2_FAIR); 
		LED_CO2_POOR = LED_OFF;//STM_EVAL_LEDOff(LED_CO2_POOR);  
	}
	else if(Led_Status &0x04) 
	{
		LED_CO2_GOOD = LED_OFF; //STM_EVAL_LEDOff(LED_CO2_GOOD); 
		LED_CO2_FAIR = LED_ON;//STM_EVAL_LEDOn(LED_CO2_FAIR); 
		LED_CO2_POOR = LED_OFF;//STM_EVAL_LEDOff(LED_CO2_POOR);  
	}
	else if(Led_Status &0x08) 
	{
		LED_CO2_GOOD = LED_OFF;//STM_EVAL_LEDOff(LED_CO2_GOOD); 
		LED_CO2_FAIR = LED_OFF;//STM_EVAL_LEDOff(LED_CO2_FAIR); 
		LED_CO2_POOR = LED_ON;//STM_EVAL_LEDOn(LED_CO2_POOR);  
	}
}

void refresh_net_status_led(char Led_Status)
{
   if(Led_Status & 0x01)
   {
      LED_NET_BEAT = ~LED_NET_BEAT;//STM_EVAL_LEDToggle(LED_HEARTBEAT);
   }
   else
   {
      LED_NET_BEAT = LED_OFF;//STM_EVAL_LEDOff(LED_HEARTBEAT);
   }
}
