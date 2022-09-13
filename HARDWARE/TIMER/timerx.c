//#include "timerx.h"
//#include "led.h"
//#include "modbus.h"
//#include "../output/analog_output.h" 
//#incldue "FreeRTOS.h"

#include "config.h"

extern volatile uint16_t SilenceTime;
//u32 dhcp_run_time = 30000;
//��ʱ��3�жϷ������	 
void TIM3_IRQHandler(void)
{ 		    		  			    
//	if(TIM3->SR & 0X0001)		//����ж�
	if(TIM_GetFlagStatus(TIM3, TIM_IT_Update) == SET)
	{
		//LED1 = !LED1;			    				   				     	    	
	}
//	TIM3->SR &= ~(1 << 0);		//����жϱ�־λ 
	TIM_ClearFlag(TIM3, TIM_IT_Update);	
}

//ͨ�ö�ʱ��3�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc; 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	//Timer3 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;			//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);								//����ָ���Ĳ�����ʼ��NVIC�Ĵ���
	
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

//TIM3 PWM���ֳ�ʼ�� 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
//void TIM3_PWM_Init(u16 arr, u16 psc)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//	TIM_OCInitTypeDef TIM_OCInitStructure;
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //TIM3_CH1->PC6, TIM3_CH2->PC7, TIM3_CH3->PC8, TIM3_CH4->PC9
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;  //TIM3_CH1-4
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	
//	TIM_TimeBaseStructure.TIM_Period = arr;
//	TIM_TimeBaseStructure.TIM_Prescaler = psc;
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
//	
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
//	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
//	
//	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	
//	TIM_Cmd(TIM3, ENABLE);									  
//}  	 

//void set_pwm_duty(TIM_TypeDef* TIMx, u8 channel, u16 duty)
//{
//	u16 compare_value = 0;
//	// calculate the compare value and then set it to change the pwm duty
//	
//	switch(channel)
//	{
//		case 1:
//			TIM_SetCompare1(TIMx, compare_value);
//		break;
//		case 2:
//			TIM_SetCompare2(TIMx, compare_value);
//		break;
//		case 3:
//			TIM_SetCompare3(TIMx, compare_value);
//		break;
//		case 4:
//			TIM_SetCompare4(TIMx, compare_value);
//		break;
//	}
//}

/////////////////////////////////////////////////////////////////////////////////////////
u32 uip_timer = 0;	//uip ��ʱ����ÿ10ms����1.
//��ʱ��6�жϷ������	 
void refresh_net_status_led(char Led_Status);
void check_Task_locked(void);
void TIM6_IRQHandler(void)//1ms
{
//	if(TIM6->SR & 0X0001)	//����ж�
	static u16 led_count = 0;
	
	
	unsigned portBASE_TYPE uxSavedInterruptStatus;
	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	//if(Test[33] > 2000)	while(1) ;
	if(TIM_GetFlagStatus(TIM6, TIM_IT_Update) == SET)
	{
		uip_timer++;		//uip��ʱ������1
		
		if(SilenceTime < 5000)
		{
			//SilenceTime ++ ;
			//miliseclast_cur = miliseclast_cur + SWTIMER_INTERVAL;
			SilenceTime = SilenceTime + SWTIMER_INTERVAL;
		}
		else
		{
				SilenceTime = 0 ;
		}
		
	//	if(dhcp_run_time >0) dhcp_run_time -- ;
		if (PRODUCT_ID == STM32_HUM_RS485)  // for heart beat
		{
			if(led_count++ >= 100)
			{			
				refresh_net_status_led(led_count % 2);
				check_Task_locked();
				led_count= 0 ;
			}
		}
		else
		{
			if(led_count++ >= 1000)
			{			
				check_Task_locked();
				led_count= 0 ;
			}
		}
	}
//	TIM6->SR &= ~(1 << 0);	//����жϱ�־λ 
	TIM_ClearFlag(TIM6, TIM_IT_Update);

	portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);	
}

//������ʱ��6�жϳ�ʼ��					  
//arr���Զ���װֵ��		
//psc��ʱ��Ԥ��Ƶ��		 
//Tout= ((arr+1)*(psc+1))/Tclk��
void TIM6_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
		
	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc; 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	
	//Timer3 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);								//����ָ���Ĳ�����ʼ��NVIC�Ĵ���
	
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM6, ENABLE);
	
//	RCC->APB1ENR |= 1 << 4;					//TIM6ʱ��ʹ��    
// 	TIM6->ARR = arr;  						//�趨�������Զ���װֵ 
//	TIM6->PSC = psc;  			 			//����Ԥ��Ƶ��.
// 	TIM6->DIER |= 1 << 0;   				//��������ж�				
// 	TIM6->CR1 |= 0x01;    					//ʹ�ܶ�ʱ��6
//	MY_NVIC_Init(0, 0, TIM6_IRQn, 2);		//��ռ1�������ȼ�2����2		
}




/*************************analog output*****************************/

void TIM_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	


	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //TIM3_CH1->PC6, TIM3_CH2->PC7, TIM3_CH3->PC8, TIM3_CH4->PC9	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_12;  //TIM3_CH1-4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = 4096;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;


	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
    
/*****************************************************************/	

	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
	TIM_Cmd(TIM4, ENABLE);
}

void set_output (uint8_t channel, uint16_t duty)
{
	// calculate the compare value and then set it to change the pwm duty
 //	duty = test[0];		  
	switch(channel)
	{ 
		
		case CHANNEL_HUM: 
//			duty = 3000;
			TIM_SetCompare1(TIM4, duty);
			break;
		case CHANNEL_TEMP: 
//			duty = 2000;
			TIM_SetCompare2(TIM4, duty);
			break;
		case CHANNEL_CO2:
//			duty = 1000;
			TIM_SetCompare3(TIM4, duty);
			break; 
	}
}
