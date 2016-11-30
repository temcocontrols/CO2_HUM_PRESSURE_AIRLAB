
#include "define.h"
#include "inputs.h"

#include "delay.h"
#include "modbus.h"
#include "../filter/filter.h"
#include "controls.h"
//#if (defined T322AI) || (T38AI8AO6DO)	
//uint16_t data_change[MAX_AI_CHANNEL] = {0} ;
//#endif

 
 
 
#define ADC_DR_ADDRESS  0x4001244C  

//vu16 AD_Value[MAX_AI_CHANNEL] = {0}; 
 
 

void inputs_io_init(void)
{
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_0|ADC_Channel_1, 1, ADC_SampleTime_28Cycles5);
// IO Configure 
	
GPIO_InitTypeDef GPIO_InitStructure;
#ifdef T322AI	
/**************************PortA configure---ADC1*****************************************/
////RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA, ENABLE);
//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;  
//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
//GPIO_Init(GPIOA, &GPIO_InitStructure);

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;  
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	
GPIO_Init(GPIOA, &GPIO_InitStructure);

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;  
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	
GPIO_Init(GPIOC, &GPIO_InitStructure);	
///**************************PortB configure----ADC1*****************************************/
//RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOB, ENABLE);
//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;  
//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
//GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

#ifdef T38AI8AO6DO
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7;  
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	
GPIO_Init(GPIOA, &GPIO_InitStructure);

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;  
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
GPIO_Init(GPIOC, &GPIO_InitStructure);


#endif
/**************************PortC configure----ADC1*****************************************/
RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOC, ENABLE);
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
GPIO_Init(GPIOC, &GPIO_InitStructure);




	
}
void inputs_adc_init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	/* configuration ------------------------------------------------------*/  
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//
//	ADC_InitStructure.ADC_ScanConvMode = ENABLE;//DMA CONTINUS MODE
//	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//
//	#ifdef T322AI
	ADC_InitStructure.ADC_NbrOfChannel = 1;//
	ADC_Init(ADC1, &ADC_InitStructure);
//	ADC_Init(ADC2, &ADC_InitStructure);
	 /* ADC1 regular channels configuration */ 
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_4 , 1, ADC_SampleTime_239Cycles5);  
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_5 , 2, ADC_SampleTime_239Cycles5); 
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_6 , 3, ADC_SampleTime_239Cycles5);
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_7 , 4, ADC_SampleTime_239Cycles5);
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_8 , 5, ADC_SampleTime_239Cycles5);
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_9 , 6, ADC_SampleTime_239Cycles5);
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_10 , 7, ADC_SampleTime_239Cycles5);
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_11 , 8, ADC_SampleTime_239Cycles5);
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_12 , 9, ADC_SampleTime_239Cycles5);
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_13 , 10, ADC_SampleTime_239Cycles5);
////	ADC_RegularChannelConfig(ADC1, ADC_Channel_14 , 11, ADC_SampleTime_239Cycles5);
//	ADC_DMACmd(ADC1, ENABLE); //enable dma
//	ADC_Cmd(ADC2, ENABLE); 
//	ADC_ResetCalibration(ADC2);
//	while(ADC_GetResetCalibrationStatus(ADC2) == SET);
//	ADC_StartCalibration(ADC2);
//	while(ADC_GetCalibrationStatus(ADC2) == SET);
//	#endif
	ADC_Cmd(ADC1, ENABLE); 
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1)== SET)
	{
		;
	}		
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
     ADC_StartCalibration(ADC1);
     while(ADC_GetCalibrationStatus(ADC1) == SET);
}

//void dma_adc_init(void)
//{
///************DMA configure*******************************/
//DMA_InitTypeDef DMA_InitStructure;
//DMA_DeInit(DMA1_Channel1);
//DMA_InitStructure.DMA_PeripheralBaseAddr = ADC_DR_ADDRESS;
//DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value  ;
//DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//DMA_InitStructure.DMA_BufferSize = 11; 
//DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
//DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
//DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
//DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

//DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
//DMA_InitStructure.DMA_Priority = DMA_Priority_High;
//DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; 
//DMA_Init(DMA1_Channel1, &DMA_InitStructure); 
//DMA_Cmd(DMA1_Channel1, ENABLE); 
//}
void inputs_init(void) 
{
	 
	inputs_io_init();
	inputs_adc_init();
	//dma_adc_init();
	 
}


 
 
void inputs_scan(void)
{
	;	  
}
 
 
  
 
  