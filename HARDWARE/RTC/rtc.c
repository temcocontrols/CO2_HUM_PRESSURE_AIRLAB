#include "delay.h"
#include "usart.h"
#include "rtc.h" 		    
#include "LCD.h"
#include "menu.h"	   
_calendar_obj calendar =  
{
	11,		//vu8 hour;
	16,		//vu8 min;
	0,		//vu8 sec;		 
	2016,	//vu16 w_year;
	6,		//vu8 w_month;
	21,		//vu8 w_date;
	2,		//vu8 week;		 
} 	;//ʱ�ӽṹ�� 
//_calendar_obj calendar_ghost;					//�����ṹ��
u8 calendar_ghost_enable = 0;
u8 Rtc_Sec_It;
//�·����ݱ�											 
u8 const table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};	//���������ݱ�	  
//ƽ����·����ڱ�
const u8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

extern void watchdog(void);
/*
void set_clock(u16 divx)
{
 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��  
	PWR_BackupAccessCmd(ENABLE);	//ʹ��RTC�ͺ󱸼Ĵ������� 

	RTC_EnterConfigMode();/// ��������	
 
	RTC_SetPrescaler(divx); //����RTCԤ��Ƶ��ֵ          									 
	RTC_ExitConfigMode();//�˳�����ģʽ  				   		 									  
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������		 									  
}	   
*/
/*
 * ��������NVIC_Configuration
 * ����  ������RTC���жϵ����ж����ȼ�Ϊ1�������ȼ�Ϊ0
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */ 
static void RTC_NVIC_Config(void)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;				//RTCȫ���ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;	//��ռ���ȼ�1λ,�����ȼ�3λ
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;			//��ռ���ȼ�0λ,�����ȼ�4λ
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ʹ�ܸ�ͨ���ж�
	NVIC_Init(&NVIC_InitStructure);								//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}
//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
static u8 Is_Leap_Year(u16 year)
{			  
	if(year % 4 == 0)				//�����ܱ�4����
	{ 
		if(year % 100 == 0) 
		{ 
			if(year % 400 == 0)		//�����00��β,��Ҫ�ܱ�400����
				return 1; 	   
			else
				return 0;   
		}
		else
		{
			return 1;   
		}
	}
	else 
	{
		return 0;
	}		
}


//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//������������������� 
//����ֵ�����ں�																						 
static u8 RTC_Get_Week(u16 year, u8 month, u8 day)
{	
	u16 temp2;
	u8 yearH, yearL;
	
	yearH = year / 100;
	yearL = year % 100;
	  
	if(yearH > 19)	// ���Ϊ21����,�������100
		yearL += 100;
	
	// ����������ֻ��1900��֮���  
	temp2 = yearL + yearL / 4;
	temp2 = temp2 % 7; 
	temp2 = temp2 + day + table_week[month - 1];
	
	if(yearL % 4 == 0 && month < 3)
		temp2--;
	
	return(temp2 % 7);
}	
//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
static u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;
	u32 seccount = 0;
	
	if(syear < 1970 || syear > 2099)return 1;			//year range
	if((smon > 12)||(smon < 1)) 	return 1;			//month reange
	if(sday > mon_table[smon - 1]) 	sday = mon_table[smon - 1];		//day range
	if(((Is_Leap_Year(t)&&(smon - 1)) == 1) &&(sday > 29)) sday = 29;
	if(sday < 1)  	return 1;										
	if(hour >= 24) 	return 1;							//hour range
	if(min >= 60)  	return 1;							//min range
	if(sec >= 60)  	return 1;							//second range
	
	for(t = 1970; t < syear; t++)				//��������ݵ��������
	{
		if(Is_Leap_Year(t))
			seccount += 31622400;				//�����������
		else 
			seccount += 31536000;				//ƽ���������
	}
	
	smon -= 1;
	for(t = 0; t < smon; t++)					//��ǰ���·ݵ����������
	{
		seccount += (u32)mon_table[t] * 86400;	//�·����������
		if(Is_Leap_Year(syear) && t == 1)
			seccount += 86400;					//����2�·�����һ���������	   
	} 
	
		
	seccount += (u32)(sday - 1) * 86400;		//��ǰ�����ڵ���������� 
	seccount += (u32)hour * 3600;					//Сʱ������
    seccount += (u32)min * 60;					//����������
	seccount += sec;							//�������Ӽ���ȥ

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//ʹ��PWR��BKP����ʱ��  
	PWR_BackupAccessCmd(ENABLE);	//ʹ��RTC�ͺ󱸼Ĵ������� 
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	RTC_SetCounter(seccount);		//����RTC��������ֵ

	RTC_WaitForLastTask();			//�ȴ����һ�ζ�RTC�Ĵ�����д�������  	
	return 0;	    
}


//�õ���ǰ��ʱ��
//����ֵ:0,�ɹ�;����:�������.
static u8 RTC_Get(void)
{
	static u16 daycnt = 0;
	u32 timecount = 0; 
	u32 temp = 0;
	u16 temp1 = 0;
	
    timecount = RTC_GetCounter();	 
 	temp = timecount / 86400;		//�õ�����(��������Ӧ��)
	if(daycnt != temp)				//����һ����
	{	  
		daycnt = temp;
		temp1 = 1970;				//��1970�꿪ʼ
		while(temp >= 365)
		{				 
			if(Is_Leap_Year(temp1))	//������
			{
				if(temp >= 366)
				{
					temp -= 366;	//�����������
				}
				else 
				{
					temp1++;
					break;
				}  
			}
			else
			{
				temp -= 365;		//ƽ��
			}				
			temp1++;  
		}   
		calendar.w_year = temp1;	//�õ����
		
		temp1 = 0;
		while(temp >= 28)			//������һ����
		{
			if(Is_Leap_Year(calendar.w_year) && temp1 == 1)	//�����ǲ�������/2�·�
			{
				if(temp >= 29)
					temp -=	29;		//�����������
				else
					break; 
			}
			else 
			{
				if(temp >= mon_table[temp1])
					temp -= mon_table[temp1];	//ƽ��
				else
					break;
			}
			temp1++;  
		}
		calendar.w_month = temp1 + 1;	//�õ��·�
		calendar.w_date = temp + 1;  	//�õ����� 
	}
	temp = timecount % 86400;     		//�õ�������   	   
	calendar.hour = temp / 3600;     	//Сʱ
	calendar.min = (temp % 3600) / 60; 	//����	
	calendar.sec = (temp % 3600) % 60; 	//����
	calendar.week = RTC_Get_Week(calendar.w_year, calendar.w_month, calendar.w_date);	//��ȡ����   
	return 0;
}
/*
 * ��������RTC_Configuration
 * ����  ������RTC
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */
u8 RTC_Configuration(void)
{
	u16 temp = 0;
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);
	
	/* Reset Backup Domain */
	BKP_DeInit();
	
	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	print("\r\n Wait till LSE is ready");
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
		//watchdog();
		temp++;
		delay_ms(10);
		if(temp >= 300)return 1;	//��ʼ��ʱ��ʧ��,����������
	}
	
	print("\r\n LSE is ready");
	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();
	
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
	
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	return 0;
}
		  

//ʵʱʱ������
//��ʼ��RTCʱ��,ͬʱ���ʱ���Ƿ�������
//BKP->DR1���ڱ����Ƿ��һ�����õ�����
//����0:����
//����:�������

u8 RTC_Init(void)
{
	//����ǲ��ǵ�һ������ʱ��
	u8 i = 0;
	uint8 rtc_state = 0;
	RTC_NVIC_Config();							//RCT�жϷ�������	
	//print( "\r\n This is a RTC demo...... \r\n" );
	
	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		/* Backup data register value is not correct or not yet programmed (when
		the first time the program is executed) */ 
		print("\r\n\n RTC not yet configured....");
		
		/* RTC Configuration */
		if(RTC_Configuration()== 1)
			return 1;
		
		print("\r\n RTC configured....");
		
		/* Adjust time by values entred by the user on the hyperterminal */
		Time_Adjust();
		
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	else
	{
//		/* Check if the Power On Reset flag is set */
//		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
//		{
//			print("\r\n\n Power On Reset occurred....");
//		}
//		/* Check if the Pin Reset flag is set */
//		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
//		{
//			print("\r\n\n External Reset occurred....");
//		}
		
		print("\r\n No need to configure RTC....");
		/* Wait for RTC registers synchronization */
		if(RTC_WaitForSynchro() == 1)
			return 1;
		
		/* Enable the RTC Second */
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		
		/* Wait until last write operation on RTC registers has finished */
		if(RTC_WaitForLastTask())
			return 1;
	}
	/* Clear reset flags */
	RCC_ClearFlag();
	
//	RTC_Get();									//����ʱ��	
	return 0;
}
/*
 * ��������Time_Adjust
 * ����  ��ʱ�����
 * ����  ����
 * ���  ����
 * ����  ���ⲿ����
 */
u8 Time_Adjust(void)
{  
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��   
//		/* Allow access to BKP Domain */
//	PWR_BackupAccessCmd(ENABLE);												//ʹ�ܺ󱸼Ĵ������� 
	
	//if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) ||( PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_PM25))
	if(isColorScreen == true)
		return RTC_Set(calendar.w_year,	calendar.w_month,	calendar.w_date,\
					calendar.hour,	calendar.min,		calendar.sec);
		else
		{
			if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
				return RTC_Set(calendar.w_year,	calendar.w_month,	calendar.w_date,\
					calendar.hour,	calendar.min,		calendar.sec);
			else
				return 1; 
		}
//		#if 1//defined (COLOR_SCREEN)
//	else if( PRODUCT_ID == STM32_HUM_NET)
//	{
//		if(isColorScreen == true)
//			return RTC_Set(calendar.w_year,	calendar.w_month,	calendar.w_date,\
//					calendar.hour,	calendar.min,		calendar.sec);
//		}
//		#endif

//	RCC_ClearFlag();
	
}
//RTCʱ���ж�
//ÿ�봥��һ��  
extern u16 tcnt; 
void RTC_IRQHandler(void)
{		 
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)	//�����ж�
	{		
		RTC_Get();//����ʱ��  
		if(Rtc_Sec_It == 0)
			Rtc_Sec_It = 1;
		else
			Rtc_Sec_It = 0;
 	}
	
	if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)		//�����ж�
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);		//�������ж�	  	   
  }
	
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);//�������ж�
	RTC_WaitForLastTask();	
	
	get_time_text();
}


 
