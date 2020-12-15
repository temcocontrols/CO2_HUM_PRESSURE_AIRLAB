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
} 	;//时钟结构体 
//_calendar_obj calendar_ghost;					//日历结构体
u8 calendar_ghost_enable = 0;
u8 Rtc_Sec_It;
//月份数据表											 
u8 const table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};	//月修正数据表	  
//平年的月份日期表
const u8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

extern void watchdog(void);
/*
void set_clock(u16 divx)
{
 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
	PWR_BackupAccessCmd(ENABLE);	//使能RTC和后备寄存器访问 

	RTC_EnterConfigMode();/// 允许配置	
 
	RTC_SetPrescaler(divx); //设置RTC预分频的值          									 
	RTC_ExitConfigMode();//退出配置模式  				   		 									  
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成		 									  
}	   
*/
/*
 * 函数名：NVIC_Configuration
 * 描述  ：配置RTC秒中断的主中断优先级为1，次优先级为0
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */ 
static void RTC_NVIC_Config(void)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;				//RTC全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;	//先占优先级1位,从优先级3位
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;			//先占优先级0位,从优先级4位
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//使能该通道中断
	NVIC_Init(&NVIC_InitStructure);								//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
}
//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
static u8 Is_Leap_Year(u16 year)
{			  
	if(year % 4 == 0)				//必须能被4整除
	{ 
		if(year % 100 == 0) 
		{ 
			if(year % 400 == 0)		//如果以00结尾,还要能被400整除
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


//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号																						 
static u8 RTC_Get_Week(u16 year, u8 month, u8 day)
{	
	u16 temp2;
	u8 yearH, yearL;
	
	yearH = year / 100;
	yearL = year % 100;
	  
	if(yearH > 19)	// 如果为21世纪,年份数加100
		yearL += 100;
	
	// 所过闰年数只算1900年之后的  
	temp2 = yearL + yearL / 4;
	temp2 = temp2 % 7; 
	temp2 = temp2 + day + table_week[month - 1];
	
	if(yearL % 4 == 0 && month < 3)
		temp2--;
	
	return(temp2 % 7);
}	
//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.
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
	
	for(t = 1970; t < syear; t++)				//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))
			seccount += 31622400;				//闰年的秒钟数
		else 
			seccount += 31536000;				//平年的秒钟数
	}
	
	smon -= 1;
	for(t = 0; t < smon; t++)					//把前面月份的秒钟数相加
	{
		seccount += (u32)mon_table[t] * 86400;	//月份秒钟数相加
		if(Is_Leap_Year(syear) && t == 1)
			seccount += 86400;					//闰年2月份增加一天的秒钟数	   
	} 
	
		
	seccount += (u32)(sday - 1) * 86400;		//把前面日期的秒钟数相加 
	seccount += (u32)hour * 3600;					//小时秒钟数
    seccount += (u32)min * 60;					//分钟秒钟数
	seccount += sec;							//最后的秒钟加上去

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//使能PWR和BKP外设时钟  
	PWR_BackupAccessCmd(ENABLE);	//使能RTC和后备寄存器访问 
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	RTC_SetCounter(seccount);		//设置RTC计数器的值

	RTC_WaitForLastTask();			//等待最近一次对RTC寄存器的写操作完成  	
	return 0;	    
}


//得到当前的时间
//返回值:0,成功;其他:错误代码.
static u8 RTC_Get(void)
{
	static u16 daycnt = 0;
	u32 timecount = 0; 
	u32 temp = 0;
	u16 temp1 = 0;
	
    timecount = RTC_GetCounter();	 
 	temp = timecount / 86400;		//得到天数(秒钟数对应的)
	if(daycnt != temp)				//超过一天了
	{	  
		daycnt = temp;
		temp1 = 1970;				//从1970年开始
		while(temp >= 365)
		{				 
			if(Is_Leap_Year(temp1))	//是闰年
			{
				if(temp >= 366)
				{
					temp -= 366;	//闰年的秒钟数
				}
				else 
				{
					temp1++;
					break;
				}  
			}
			else
			{
				temp -= 365;		//平年
			}				
			temp1++;  
		}   
		calendar.w_year = temp1;	//得到年份
		
		temp1 = 0;
		while(temp >= 28)			//超过了一个月
		{
			if(Is_Leap_Year(calendar.w_year) && temp1 == 1)	//当年是不是闰年/2月份
			{
				if(temp >= 29)
					temp -=	29;		//闰年的秒钟数
				else
					break; 
			}
			else 
			{
				if(temp >= mon_table[temp1])
					temp -= mon_table[temp1];	//平年
				else
					break;
			}
			temp1++;  
		}
		calendar.w_month = temp1 + 1;	//得到月份
		calendar.w_date = temp + 1;  	//得到日期 
	}
	temp = timecount % 86400;     		//得到秒钟数   	   
	calendar.hour = temp / 3600;     	//小时
	calendar.min = (temp % 3600) / 60; 	//分钟	
	calendar.sec = (temp % 3600) % 60; 	//秒钟
	calendar.week = RTC_Get_Week(calendar.w_year, calendar.w_month, calendar.w_date);	//获取星期   
	return 0;
}
/*
 * 函数名：RTC_Configuration
 * 描述  ：配置RTC
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
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
		if(temp >= 300)return 1;	//初始化时钟失败,晶振有问题
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
		  

//实时时钟配置
//初始化RTC时钟,同时检测时钟是否工作正常
//BKP->DR1用于保存是否第一次配置的设置
//返回0:正常
//其他:错误代码

u8 RTC_Init(void)
{
	//检查是不是第一次配置时钟
	u8 i = 0;
	uint8 rtc_state = 0;
	RTC_NVIC_Config();							//RCT中断分组设置	
	print( "\r\n This is a RTC demo...... \r\n" );
	
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
	
//	RTC_Get();									//更新时间	
	return 0;
}
/*
 * 函数名：Time_Adjust
 * 描述  ：时间调节
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */
u8 Time_Adjust(void)
{  
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   
//		/* Allow access to BKP Domain */
//	PWR_BackupAccessCmd(ENABLE);												//使能后备寄存器访问 
	
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
//RTC时钟中断
//每秒触发一次  
extern u16 tcnt; 
void RTC_IRQHandler(void)
{		 
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)	//秒钟中断
	{							
		RTC_Get();//更新时间  
		if(Rtc_Sec_It == 0)
			Rtc_Sec_It = 1;
		else
			Rtc_Sec_It = 0;
 	}
	
	if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)		//闹钟中断
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);		//清闹钟中断	  	   
  	}
	
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);//清闹钟中断
	RTC_WaitForLastTask();	
	
	get_time_text();
}


 
