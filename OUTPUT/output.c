#include "modbus.h"
#include "output.h"
#include "define.h"
#include "controls.h"
 

void output_init(void)
{ 
//	
//	GPIO_InitTypeDef GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//	TIM_OCInitTypeDef TIM_OCInitStructure;
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //TIM3_CH1->PC6, TIM3_CH2->PC7, TIM3_CH3->PC8, TIM3_CH4->PC9	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;  //TIM3_CH1-4
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);

//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); //TIM3_CH1->PC6, TIM3_CH2->PC7, TIM3_CH3->PC8, TIM3_CH4->PC9	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;  //TIM3_CH1-4
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOD, &GPIO_InitStructure);

//	
//	
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2 , ENABLE);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11  ;  
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_3 ;  
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	
//	
//	TIM_TimeBaseStructure.TIM_Period = 1000;
//	TIM_TimeBaseStructure.TIM_Prescaler = 0;
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
//	
//	
//	TIM_TimeBaseStructure.TIM_Period = 1000;
//	TIM_TimeBaseStructure.TIM_Prescaler = 0;
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
//	
//	TIM_TimeBaseStructure.TIM_Period = 1000;
//	TIM_TimeBaseStructure.TIM_Prescaler = 0;
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
//	
//	TIM_TimeBaseStructure.TIM_Period = 1000;
//	TIM_TimeBaseStructure.TIM_Prescaler = 0;
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
//	
//	
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
//	
//	TIM_OC1Init(TIM5, &TIM_OCInitStructure);
//	TIM_OC2Init(TIM5, &TIM_OCInitStructure);
//	TIM_OC3Init(TIM5, &TIM_OCInitStructure);
//	TIM_OC4Init(TIM5, &TIM_OCInitStructure);
//	

//	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
//	TIM_OC4Init(TIM2, &TIM_OCInitStructure);
//	
//	/**************digit output*********************/	
//	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
//	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
//	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
//	TIM_OC4Init(TIM4, &TIM_OCInitStructure);
///*****************************************************************/	

//	
////	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
////	TIM_OC4Init(TIM4, &TIM_OCInitStructure);

////	TIM_OC1Init(TIM5, &TIM_OCInitStructure);
////	TIM_OC2Init(TIM5, &TIM_OCInitStructure);
////	TIM_OC3Init(TIM5, &TIM_OCInitStructure);
//	
//	TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable);
//	TIM_OC2PreloadConfig(TIM5, TIM_OCPreload_Enable);
//	TIM_OC3PreloadConfig(TIM5, TIM_OCPreload_Enable);
//	TIM_OC4PreloadConfig(TIM5, TIM_OCPreload_Enable);
//	
//	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
//	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
//	
//	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
//	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
//	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
//	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

//	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	
//	TIM_Cmd(TIM3, ENABLE);
//	TIM_Cmd(TIM2, ENABLE);
//	TIM_Cmd(TIM5, ENABLE);
//	TIM_Cmd(TIM4, ENABLE);
}
 

	
	


void output_refresh(void)
{
  ;
}

 




