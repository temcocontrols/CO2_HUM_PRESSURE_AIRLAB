#include "config.h"
#include "sht3x.h"
#include "temperature.h"
#include "check_AQ.h"

//int16 Test[20];
uint8 update_flag = 0;
unsigned char PRODUCT_ID;  
void vWifitask(void *pvParameters);
//static void vFlashTask( void *pvParameters );  
static void vCOMMTask( void *pvParameters );

xTaskHandle task_handle[20];
//static void vUSBTask( void *pvParameters );
void AQ_Alarm_task(void *pvParameters );
static void vNETTask( void *pvParameters );
//void SHT4x_Task( void *pvParameters );
//void SCD40_Task( void *pvParameters );
uint8 isWagnerProduct;
 void vLCDtask(void *pvParameters);
static void vMSTP_TASK(void *pvParameters );
void uip_polling(void);

static void watchdog_init(void);
void check_Task_locked(void);
STR_Task_Test task_test;

void watchdog(void);
#define	BUF	((struct uip_eth_hdr *)&uip_buf[0])	
	
u8 update = 0 ;
u8 read_cal = 0 ;

u32 Instance = 0x0c;
uint8_t  PDUBuffer[MAX_APDU];

extern bool isFirstLineChange ; 
extern bool isSecondLineChange ; 
extern bool isThirdLineChange ;

extern void get_data_format(u8 loc,float num,char *s);
extern uint8 light_sensor;
//u8 global_key = KEY_NON;
uint8_t sub_product;// 0- default 1 - only support temperature  2 - AQ
static void debug_config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}
U8_T MAX_MASTER;
U8_T data_backup_rev;
int main(void)
{
	uint8 rtc_state = 0, rtc_state1 = 1;
  uint16 j;	
 	uint8 i; 
	int16 offset = 0;
	
 	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8008000);
// 	debug_config(); 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
 	delay_init(72);	
	//modbus_init();
  watchdog_init();
	//printf("step1\r\n");
	qSubSerial = xQueueCreate(SUB_BUF_LEN, 1);
	xMutex = xQueueCreateMutex();
	IicMutex = xQueueCreateMutex();
	qKey = xQueueCreate(5, 2);
	watchdog();
	if(( qSubSerial == NULL )  ||( xMutex == NULL )||( IicMutex == NULL ))    
	{
		while(1) ;
	}
	
	sub_product = read_eeprom(EEP_SUB_PRODUCT);
	if(sub_product > 2)
	{ // 0- default 1 - only support temperature  2 - AQ
		sub_product = 0;
	}
	//printf("step2\r\n");
	if(read_eeprom(EEP_CLEAR_EEP) == 99)
	{
		for(j=66;j<2047;j++)
		{
			write_eeprom(j,0xff);
			watchdog();
		}
		write_eeprom(EEP_UPDATE_STATUS,0);
	}
	MAX_MASTER = read_eeprom(EEP_MAX_MASTER);
	if(MAX_MASTER == 255 || MAX_MASTER <= 1)
	{
		MAX_MASTER = 254;
		write_eeprom(EEP_MAX_MASTER,MAX_MASTER);
	}
	isColorScreen = read_eeprom(EEP_IS_COLOR_SCREEN);
	if(isColorScreen == false)
	{
		Lcd_Initial();
		Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)"Initialization...");
	}
	else if(isColorScreen == true)
	{
		scroll = &scroll_ram[0][0];
		LCDtest();
	}
	else
	{
		Lcd_Initial();
		Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)"Initialization...");
		write_eeprom(EEP_IS_COLOR_SCREEN ,0);
	}
	
	
	
	EEP_Dat_Init();
	
//------AQ CODE-------
	if(sub_product == 2)
	{
		init_AQ_calibration();
		for(i=0;i<3;i++)	   
			aq_level_value[i] = AT24CXX_ReadOneByte(EEP_AQI_FIRST_LINE_LO+i*2) + (AT24CXX_ReadOneByte(EEP_AQI_FIRST_LINE_LO+i*2+1)<<8);
		 
		if((aq_level_value[0]>1000)||(aq_level_value[1]>1000)||(aq_level_value[2]>1000))	
		{
			aq_level_value[0] = 200;
			aq_level_value[1] = 400;
			aq_level_value[2] = 600;
			for(i=0;i<3;i++)
			{
				write_eeprom( EEP_AQI_FIRST_LINE_LO +2*i, aq_level_value[i] );
				write_eeprom( EEP_AQI_FIRST_LINE_LO+2*i + 1, aq_level_value[i]>>8 );
			}														  
		}
		aq_level_value[3] = AT24CXX_ReadOneByte(EEP_MAX_AQ_VAL) + ((int)AT24CXX_ReadOneByte(EEP_MAX_AQ_VAL+1)<<8);
		if(aq_level_value[3] < 0)
			aq_level_value[3] = 1000;
		aq_calibration = AT24CXX_ReadOneByte(EEP_CALIBRATION_AQ) + AT24CXX_ReadOneByte(EEP_CALIBRATION_AQ +1) * 256;
		if (aq_calibration > 1000 || aq_calibration < 0)
		{
			aq_calibration = 500;
			write_eeprom(EEP_CALIBRATION_AQ, CALIBRATION_DEFAULT & 0xFF);
			write_eeprom(EEP_CALIBRATION_AQ+1, CALIBRATION_DEFAULT / 256);
		}
	}
//--------------------------end of AQ ini------------------	
	
	if(PRODUCT_ID == STM32_PM25)
	{
		if(isColorScreen == false)
		{
			offset |= read_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET + 1);
			offset = offset << 8;
			offset |= read_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET);
			if((offset > -100) || (offset < -300))
			{
				write_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET ,offset & 0xff);
				write_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET + 1, (offset >> 8)&0xff);
				output_offset[1][CHANNEL_TEMP] = -200;			
			}
		}
	}
	
//	for(i=0; i<255; i++)
//		AT24CXX_WriteOneByte(i, 0xff);
//	
//	EEP_Dat_Init();
	//if(isColorScreen == false)
		start_back_light(backlight_keep_seconds);
	
	
	//print("EEP Init Done!\r\n");
   	
 	mass_flash_init();
	memset(&task_test,0,sizeof(STR_Task_Test));
	
	//print("FLASH Init Done!\r\n");
//	beeper_gpio_init();
//	beeper_on();
//	delay_ms(1000);n
//	beeper_off();
	//Lcd_Initial();
	SPI1_Init();
//	print("SPI1 Init Done!\r\n");
	Lcd_Show_String(1, 0, DISP_NOR, (uint8 *)"EEP is Done");
//	SPI2_Init();
//	mem_init(SRAMIN);
//	TIM3_Int_Init(5000, 7199);
// TIMR6_INIT is called in ENC28J60_Reset() when product is XX_NET
	if(PRODUCT_ID == STM32_HUM_RS485 || PRODUCT_ID == STM32_CO2_RS485)
	{	// heartbeat LED 
		LED_Init();
		TIM6_Int_Init(100, 7199);
	}
   	
//	uart3_modbus_init();
	
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
	{
		//RTC_Init();
		co2_autocal_disable = read_eeprom(EEP_CO2_AUTOCAL_SW);
	}
	rtc_state = 1;
	i = 0;
	watchdog();
	if(isColorScreen == true)
	{
		//if( (PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_PM25))
		  while(rtc_state)
			{
				watchdog();
				if(i<20)
				{
					if(RTC_Init() == 0) //initial OK
					{
						rtc_state1 = 0;
						rtc_state = 0;
						break;
					}
					else
						i++;
				}
				else
				{
					rtc_state1 = 1;
					rtc_state = 0;
					break;
				}
				delay_ms(20);	
			}
//			RTC_Init();
	}
//	print("RTC Init Done!\r\n");
	if(rtc_state1 == 0)
		Lcd_Show_String(2, 0, DISP_NOR, (uint8 *)"RTC is Done");
	else
		Lcd_Show_String(2, 0, DISP_NOR, (uint8 *)"RTC is Failed");
	
	//watchdog_init();
	watchdog(); 
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PM25))
	{	
		while(tapdev_init())	//��ʼ��ENC28J60����
		{								   
	// 		print("ENC28J60 Init Error!\r\n");
			delay_ms(100);
		}
	}
// 	print("ENC28J60 Init Done!\r\n");
//	watchdog();  
	
	Lcd_Show_String(3, 0, DISP_NOR, (uint8 *)"Net is Done");
//	print("CO2_NET\n\r");
 	Lcd_Show_String(4, 0, DISP_NOR, (uint8 *)"Done");
	delay_ms(100);
	watchdog(); 
	
	Test[0] = read_eeprom(EEP_RESTART_NUM);

	Test[41] =  read_eeprom(EEP_HARDFAULT1);
	Test[42] =  read_eeprom(EEP_HARDFAULT2);
	Test[43] =  read_eeprom(EEP_HARDFAULT3);
	Test[44] =  read_eeprom(EEP_HARDFAULT4);
	Test[45] =  read_eeprom(EEP_HARDFAULT5);
	if(Test[0] == 0xff)
	{
		Test[0] = 1;
		AT24CXX_WriteOneByte(EEP_RESTART_NUM, Test[0]);
	}
	else
	{
		AT24CXX_WriteOneByte(EEP_RESTART_NUM, Test[0] + 1);
	}
	Lcd_Show_String(5, 0, DISP_NOR, (uint8 *)"Done");

	if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PM25))  
		xTaskCreate( vNETTask, ( signed portCHAR * ) "NET",  configMINIMAL_STACK_SIZE + 512, NULL, tskIDLE_PRIORITY + 1 , &task_handle[0] );
  
  xTaskCreate( vMSTP_TASK, ( signed portCHAR * ) "MSTP", configMINIMAL_STACK_SIZE + 512  , NULL, tskIDLE_PRIORITY + 6, &task_handle[1] );
 	xTaskCreate( vCOMMTask, ( signed portCHAR * ) "COMM", configMINIMAL_STACK_SIZE + 128, NULL, tskIDLE_PRIORITY + 7, &task_handle[2] );
//	TXEN = 0;
	if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
	{
		xTaskCreate( Co2_task,   ( signed portCHAR * ) "Co2Task", configMINIMAL_STACK_SIZE+50, NULL, tskIDLE_PRIORITY + 5, &task_handle[3]);		
		//xTaskCreate( Alarm_task,   ( signed portCHAR * ) "AlarmTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3,  &task_handle[4]);
	}
	if(PRODUCT_ID == STM32_HUM_RS485)
	{
		if(sub_product == 2)
		{
			xTaskCreate( AQ_Alarm_task,  ( signed portCHAR * ) "AQ_Alarm_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3,  &task_handle[4]);	
		}
	}

	if(PRODUCT_ID == STM32_PM25)
	{
		xTaskCreate( PM25_task, ( signed portCHAR * ) "PM25Task", configMINIMAL_STACK_SIZE+128, NULL, tskIDLE_PRIORITY + 6, &task_handle[5]);
	}
	watchdog(); 
	if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) )
		xTaskCreate(vUpdate_Pressure_Task, (signed portCHAR *)"Update_Pressure_Task", configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 4, &task_handle[6]);
	else //if(PRODUCT_ID != STM32_PM25)
	{
		xTaskCreate(vUpdate_Temperature_Task, (signed portCHAR *)"Update_Temperature_Task", configMINIMAL_STACK_SIZE + 512, NULL,  tskIDLE_PRIORITY + 3, &task_handle[7]);
	}
	xTaskCreate(vStartPIDTask, (signed portCHAR *)"vStartPIDTask", configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 3, &task_handle[8]);
 
// 	xTaskCreate( vFlashTask, ( signed portCHAR * ) "FLASH", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5, NULL );
  
  xTaskCreate( vOutPutTask	,( signed portCHAR * ) "OutPut" , configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &task_handle[9] );
 
  xTaskCreate( vKEYTask, ( signed portCHAR * ) "KEY", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &task_handle[10] );
	//#if defined (COLOR_SCREEN)
	if(isColorScreen == true)
		xTaskCreate( vLCDtask, ( signed portCHAR * ) "LCD", configMINIMAL_STACK_SIZE + 20, NULL, tskIDLE_PRIORITY + 5, &task_handle[11] );
	//#endif
#if WIFITEST
	if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PM25)) 
		xTaskCreate( vWifitask, ( signed portCHAR * ) "Wifitask", configMINIMAL_STACK_SIZE+1024, NULL, tskIDLE_PRIORITY + 5, &task_handle[12] );
#endif 
	vStartMenuTask(tskIDLE_PRIORITY + 2);
	Lcd_Show_String(6, 0, DISP_NOR, (uint8 *)"Done");
	//xTaskCreate( vGet_Hum_Para_Task, ( signed portCHAR * ) "Hum_Paratask", configMINIMAL_STACK_SIZE+512, NULL, tskIDLE_PRIORITY + 5, NULL );
	vTaskStartScheduler();
}

#if 1//defined (COLOR_SCREEN)
void vLCDtask(void *pvParameters)
{
	uint8 i;
	uint16 lastCO2 = 100;
	int16 lastTemp = -100, lastHumi = -100;
	static bool isLcdFirstTime = false;
	u8 temp = 0; 
	float ftemp;
	//static uint16 lcdCounter = 0;
	delay_ms(500);
	
	Lcd_Full_Screen(0);
	task_test.enable[11] = 1;
	for(;;)
	{task_test.count[11]++;Test[40] = 11;
		//lcdCounter++;
		//Lcd_Show_String(5, 0, DISP_NOR, (uint8 *)"Done");
		Lcd_Show_Data(5, 19, task_test.count[11], 1, ALIGN_RIGHT, DISP_NOR);
		if(update == 1)
		{	
			Lcd_Full_Screen(0);
			delay_ms(10);
			Lcd_Show_String(1, 6, 0, (uint8 *)"Updating...");
			Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
			SoftReset();
		}
		else if((update == 2) || (IP_Change == 1))
		{
			Lcd_Full_Screen(0);
			Lcd_Show_String(1, 6, 0, (uint8 *)"Restarting...");
			Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
			SoftReset();
		}
		
		if(CurrentState.Index == MenuIdle)
		{
			if(isLcdFirstTime)
			{
				isLcdFirstTime = false;
				Lcd_Full_Screen(0);
			}
			
			if(PRODUCT_ID == STM32_HUM_RS485 || PRODUCT_ID == STM32_HUM_NET)
			{
				if(light_sensor == 1)
				{
					screenArea3 = SCREEN_AREA_LIGHT;
					isThirdLineChange = 1;
				}
			}
			switch(screenArea1)
			{
				case SCREEN_AREA_TEMP:					
					if(isFirstLineChange)
					{
						for(i = 0; i<6; i++)
						{
							disp_ch(0,THERM_METER_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						}
						if((hum_exists== 3)||(hum_exists==2))
							disp_icon(14, 14, degree_o, 30+THERM_METER_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//						if(sub_product == 1)
//						{
//							if(deg_c_or_f == DEGREE_C)
//								Top_area_display(TOP_AREA_DISP_ITEM_LINE1, internal_temperature_c, TOP_AREA_DISP_UNIT_C);
//							else
//								Top_area_display(TOP_AREA_DISP_ITEM_LINE1, internal_temperature_f, TOP_AREA_DISP_UNIT_F);
//						}
//						else
						{
							if(deg_c_or_f == DEGREE_C)
								Top_area_display(TOP_AREA_DISP_ITEM_LINE1, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
							else
								Top_area_display(TOP_AREA_DISP_ITEM_LINE1, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
						}
						isFirstLineChange = false;
					}
					//if( lastTemp != HumSensor.temperature_c)
					{
						if((hum_exists== 3)||(hum_exists==2))
							disp_icon(14, 14, degree_o, 30+THERM_METER_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);

						if((output_auto_manual & 0x01) == 0x01)
						{
							if(deg_c_or_f == DEGREE_C)
								Top_area_display(TOP_AREA_DISP_ITEM_LINE1, output_manual_value_temp, TOP_AREA_DISP_UNIT_C);
							else
								Top_area_display(TOP_AREA_DISP_ITEM_LINE1, output_manual_value_temp, TOP_AREA_DISP_UNIT_F);
						}
						else
						{
							if(deg_c_or_f == DEGREE_C)
								Top_area_display(TOP_AREA_DISP_ITEM_LINE1, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
							else
								Top_area_display(TOP_AREA_DISP_ITEM_LINE1, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
						}
						if(Run_Timer>FIRST_TIME)
						{
							if(deg_c_or_f == DEGREE_C)
								lastTemp = HumSensor.temperature_c;
							else
								lastTemp = HumSensor.temperature_f;
						}
					}
					break;
				case SCREEN_AREA_HUMI:
					if(isFirstLineChange)
					{
						disp_ch(0,30+THERM_METER_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						for(i = 0; i<7; i++)
						{
							disp_ch(0,THERM_METER_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
							Top_area_display(TOP_AREA_DISP_ITEM_LINE1, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
						}
						isFirstLineChange = false;
					}
					//if(lastHumi != HumSensor.humidity)
					
					{						
						Top_area_display(TOP_AREA_DISP_ITEM_LINE1, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
						if(Run_Timer>FIRST_TIME)
							lastHumi = HumSensor.humidity;
					}
					break;
				case SCREEN_AREA_CO2:
					if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485)||(PRODUCT_ID == STM32_PM25))
					{
						if(isFirstLineChange)
						{
							disp_ch(0,30+THERM_METER_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
							for(i = 0; i<7; i++)
							{
								disp_ch(0,THERM_METER_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
							}
							Top_area_display(TOP_AREA_DISP_ITEM_LINE1, int_co2_str.co2_int, TOP_AREA_DISP_UNIT_C);
							//Top_area_display(TOP_AREA_DISP_ITEM_LINE1, pm25_sensor.AQI, TOP_AREA_DISP_UNIT_C);
							isFirstLineChange = false;
						}
						//if( var[CHANNEL_CO2].value != lastCO2)
						{
							Top_area_display(TOP_AREA_DISP_ITEM_LINE1, int_co2_str.co2_int, TOP_AREA_DISP_UNIT_C);
							//Top_area_display(TOP_AREA_DISP_ITEM_LINE1, pm25_sensor.AQI, TOP_AREA_DISP_UNIT_C);
							if(Run_Timer>FIRST_TIME)
								lastCO2 = int_co2_str.co2_int;
						}
					}					
					break;
				case SCREEN_AREA_PM25:
					if(PRODUCT_ID == STM32_PM25)
					{
						Top_area_display(TOP_AREA_DISP_ITEM_LINE1, pm25_weight_25, TOP_AREA_DISP_UNIT_C);
					}
					break;
				case SCREEN_AREA_AQI:
					if(PRODUCT_ID == STM32_PM25)
					{
						Top_area_display(TOP_AREA_DISP_ITEM_LINE1, pm25_sensor.AQI, TOP_AREA_DISP_UNIT_C);
					}
					break;
				case SCREEN_AREA_PRESSURE:
					Top_area_display(TOP_AREA_DISP_ITEM_LINE1, 0, TOP_AREA_DISP_UNIT_C);
					break;
				default:
					break;
			}
			
			switch(screenArea2)
			{
				case SCREEN_AREA_TEMP:
					if(isSecondLineChange)
					{
						for(i = 0; i<7; i++)
						{
							disp_ch(0,HUM_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						}
						disp_icon(14, 14, degree_o, 30+HUM_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
						if(deg_c_or_f == DEGREE_C)
							Top_area_display(TOP_AREA_DISP_ITEM_LINE2, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
						else
							Top_area_display(TOP_AREA_DISP_ITEM_LINE2, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
						isSecondLineChange = false;
					}
					//if( lastTemp != HumSensor.temperature_c)
					{
						
						disp_icon(14, 14, degree_o, 30+HUM_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
						
						if(sub_product == 1)
						{
							if(deg_c_or_f == DEGREE_C)
								Top_area_display(TOP_AREA_DISP_ITEM_LINE2, internal_temperature_c, TOP_AREA_DISP_UNIT_C);
							else
								Top_area_display(TOP_AREA_DISP_ITEM_LINE2, internal_temperature_f, TOP_AREA_DISP_UNIT_F);
						}
						else
						{
							if(deg_c_or_f == DEGREE_C)
								Top_area_display(TOP_AREA_DISP_ITEM_LINE2, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
							else
								Top_area_display(TOP_AREA_DISP_ITEM_LINE2, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
						}						
						
						if(Run_Timer>FIRST_TIME)
							lastTemp = HumSensor.temperature_c;
					}
					break;
				case SCREEN_AREA_HUMI:			
					if(isSecondLineChange)
					{
						disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						for(i = 0; i<7; i++)
						{
							disp_ch(0,HUM_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						}
						Top_area_display(TOP_AREA_DISP_ITEM_LINE2, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
						isSecondLineChange = false;
					}
					//if(lastHumi != HumSensor.humidity)
					if((output_auto_manual & 0x02))
					{
						Top_area_display(TOP_AREA_DISP_ITEM_LINE2, output_manual_value_humidity, TOP_AREA_DISP_UNIT_C);
					}
					else
					{
						//disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						
						Top_area_display(TOP_AREA_DISP_ITEM_LINE2, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
						if(Run_Timer>FIRST_TIME)
							lastHumi = HumSensor.humidity;
					}
					break;
				case SCREEN_AREA_CO2:			
						if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485)||(PRODUCT_ID == STM32_PM25) )
						{
							if(isSecondLineChange)
							{
								disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
								for(i = 0; i<7; i++)
								{
									disp_ch(0,HUM_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
								}
								Top_area_display(TOP_AREA_DISP_ITEM_LINE2, int_co2_str.co2_int, TOP_AREA_DISP_UNIT_C);
								//Top_area_display(TOP_AREA_DISP_ITEM_LINE2, pm25_weight_25, TOP_AREA_DISP_UNIT_C);
								isSecondLineChange = false;
							}
							//if( var[CHANNEL_CO2].value != lastCO2)
							{
								//disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
								
								Top_area_display(TOP_AREA_DISP_ITEM_LINE2, int_co2_str.co2_int, TOP_AREA_DISP_UNIT_C);
								//Top_area_display(TOP_AREA_DISP_ITEM_LINE2, pm25_weight_25, TOP_AREA_DISP_UNIT_C);
								if(Run_Timer > FIRST_TIME)
									lastCO2 = int_co2_str.co2_int;
									//lastCO2 = pm25_weight_25;
							}	
						}						
						break;
					case SCREEN_AREA_PM25:				
						if(PRODUCT_ID == STM32_PM25)
							Top_area_display(TOP_AREA_DISP_ITEM_LINE2, pm25_weight_25, TOP_AREA_DISP_UNIT_C);
						break;
					case SCREEN_AREA_PM10:				
						if(PRODUCT_ID == STM32_PM25)
							Top_area_display(TOP_AREA_DISP_ITEM_LINE2, pm25_weight_100, TOP_AREA_DISP_UNIT_C);
						break;
					case SCREEN_AREA_PRESSURE:			
						if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
						{
								if(Pressure.default_unit == inWC)  			//when the default unit is inwc, it has two decimals		 
								{	
									temp = decimal_num[0][Pressure.unit];
									ftemp  = Pressure.val_temp / 100;
								}
								else if(Pressure.default_unit == Psi)  		//when the default unit is psi, it has one decimals	 
								{
									temp = decimal_num[1][Pressure.unit];
									ftemp = Pressure.val_temp / 10;
								}  
								get_data_format(temp,ftemp,(char *)text);
								Top_area_display(TOP_AREA_DISP_ITEM_LINE2, 0, TOP_AREA_DISP_UNIT_C);
						}
						break;
					default:
						break;
					}

			switch(screenArea3)
			{
				case SCREEN_AREA_TEMP:
					if(isThirdLineChange)
					{
						for(i=0;i<12;i++)
						{
							disp_ch(FORM15X30, 33+CO2_POS,0+20*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						}
						if(deg_c_or_f == DEGREE_C)
							Top_area_display(TOP_AREA_DISP_ITEM_LINE3, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
						else
							Top_area_display(TOP_AREA_DISP_ITEM_LINE3, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
						isThirdLineChange = false;
					}
					//disp_icon(14, 14, degree_o, 30+HUM_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
					//if( lastTemp != HumSensor.temperature_c)
					{
						
						if(deg_c_or_f == DEGREE_C)
							Top_area_display(TOP_AREA_DISP_ITEM_LINE3, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
						else
							Top_area_display(TOP_AREA_DISP_ITEM_LINE3, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
						if(Run_Timer>FIRST_TIME)
							lastTemp = HumSensor.temperature_c;
					}
					break;
				case SCREEN_AREA_HUMI:
					if(isThirdLineChange)
					{
						for(i=0;i<12;i++)
						{
							disp_ch(FORM15X30, 33+CO2_POS,0+20*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						}
						Top_area_display(TOP_AREA_DISP_ITEM_LINE3, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
						isThirdLineChange = false;
					}
					//disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR
					//if(lastHumi != HumSensor.humidity)
					{
						
						Top_area_display(TOP_AREA_DISP_ITEM_LINE3, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
						if(Run_Timer>FIRST_TIME)
							lastHumi = HumSensor.humidity;
					}
					break;
				case SCREEN_AREA_CO2:
					if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
					{
						if(isThirdLineChange)
						{
							for(i=0;i<12;i++)
							{
								disp_ch(FORM15X30, 33+CO2_POS,0+20*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
							}
							if(!enableScroll)
								disp_icon(55, 55, co2Icon, 170, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
							Top_area_display(TOP_AREA_DISP_ITEM_LINE3, int_co2_str.co2_int, TOP_AREA_DISP_UNIT_C);
							isThirdLineChange = false;
						}
						//disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						//if( var[CHANNEL_CO2].value != lastCO2)
						{
							if(!enableScroll)
								disp_icon(55, 55, co2Icon, 170, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
							Top_area_display(TOP_AREA_DISP_ITEM_LINE3, int_co2_str.co2_int, TOP_AREA_DISP_UNIT_C);
							if(Run_Timer>FIRST_TIME)
								lastCO2 = int_co2_str.co2_int;
						}
					}
					break;
				case SCREEN_AREA_PRESSURE:
					Top_area_display(TOP_AREA_DISP_ITEM_LINE3, 0, TOP_AREA_DISP_UNIT_C);
					break;				
				case SCREEN_AREA_PM10:
					Top_area_display(TOP_AREA_DISP_ITEM_LINE3, pm25_weight_100, TOP_AREA_DISP_UNIT_C);
					break;
				case SCREEN_AREA_LIGHT:
					//if(isThirdLineChange)
					{						
//						for(i=0;i<12;i++)
//						{
//							disp_ch(FORM15X30, 33+CO2_POS,0+20*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
//						}
//						if(!enableScroll)
						disp_icon(55, 55, LightIcon, 170, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
						Top_area_display(TOP_AREA_DISP_ITEM_LINE3, light.val, TOP_AREA_DISP_UNIT_C);						
						isThirdLineChange = false;
					}

					break;
					// AQ
					case SCREEN_AREA_AQI:
					if(sub_product == 2)
					{
						Top_area_display(TOP_AREA_DISP_ITEM_LINE3, aq_value, TOP_AREA_DISP_UNIT_C);
					}
					break;
				default:
					break;
			}				


			if(enableScroll)
				display_scroll();
		}
		else
		{
			isLcdFirstTime = true;
			lastCO2 = 0;
			lastTemp = -100;
			lastHumi = -1;
		}

		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
#endif
void check_TXEN(void);
 	// if low-priority task is locked, need reboot
		// check whether task is locked
//uint16 low_pri;
//uint16 low_pri_backup;
//uint16  locked_count;
//void check_task_locked(void)
//{
//	if(low_pri != low_pri_backup)
//	{
//		low_pri_backup = low_pri;
//		locked_count = 0;
//	}
//	else
//		locked_count++;
//	
//	if(locked_count > 10000)
//	{
//		locked_count = 0;
//		AT24CXX_WriteOneByte(EEP_HARDFAULT3, Test[43]++);
//		SoftReset();
//	}
//}
void save_wifi(void);
void vCOMMTask(void *pvParameters )
{
	uint16_t locked_count = 0;
	modbus_init(); 
	reply_done =receive_delay_time;
//	print("COMM Task\r\n");
	delay_ms(100);
	
	task_test.enable[2] = 1;
	for( ;; )
	{		
//		watchdog(); 
		{// TEST stack lenght;
			char i;
			for(i = 0;i < 16;i++)
			{
			if(task_test.enable[i] == 1)
				Test[i + 1] = uxTaskGetStackHighWaterMark( task_handle[i] );
		}
		}
		
		save_wifi();
		
		task_test.count[2]++; Test[40] = 2;
		if(dealwithTag)
		{  Test[40] = 30;
		  dealwithTag--;
		  if(dealwithTag == 1)//&& !Serial_Master )	
			{Test[40] = 31;
				dealwithData();
				Test[40] = 32;
			}
		}
		check_TXEN();
		// net product
//		check_task_locked();
		Test[40] = 33;
		if(serial_receive_timeout_count > 0)  
		{
			serial_receive_timeout_count--; 
			if(serial_receive_timeout_count == 0)
			{
				serial_restart();
			}
		}Test[40] = 34;
		if(update_flag == 1) //protocol change to modbus 
		{
			update_flag = 0;	
		  serial_restart();			
		}
		else if(update_flag == 2)//protocol change to modbus
		{
			update_flag = 0;	
			Inital_Bacnet_Server();
			Recievebuf_Initialize(0);
		}
//		stack_detect(&test[2]);
		vTaskDelay(10 / portTICK_RATE_MS);
		if(locked_count++ % 200 == 0)
		{
			Test[32]++;
			check_Task_locked();
		}
		Test[33]++;
	}
	
}

 


 void Flash_task(void)
 {
	 uint8 i;Test[40] = 22;
	 Flash_Write_Mass(); 
	Test[40] = 23;
	poll_main_net_status();
	if(modbus.write_ghost_system == 1)
	{Test[40] = 24;
		modbus.ip_mode = modbus.ghost_ip_mode ;
		modbus.tcp_server = modbus.ghost_tcp_server ;
		modbus.listen_port = modbus.ghost_listen_port ;
		AT24CXX_WriteOneByte(EEP_IP_MODE, modbus.ip_mode);
		AT24CXX_WriteOneByte(EEP_TCP_SERVER, modbus.tcp_server);				
		AT24CXX_WriteOneByte(EEP_LISTEN_PORT_HI, modbus.listen_port>>8);
		AT24CXX_WriteOneByte(EEP_LISTEN_PORT_LO, modbus.listen_port &0xff);
		for(i=0; i<4; i++)
		{
			modbus.ip_addr[i] = modbus.ghost_ip_addr[i] ;
			modbus.mask_addr[i] = modbus.ghost_mask_addr[i] ;
			modbus.gate_addr[i] = modbus.ghost_gate_addr[i] ;
			
			AT24CXX_WriteOneByte(EEP_IP_ADDRESS_1+i, modbus.ip_addr[i]);
			AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_1+i, modbus.mask_addr[i]);
			AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_1+i, modbus.gate_addr[i]);						
		}

		IP_Change = 1; 
		modbus.write_ghost_system = 0 ;
 }
	Test[40] = 25;
}
 
 /*
void vFlashTask( void *pvParameters )
{ 
	uint8 i;
	modbus.write_ghost_system = 0;
//	print("Flash Task\r\n");
	delay_ms(100);
	
	for( ;; )
	{ 
		Flash_Write_Mass(); 
		poll_main_net_status();
		if(modbus.write_ghost_system == 1)
		{
			modbus.ip_mode = modbus.ghost_ip_mode ;
			modbus.tcp_server = modbus.ghost_tcp_server ;
			modbus.listen_port = modbus.ghost_listen_port ;
			AT24CXX_WriteOneByte(EEP_IP_MODE, modbus.ip_mode);
			AT24CXX_WriteOneByte(EEP_TCP_SERVER, modbus.tcp_server);				
			AT24CXX_WriteOneByte(EEP_LISTEN_PORT_HI, modbus.listen_port>>8);
			AT24CXX_WriteOneByte(EEP_LISTEN_PORT_LO, modbus.listen_port &0xff);
			for(i=0; i<4; i++)
			{
				modbus.ip_addr[i] = modbus.ghost_ip_addr[i] ;
				modbus.mask_addr[i] = modbus.ghost_mask_addr[i] ;
				modbus.gate_addr[i] = modbus.ghost_gate_addr[i] ;
				
				AT24CXX_WriteOneByte(EEP_IP_ADDRESS_1+i, modbus.ip_addr[i]);
				AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_1+i, modbus.mask_addr[i]);
				AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_1+i, modbus.gate_addr[i]);						
			}

			IP_Change = 1; 
			modbus.write_ghost_system = 0 ;
		}

		vTaskDelay(1000 / portTICK_RATE_MS);
	}	
}
*/  










void Inital_Bacnet_Server(void)
{
	uint32 ltemp; 
	Set_Object_Name((char *)panelname); 
	Device_Init();
	
	ltemp = 0;
	ltemp |= AT24CXX_ReadOneByte(EEP_INSTANCE_LOWORD);
	ltemp |= (uint32)AT24CXX_ReadOneByte(EEP_INSTANCE_LOWORD+1)<<8;
	ltemp |= (uint32)AT24CXX_ReadOneByte(EEP_INSTANCE_LOWORD+2)<<16;
	ltemp |= (uint32)AT24CXX_ReadOneByte(EEP_INSTANCE_LOWORD+3)<<24;
	if((ltemp == 0xffffffff)||(ltemp == 0))
		Instance = ((uint32)modbus.serial_Num[3]<<24)|((uint32)modbus.serial_Num[2]<<16)|((uint32)modbus.serial_Num[1]<<8) | modbus.serial_Num[0];
	else
		Instance = ltemp;
	
	Station_NUM = modbus.address;//AT24CXX_ReadOneByte(EEP_STATION_NUMBER);
	Device_Set_Object_Instance_Number(Instance);  
	address_init();
	bip_set_broadcast_addr(0xffffffff); 
 
	if(PRODUCT_ID == STM32_PM25)
	{
		AIS = 5; // Sprng change from 2 to 5 
		AOS = 2;
		BOS = 0;
	}
	else if(PRODUCT_ID == STM32_HUM_NET || PRODUCT_ID == STM32_HUM_RS485)
	{
		AIS = 3; // temp humdity Light
		AOS = 2;
		BOS = 0;
	}
	else if(PRODUCT_ID == STM32_CO2_NET || PRODUCT_ID == STM32_CO2_RS485)
	{
		AIS = 3;
		AOS = 3;
		BOS = 0;
	}
	else if(PRODUCT_ID == STM32_PRESSURE_NET || PRODUCT_ID == STM32_PRESSURE_RS485)
	{
		AIS = 1;
		AOS = 1;
		BOS = 0;
	}
	else  // tbd:
	{
		AIS = MAX_INS + 1;
		AOS = MAX_AOS + 1;
		BOS = 0;
	}
  Count_VAR_Object_Number();

}
//#define SWITCH_TIMER	 600
void vMSTP_TASK(void *pvParameters )
{
	uint16_t pdu_len = 0; 
	BACNET_ADDRESS  src;
//	static u16 protocal_timer = SWITCH_TIMER;
	modbus.protocal_timer_enable = 0;
//	bacnet_inital();	
	Inital_Bacnet_Server();
	dlmstp_init(NULL);
	Recievebuf_Initialize(0);
//	print("MSTP Task\r\n");
	delay_ms(100);
	task_test.enable[1] = 1;
	for (;;)
    {
//			if(update_flag == 7)
//			{
//				Lcd_Initial();
//				Lcd_Show_String(1, 18, DISP_NOR, (uint8 *)ppm_text);
//			  print_online_status(TRUE);
//			  update_flag = 0;
//			}
		
//		if(modbus.protocal_timer_enable == 1) // switch to bac_mstp,delay 3000 ms
//		{	
//			if(protocal_timer !=0) protocal_timer--;
//			else
//			{
//				u8 i;
//				modbus.protocal_timer_enable = 0;
//				protocal_timer  =SWITCH_TIMER;
//				
//				for(i=0;i < USART_REC_LEN;i++)
//					USART_RX_BUF[revce_count++] = 0; //clear receiver buf
//	 
//				serial_restart();
//				
//				modbus.protocal= BAC_MSTP;
//				AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, BAC_MSTP);
//			}
//		} 
			
		task_test.count[1]++;	Test[40] = 1;
		if(modbus.protocal == BAC_MSTP)
		{
			pdu_len = datalink_receive(&src, &PDUBuffer[0], sizeof(PDUBuffer), 0,	modbus.protocal);
			if(pdu_len) 
			{
				npdu_handler(&src, &PDUBuffer[0], pdu_len, BAC_MSTP);	
			} 
						
		}
// 		stack_detect(&test[1]);
		vTaskDelay(5 / portTICK_RATE_MS);
	} 	
}

////extern u32 count_out;
////extern u8 buffer_out[64];
//void vUSBTask( void *pvParameters )
//{
////	u8 i;
//	for( ;; )
//	{
////		if((count_out != 0) && (bDeviceState == CONFIGURED))
////		{
//////			USB_To_USART_Send_Data(&buffer_out[0], count_out);
////			for(i = 0; i < count_out; i++)
////			{

////			}
////			count_out = 0;
////		}
//		delay_ms(200);

//	}
//}


void vNETTask( void *pvParameters )
{
	//uip_ipaddr_t ipaddr;
	
	//u8 tcp_server_tsta = 0XFF;
	//u8 tcp_client_tsta = 0XFF;
	//print("Temco ARM Demo\r\n");
//	delay_ms(500);
	u8 count = 0 ;
//	while(tapdev_init())	//��ʼ��ENC28J60����
//	{								   
////	 	print("ENC28J60 Init Error!\r\n");
////		delay_ms(100);
//		;
//	};		
	//print("ENC28J60 Init OK!\r\n");
//	uip_init();							//uIP��ʼ��
//	
//	uip_ipaddr(ipaddr, 192, 168, 0, 163);	//���ñ�������IP��ַ
//	uip_sethostaddr(ipaddr);					    
//	uip_ipaddr(ipaddr, 192, 168, 0, 4); 	//��������IP��ַ(��ʵ������·������IP��ַ)
//	uip_setdraddr(ipaddr);						 
//	uip_ipaddr(ipaddr, 255, 255, 255, 0);	//������������
//	uip_setnetmask(ipaddr);

//	print("IP:192.168.0.163\r\n"); 
//	print("MASK:255.255.255.0\r\n"); 
//	print("GATEWAY:192.168.0.4\r\n"); 	
//	
//	uip_listen(HTONS(1200));			//����1200�˿�,����TCP Server
//	uip_listen(HTONS(80));				//����80�˿�,����Web Server
//	tcp_client_reconnect();	   		    //�������ӵ�TCP Server��,����TCP Client
//	print("N e t Task\r\n");
	delay_ms(100);
	task_test.enable[0] = 1;
  for( ;; )
	{
		//low_pri++;
		task_test.count[0]++; 
		//watchdog();
		uip_polling();	//����uip�¼���������뵽�û������ѭ������ 
//		if((IP_Change == 1)/*||(update == 1)*/)
//		{
//			count++ ;
//			if(count == 10)
//			{
//				count = 0 ;
//				IP_Change = 0 ;	
//				
////				//if(!tapdev_init()) print("Init fail\n\r");				
////				while(tapdev_init())	//��ʼ��ENC28J60����
////				{								   
////				//	print("ENC28J60 Init Error!\r\n");
////				delay_ms(50);
////				};	
//				
//				
//				SoftReset();
//			}			
//		}
		
//		stack_detect(&test[0]);
		
//		if(tcp_server_tsta != tcp_server_sta)		//TCP Server״̬�ı�
//		{															 
//			if(tcp_server_sta & (1 << 7))
//				print("TCP Server Connected   \r\n");
//			else
//				print("TCP Server Disconnected\r\n"); 
//			
// 			if(tcp_server_sta & (1 << 6))			//�յ�������
//			{
//   			print("TCP Server RX:%s\r\n", tcp_server_databuf);//��ӡ����
//				tcp_server_sta &= ~(1 << 6);		//��������Ѿ�������	
//			}
//			tcp_server_tsta = tcp_server_sta;
//		}
//		
//		if(global_key == KEY_1)						//TCP Server ����������
//		{
//			global_key = KEY_NON;
//			if(tcp_server_sta & (1 << 7))			//���ӻ�����
//			{
//				sprint((char*)tcp_server_databuf, "TCP Server OK\r\n");	 
//				tcp_server_sta |= 1 << 5;			//�����������Ҫ����
//			}
//		}
//		
//		if(tcp_client_tsta != tcp_client_sta)		//TCP Client״̬�ı�
//		{
//			if(tcp_client_sta & (1 << 7))
//				print("TCP Client Connected   \r\n");
//			else
//				print("TCP Client Disconnected\r\n");
//			
// 			if(tcp_client_sta & (1 << 6))			//�յ�������
//			{
//    			print("TCP Client RX:%s\r\n", tcp_client_databuf);//��ӡ����
//				tcp_client_sta &= ~(1 << 6);		//��������Ѿ�������	
//			}
//			tcp_client_tsta = tcp_client_sta;
//		}
//		
//		if(global_key == KEY_2)						//TCP Client ����������
//		{
//			global_key = KEY_NON;
//			if(tcp_client_sta & (1 << 7))			//���ӻ�����
//			{
//				sprint((char*)tcp_client_databuf, "TCP Client OK\r\n");	 
//				tcp_client_sta |= 1 << 5;			//�����������Ҫ����
//			}
//		}
		
//		if(global_key == KEY_3)
//		{
//			global_key = KEY_NON;
//			print("global_key == KEY_3\r\n");
//			mf_mount(0);
//			mf_scan_files("0:");
//			mf_showfree("0:");
//		}
		
 		vTaskDelay(5 / portTICK_RATE_MS);
    }
}



//uip�¼�������
//���뽫�ú��������û���ѭ��,ѭ������.
void uip_polling(void)
{
	u8 i;
	static struct timer periodic_timer, arp_timer;
	static u8 timer_ok = 0;	 
	if(timer_ok == 0)		//����ʼ��һ��
	{
		timer_ok = 1;
		timer_set(&periodic_timer, CLOCK_SECOND / 2); 	//����1��0.5��Ķ�ʱ�� 
		timer_set(&arp_timer, CLOCK_SECOND * 10);	   	//����1��10��Ķ�ʱ�� 
	}
	
	uip_len = tapdev_read();							//�������豸��ȡһ��IP��,�õ����ݳ���.uip_len��uip.c�ж���
	if(uip_len > 0)							 			//������
	{   
		//����IP���ݰ�(ֻ��У��ͨ����IP���Żᱻ����) 
		if(BUF->type == htons(UIP_ETHTYPE_IP))			//�Ƿ���IP��? 
		{
			uip_arp_ipin();								//ȥ����̫��ͷ�ṹ������ARP��
			uip_input();   								//IP������			
			//������ĺ���ִ�к������Ҫ�������ݣ���ȫ�ֱ��� uip_len > 0
			//��Ҫ���͵�������uip_buf, ������uip_len  (����2��ȫ�ֱ���)		    
			if(uip_len > 0)								//��Ҫ��Ӧ����
			{
				uip_arp_out();							//����̫��ͷ�ṹ������������ʱ����Ҫ����ARP����
				tapdev_send();							//�������ݵ���̫��
			}
		}
		else if (BUF->type == htons(UIP_ETHTYPE_ARP))	//����arp����,�Ƿ���ARP�����?
		{
			uip_arp_arpin();
			
 			//������ĺ���ִ�к������Ҫ�������ݣ���ȫ�ֱ���uip_len>0
			//��Ҫ���͵�������uip_buf, ������uip_len(����2��ȫ�ֱ���)
 			if(uip_len > 0)
				tapdev_send();							//��Ҫ��������,��ͨ��tapdev_send����	 
		}
	}
	else if(timer_expired(&periodic_timer))				//0.5�붨ʱ����ʱ
	{
		timer_reset(&periodic_timer);					//��λ0.5�붨ʱ�� 
		
		//��������ÿ��TCP����, UIP_CONNSȱʡ��40��  
		for(i = 0; i < UIP_CONNS; i++)
		{
			 uip_periodic(i);							//����TCPͨ���¼�
			
	 		//������ĺ���ִ�к������Ҫ�������ݣ���ȫ�ֱ���uip_len>0
			//��Ҫ���͵�������uip_buf, ������uip_len (����2��ȫ�ֱ���)
	 		if(uip_len > 0)
			{
				uip_arp_out();							//����̫��ͷ�ṹ������������ʱ����Ҫ����ARP����
				tapdev_send();							//�������ݵ���̫��
			}
		}
		
#if UIP_UDP	//UIP_UDP 
		//��������ÿ��UDP����, UIP_UDP_CONNSȱʡ��10��
		for(i = 0; i < UIP_UDP_CONNS; i++)
		{
			uip_udp_periodic(i);						//����UDPͨ���¼�
			
	 		//������ĺ���ִ�к������Ҫ�������ݣ���ȫ�ֱ���uip_len>0
			//��Ҫ���͵�������uip_buf, ������uip_len (����2��ȫ�ֱ���)
			if(uip_len > 0)
			{
				uip_arp_out();							//����̫��ͷ�ṹ������������ʱ����Ҫ����ARP����
				tapdev_send();							//�������ݵ���̫��
			}
		}
#endif 
		//ÿ��10�����1��ARP��ʱ������ ���ڶ���ARP����,ARP��10�����һ�Σ��ɵ���Ŀ�ᱻ����
		if(timer_expired(&arp_timer))
		{
			timer_reset(&arp_timer);
			uip_arp_timer();
		}
	}
}

u8 cpu_type;
void EEP_Dat_Init(void)
{
		u8 loop ;
		u8 temp[6];
		int16 itemp;
		AT24CXX_Init();
	  SHT3X_Init(0x45); 


	  initial_hum_eep();
		modbus.serial_Num[0] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD);
		modbus.serial_Num[1] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD+1);
		modbus.serial_Num[2] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_HIWORD);
		modbus.serial_Num[3] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_HIWORD+1);
	
		cpu_type = AT24CXX_ReadOneByte(EEP_CPU_TYPE);
//		if((modbus.serial_Num[0]==0xff)&&(modbus.serial_Num[1]== 0xff)&&(modbus.serial_Num[2] == 0xff)&&(modbus.serial_Num[3] == 0xff))
//		{
//					modbus.serial_Num[0] = 1 ;
//					modbus.serial_Num[1] = 1 ;
//					modbus.serial_Num[2] = 2 ;
//					modbus.serial_Num[3] = 2 ;
//					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD, modbus.serial_Num[0]);
//					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD+1, modbus.serial_Num[1]);
//					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD+2, modbus.serial_Num[2]);
//					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD+3, modbus.serial_Num[3]);
//		}
//		Instance = modbus.serial_Num[0] + (U16_T)(modbus.serial_Num[1] << 8);
//		temp[0] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD);
//		temp[1] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD);
//		temp[2] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD);
//		temp[3] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD);
//		if((temp[0]==0xff)&&(temp[1]== 0xff)&&(temp[2] == 0xff)&&(temp[3] == 0xff))
//		{
//				temp[0] = 1 ;
//				temp[1] = 2 ;
//				temp[2] = 3 ;
//				temp[3] = 4 ;
//				AT24CXX_WriteOneByte(EEP_INSTANCE_1, temp[0]);
//				AT24CXX_WriteOneByte(EEP_INSTANCE_1+1, temp[1]);
//				AT24CXX_WriteOneByte(EEP_INSTANCE_1+2, temp[2]);
//				AT24CXX_WriteOneByte(EEP_INSTANCE_1+3, temp[3]);
//		}
//		Instance = ((u32)temp[0]<<24) +((u32)temp[1]<<16)+((u32)temp[2]<<8)+(u32)temp[0] ;
		AT24CXX_WriteOneByte(EEP_VERSION_NUMBER_LO, SOFTREV&0XFF);
		AT24CXX_WriteOneByte(EEP_VERSION_NUMBER_HI, (SOFTREV>>8)&0XFF);
		modbus.address = AT24CXX_ReadOneByte(EEP_ADDRESS);
		if((modbus.address == 255)||(modbus.address == 0))
		{
			if(isWagnerProduct )
				modbus.address = 2;
			else
				modbus.address = 254 ;
			AT24CXX_WriteOneByte(EEP_ADDRESS, modbus.address);
		}
		modbus.product = AT24CXX_ReadOneByte(EEP_PRODUCT_MODEL);
		PRODUCT_ID = modbus.product;
//		if(modbus.product != PRODUCT_ID)//((modbus.product == 255)||(modbus.product == 0))
//		{
//			modbus.product = PRODUCT_ID ;
//			AT24CXX_WriteOneByte(EEP_PRODUCT_MODEL, modbus.product);
//		}
		modbus.hardware_Rev = AT24CXX_ReadOneByte(EEP_HARDWARE_REV);
		if((modbus.hardware_Rev == 255)||(modbus.hardware_Rev == 0))
		{
					modbus.hardware_Rev = HW_VER ;
					AT24CXX_WriteOneByte(EEP_HARDWARE_REV, modbus.hardware_Rev);
		}
		modbus.update = AT24CXX_ReadOneByte(EEP_UPDATE_STATUS);
		modbus.SNWriteflag = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_WRITE_FLAG);
		
 		if ((PRODUCT_ID == STM32_HUM_RS485) ||(PRODUCT_ID == STM32_HUM_NET) || (PRODUCT_ID == STM32_PM25))
		{
			uart1_parity =  AT24CXX_ReadOneByte(EEP_UART1_PARITY);
			if(( uart1_parity!= NONE_PARITY)&&( uart1_parity!= ODD_PARITY)&&( uart1_parity!= EVEN_PARITY))
				uart1_parity = NONE_PARITY;	
		}
		else
		{
			uart1_parity = NONE_PARITY;	 
		}
		modbus.baud = AT24CXX_ReadOneByte(EEP_BAUDRATE);
		if(modbus.baud > 5) 
		{	
			modbus.baud = 1 ;
			AT24CXX_WriteOneByte(EEP_BAUDRATE, modbus.baud);
		}
		switch(modbus.baud)
				{
					case 0:
						modbus.baudrate = BAUDRATE_9600 ;
						uart1_init(BAUDRATE_9600);
				
						SERIAL_RECEIVE_TIMEOUT = 6;
					break ;
					case 1:
						modbus.baudrate = BAUDRATE_19200 ;
						uart1_init(BAUDRATE_19200);	
						SERIAL_RECEIVE_TIMEOUT = 3;
					break;
					case 2:
						modbus.baudrate = BAUDRATE_38400 ;
						uart1_init(BAUDRATE_38400);
						SERIAL_RECEIVE_TIMEOUT = 2;
					break;
					case 3:
						modbus.baudrate = BAUDRATE_57600 ;
						uart1_init(BAUDRATE_57600);	
						SERIAL_RECEIVE_TIMEOUT = 1;
					break;
					case 4:
						modbus.baudrate = BAUDRATE_115200 ;
						uart1_init(BAUDRATE_115200);	
						SERIAL_RECEIVE_TIMEOUT = 1;
					break;
					
					case 5:
						modbus.baudrate = BAUDRATE_76800 ;
						uart1_init(BAUDRATE_76800);	
						SERIAL_RECEIVE_TIMEOUT = 1;
					break;

					default:
					break ;				
				}
 				modbus.protocal= AT24CXX_ReadOneByte(EEP_MODBUS_COM_CONFIG);
				if((modbus.protocal!=MODBUS)&&(modbus.protocal!=BAC_MSTP ))
				{
					modbus.protocal = MODBUS ;
					AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, modbus.protocal);
				}
				//modbus.protocal = BAC_MSTP;
				
//				modbus.protocal= AT24CXX_ReadOneByte(EEP_MODBUS_COM_CONFIG);
//				if(modbus.protocal >10)
//				{
//					modbus.protocal = MODBUS;
//					AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, modbus.protocal);
//				}
//				modbus.protocal = MODBUS ;
//				modbus.protocal = BAC_MSTP;
				for(loop = 0 ; loop<6; loop++)
				{
					temp[loop] = AT24CXX_ReadOneByte(EEP_MAC_ADDRESS_1+loop); 
				}
				if((temp[0]== 0xff)&&(temp[1]== 0xff)&&(temp[2]== 0xff)&&(temp[3]== 0xff)&&(temp[4]== 0xff)&&(temp[5]== 0xff) )
				{
					temp[0] = 0x04 ;
					temp[1] = 0x02 ;
					temp[2] = 0x35 ;
					temp[3] = 0xaF ;
					temp[4] = 0x00 ;
					temp[5] = 0x01 ;
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_1, temp[0]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_2, temp[1]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_3, temp[2]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_4, temp[3]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_5, temp[4]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_6, temp[5]);		
				}
				for(loop =0; loop<6; loop++)
				{
					modbus.mac_addr[loop] =  temp[loop]	;
				}
				
				for(loop = 0 ; loop<4; loop++)
				{
					temp[loop] = AT24CXX_ReadOneByte(EEP_IP_ADDRESS_1+loop); 
				}
				if((temp[0]== 0xff)&&(temp[1]== 0xff)&&(temp[2]== 0xff)&&(temp[3]== 0xff) )
				{
					temp[0] = 192 ;
					temp[1] = 168 ;
					temp[2] = 0;
					temp[3] = 3;
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_1, temp[0]);
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_2, temp[1]);
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_3, temp[2]);
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_4, temp[3]);
				}
				for(loop = 0 ; loop<4; loop++)
				{
					modbus.ip_addr[loop] = 	temp[loop] ;
					modbus.ghost_ip_addr[loop] = modbus.ip_addr[loop] ;
				}
				
				temp[0] = AT24CXX_ReadOneByte(EEP_IP_MODE);
				if(temp[0] >1)
				{
					temp[0] = 0 ;
					AT24CXX_WriteOneByte(EEP_IP_MODE, temp[0]);	
				}
				modbus.ip_mode = temp[0] ;
				modbus.ghost_ip_mode = modbus.ip_mode ;
				
				
				for(loop = 0 ; loop<4; loop++)
				{
					temp[loop] = AT24CXX_ReadOneByte(EEP_SUB_MASK_ADDRESS_1+loop); 
				}
				if((temp[0]== 0xff)&&(temp[1]== 0xff)&&(temp[2]== 0xff)&&(temp[3]== 0xff) )
				{
					temp[0] = 0xff ;
					temp[1] = 0xff ;
					temp[2] = 0xff ;
					temp[3] = 0 ;
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_1, temp[0]);
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_2, temp[1]);
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_3, temp[2]);
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_4, temp[3]);
				
				}				
				for(loop = 0 ; loop<4; loop++)
				{
					modbus.mask_addr[loop] = 	temp[loop] ;
					modbus.ghost_mask_addr[loop] = modbus.mask_addr[loop] ;
				}
				
				for(loop = 0 ; loop<4; loop++)
				{
					temp[loop] = AT24CXX_ReadOneByte(EEP_GATEWAY_ADDRESS_1+loop); 
				}
				if((temp[0]== 0xff)&&(temp[1]== 0xff)&&(temp[2]== 0xff)&&(temp[3]== 0xff) )
				{
					temp[0] = 192 ;
					temp[1] = 168 ;
					temp[2] = 0 ;
					temp[3] = 4 ;
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_1, temp[0]);
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_2, temp[1]);
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_3, temp[2]);
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_4, temp[3]);
				
				}				
				for(loop = 0 ; loop<4; loop++)
				{
					modbus.gate_addr[loop] = 	temp[loop] ;
					modbus.ghost_gate_addr[loop] = modbus.gate_addr[loop] ;
				}
				
				temp[0] = AT24CXX_ReadOneByte(EEP_TCP_SERVER);
				if(temp[0] == 0xff)
				{
					temp[0] = 0 ;
					AT24CXX_WriteOneByte(EEP_TCP_SERVER, temp[0]);
				}
				modbus.tcp_server = temp[0];
				modbus.ghost_tcp_server = modbus.tcp_server  ;
				
				temp[0] =AT24CXX_ReadOneByte(EEP_LISTEN_PORT_HI);
				temp[1] =AT24CXX_ReadOneByte(EEP_LISTEN_PORT_LO);
				if(temp[0] == 0xff && temp[1] == 0xff )
				{
					modbus.listen_port = 502 ;
					temp[0] = (modbus.listen_port>>8)&0xff ;
					temp[1] = modbus.listen_port&0xff ;				
				}
				modbus.listen_port = (temp[0]<<8)|temp[1] ;
				modbus.ghost_listen_port = modbus.listen_port ;
				
				modbus.write_ghost_system = 0 ;
				modbus.reset = 0 ;
	 
				
					
//	alarm_state = read_eeprom(MODBUS_ALARM_AUTO_MANUAL);
//	if((alarm_state == 0x00) || (alarm_state == 0xff))
		alarm_state = STOP_ALARM;
//	else
//		alarm_state &= 0x83;

	pre_alarm_on_time = read_eeprom(EEP_PRE_ALARM_SETTING_ON_TIME);
	if((pre_alarm_on_time == 0x00) || (pre_alarm_on_time == 0xff))
		pre_alarm_on_time = 2;
	else if(pre_alarm_on_time > ALARM_ON_TIME_MAX)
		pre_alarm_on_time = ALARM_ON_TIME_MAX;

	pre_alarm_off_time = read_eeprom(EEP_PRE_ALARM_SETTING_OFF_TIME);
	if((pre_alarm_off_time == 0x00) || (pre_alarm_off_time == 0xff))
		pre_alarm_off_time = 2;
	else if(pre_alarm_off_time > ALARM_OFF_TIME_MAX)
		pre_alarm_off_time = ALARM_OFF_TIME_MAX;

// co2 output range
	itemp = ((int16)read_eeprom(EEP_OUTPUT_CO2_RANGE_MIN + 1) << 8) | read_eeprom(EEP_OUTPUT_CO2_RANGE_MIN);
//	if(itemp != -1)
 		output_range_table[CHANNEL_CO2].min = itemp;
//	else
//		output_range_table[CHANNEL_CO2].min = 0;

	itemp = ((int16)read_eeprom(EEP_OUTPUT_CO2_RANGE_MAX + 1) << 8) | read_eeprom(EEP_OUTPUT_CO2_RANGE_MAX);
//	if(itemp != -1)
 		output_range_table[CHANNEL_CO2].max = itemp;
//	else
//		#ifdef CO2_SENSOR
//		output_range_table[CHANNEL_CO2].max = 2000;
//	    #else 
//		output_range_table[CHANNEL_CO2].max = 1000;
//		#endif

	if(output_range_table[CHANNEL_CO2].min > output_range_table[CHANNEL_CO2].max)
	{
		itemp = output_range_table[CHANNEL_CO2].max;
		output_range_table[CHANNEL_CO2].min = output_range_table[CHANNEL_CO2].max;
		output_range_table[CHANNEL_CO2].max = itemp;
	}

// temperature output range
	itemp = ((int16)read_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN + 1) << 8) | read_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN);
//	if(itemp != -1)
 		output_range_table[CHANNEL_TEMP].min = itemp;
//	else
//		output_range_table[CHANNEL_TEMP].min = 0;

	itemp = ((int16)read_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX + 1) << 8) | read_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX);
//	if(itemp != -1)
 		output_range_table[CHANNEL_TEMP].max = itemp;
//	else
//		output_range_table[CHANNEL_TEMP].max = 1000;

	if(output_range_table[CHANNEL_TEMP].min > output_range_table[CHANNEL_TEMP].max)
	{
		itemp = output_range_table[CHANNEL_TEMP].max;
		output_range_table[CHANNEL_TEMP].min = output_range_table[CHANNEL_TEMP].max;
		output_range_table[CHANNEL_TEMP].max = itemp;
	}

// humidity output range
	itemp = ((int16)read_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN + 1) << 8) | read_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN);
//	if(itemp != -1)
 		output_range_table[CHANNEL_HUM].min = itemp;
//	else
//		output_range_table[CHANNEL_HUM].min = 0;

	itemp = ((int16)read_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX + 1) << 8) | read_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX);
//	if(itemp != -1)
 		output_range_table[CHANNEL_HUM].max = itemp;
//	else
//		output_range_table[CHANNEL_HUM].max = 1000;

	if(output_range_table[CHANNEL_HUM].min > output_range_table[CHANNEL_HUM].max)
	{
		itemp = output_range_table[CHANNEL_HUM].max;
		output_range_table[CHANNEL_HUM].min = output_range_table[CHANNEL_HUM].max;
		output_range_table[CHANNEL_HUM].max = itemp;
	}

 

// password
	itemp = read_eeprom(EEP_USE_PASSWORD);
	if((itemp == 0x00) || (itemp == 0xff))
		use_password = 0;
	else
		use_password = 1;

	user_password[0] = read_eeprom(EEP_USER_PASSWORD0);
	user_password[1] = read_eeprom(EEP_USER_PASSWORD1);
	user_password[2] = read_eeprom(EEP_USER_PASSWORD2);
	user_password[3] = read_eeprom(EEP_USER_PASSWORD3);
	if((user_password[0] > '9') || (user_password[0] < '0')
	|| (user_password[1] > '9') || (user_password[1] < '0')
	|| (user_password[2] > '9') || (user_password[2] < '0')
	|| (user_password[3] > '9') || (user_password[3] < '0'))
	{
		user_password[0] = '1';
		user_password[1] = '2';
		user_password[2] = '3';
		user_password[3] = '4';
		write_eeprom(EEP_USER_PASSWORD0, user_password[0]);
		write_eeprom(EEP_USER_PASSWORD1, user_password[1]);
		write_eeprom(EEP_USER_PASSWORD2, user_password[2]);
		write_eeprom(EEP_USER_PASSWORD3, user_password[3]);
	}

//	menu_block_seconds = read_eeprom(EEP_MENU_BLOCK_SECONDS);
//	if((menu_block_seconds == 0) || (menu_block_seconds == 0xff))
//		menu_block_seconds = MENU_BLOCK_SECONDS_DEFAULT;
//	refresh_menu_block_timer();
	data_backup_rev = read_eeprom(EEP_DATA_BACKUP);
	if(data_backup_rev == 255 || data_backup_rev == 0)
	{
		write_eeprom(EEP_DATA_BACKUP,1);
		write_eeprom(EEP_BACKLIGHT_KEEP_SECONDS,255);
	}
	backlight_keep_seconds = read_eeprom(EEP_BACKLIGHT_KEEP_SECONDS);
//	if(backlight_keep_seconds == 0xff)
//		backlight_keep_seconds = BACKLIGHT_KEEP_SECONDS_DEFAULT;
	
	
	HumSensor.H_Filter = read_eeprom(EEP_HUMIDITY_FILTER);
	if(HumSensor.H_Filter == 0xff)
	{
		HumSensor.H_Filter = DEFAULT_FILTER;
		write_eeprom(EEP_HUMIDITY_FILTER,DEFAULT_FILTER); 
	}
	
	HumSensor.T_Filter = read_eeprom(EEP_EXT_TEMPERATURE_FILTER);
	if(HumSensor.T_Filter == 0xff)
	{
		HumSensor.T_Filter = DEFAULT_FILTER;
		write_eeprom(EEP_EXT_TEMPERATURE_FILTER,DEFAULT_FILTER);
	}

	Temperature_Filter = read_eeprom(EEP_INT_TEMPERATURE_FILTER);
	if(Temperature_Filter == 0xff)
	{
		Temperature_Filter = DEFAULT_FILTER;
		write_eeprom(EEP_INT_TEMPERATURE_FILTER,DEFAULT_FILTER); 
	}
	HumSensor.pre_temperature_c = 0;
	HumSensor.pre_humidity = 0; 	
	
	AT24CXX_Read(EEP_TSTAT_NAME1, panelname, 21); 
	
	
	output_offset[0][CHANNEL_HUM] = ((uint16)read_eeprom(EEP_OUTPUT_HUM_VOL_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_HUM_VOL_OFFSET);
	output_offset[0][CHANNEL_TEMP] =((uint16)read_eeprom(EEP_OUTPUT_TEMP_VOL_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_TEMP_VOL_OFFSET);
	output_offset[0][CHANNEL_CO2] =((uint16)read_eeprom(EEP_OUTPUT_CO2_VOL_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_CO2_VOL_OFFSET);
	output_offset[1][CHANNEL_HUM] =((uint16)read_eeprom(EEP_OUTPUT_HUM_CUR_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_HUM_CUR_OFFSET);
	output_offset[1][CHANNEL_TEMP] =((uint16)read_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET);
	output_offset[1][CHANNEL_CO2] =((uint16)read_eeprom(EEP_OUTPUT_CO2_CUR_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_CO2_CUR_OFFSET);
 	
	pm25_unit = read_eeprom(EEP_PM25_UNIT);
	pm25_rand_offset = read_eeprom(EEP_PM25_RAND_OFFSET);
	if(pm25_rand_offset == 0xff)
	{
		pm25_rand_offset = 0;
		write_eeprom(EEP_PM25_RAND_OFFSET,pm25_rand_offset); 
	}
		
	pm25_rand_sign = read_eeprom(EEP_PM25_RAND_SIGN);
	if(pm25_rand_sign > 1)
	{
		pm25_rand_sign = 0;
		write_eeprom(EEP_PM25_RAND_SIGN,pm25_rand_sign); 
	}
	
	if(isColorScreen == true)
	{
		if(PRODUCT_ID == STM32_PM25)
		{
			isWagnerProduct = read_eeprom(EEP_IS_WAGNER_PRODUCT);
			if(isWagnerProduct > 1)
			{
				isWagnerProduct = 0;
				write_eeprom(EEP_IS_WAGNER_PRODUCT,isWagnerProduct); 
			}
		}
		else
			isWagnerProduct = 0;
	
// first line 		
		screenArea1 = read_eeprom(EEP_SCREEN_AREA_1);
		if(screenArea1 == 0xff)
		{
			screenArea1 = SCREEN_AREA_TEMP;
			write_eeprom(EEP_SCREEN_AREA_1,screenArea1); 
		}
		if(PRODUCT_ID == STM32_PM25)
		{
			if(isWagnerProduct)
				screenArea1 = SCREEN_AREA_PM25;
			else
				screenArea1 = SCREEN_AREA_AQI;
		}
		else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				screenArea1 = SCREEN_AREA_PRESSURE;
		else if(PRODUCT_ID == STM32_HUM_RS485 || PRODUCT_ID == STM32_HUM_NET)
		{
			if(sub_product == 1) // RTS2
			{
				screenArea1 = SCREEN_AREA_NONE;
			}			
			else
				screenArea1 = SCREEN_AREA_TEMP;
		}
		
// second line 			
		screenArea2 = read_eeprom(EEP_SCREEN_AREA_2);
		if(screenArea2 == 0xff)
		{
			screenArea2 = SCREEN_AREA_HUMI;
			write_eeprom(EEP_SCREEN_AREA_2,screenArea2); 
		}
		if(PRODUCT_ID == STM32_PM25)
		{
			if(isWagnerProduct)
				screenArea2 = SCREEN_AREA_PM10;
			else
				screenArea2 = SCREEN_AREA_PM25;
		}
		else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				screenArea2 = SCREEN_AREA_PRESSURE;
		else if(PRODUCT_ID == STM32_HUM_RS485 || PRODUCT_ID == STM32_HUM_NET)
		{
			if(sub_product == 1) // RTS2
			{
				screenArea2 = SCREEN_AREA_TEMP;
			}
			else
				screenArea2 = SCREEN_AREA_HUMI;
		}
		
// third line		
		screenArea3 = read_eeprom(EEP_SCREEN_AREA_3);
		if(screenArea3 == 0xff)
		{
			screenArea3 = SCREEN_AREA_CO2;
			write_eeprom(EEP_SCREEN_AREA_3,screenArea3); 
		}		
		if(PRODUCT_ID == STM32_PM25)
		{
			if(isWagnerProduct)
				screenArea3 = SCREEN_AREA_NONE;
			else
				screenArea3 = SCREEN_AREA_PM10;
		}			
		else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				screenArea3 = SCREEN_AREA_PRESSURE;
		else if(PRODUCT_ID == STM32_HUM_RS485 || PRODUCT_ID == STM32_HUM_NET)
		{
			if(sub_product == 2) // AQ
			{
				screenArea3 = SCREEN_AREA_AQI;
			}
		}
		
		
		enableScroll = read_eeprom(EEP_ENABLE_SCROLL);
		if(enableScroll > 1)
		{
			if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
				enableScroll = false;
			else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				enableScroll = true;
			else if(PRODUCT_ID == STM32_HUM_RS485 || PRODUCT_ID == STM32_HUM_NET)
				enableScroll = true;

			write_eeprom(EEP_ENABLE_SCROLL, enableScroll);
		}
		
		if(PRODUCT_ID == STM32_PM25)
		{
			if(isWagnerProduct)
				enableScroll = true;
			else
				enableScroll = false;
		}
		alarmEnable = read_eeprom(EEP_ENABLE_ALARM);
		if(alarmEnable >1)
		{
			alarmEnable = true;
			write_eeprom(EEP_ENABLE_ALARM, alarmEnable);
		}
	}
	
//	pm25_sensor.pm25_range = read_eeprom(EEP_PM25_RANGE);
//	if(pm25_sensor.pm25_range > PM25_0_1000)
//	{
//		pm25_sensor.pm25_range = PM25_0_100;
//		write_eeprom(EEP_PM25_RANGE, pm25_sensor.pm25_range);
//	}
//	if(read_eeprom(EEP_HUM_CLK_DELAY) == 0xff)
//		hum_read_delay = 10;
//	else
//		hum_read_delay = (uint16)read_eeprom(EEP_HUM_CLK_DELAY);
//	if(hum_read_delay > 100)
//		hum_read_delay = 1;
	
}


u16 swap_int16( u16 value)
{
	u8 temp1, temp2 ;
	temp1 = value &0xff ;
	temp2 = (value>>8)&0xff ;
	
	return  (temp1<<8)|temp2 ;
}

u32 swap_int32( u32 value)
{
	u8 temp1, temp2, temp3, temp4 ;
	temp1 = value &0xff ;
	temp2 = (value>>8)&0xff ;
	temp3 = (value>>16)&0xff ;
	temp4 = (value>>24)&0xff ;
	
	return  ((u32)temp1<<24)|((u32)temp2<<16)|((u32)temp3<<8)|temp4 ;
}


void watchdog_init(void)
{
		/* Enable write access to IWDG_PR and IWDG_RLR registers */ 
		IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
		/* IWDG counter clock: 40KHz(LSI) / 128 = 0.3125 KHz */ 
		IWDG_SetPrescaler(IWDG_Prescaler_128); 
		/* Set counter reload value to 4000 = 12.8s */ 
		IWDG_SetReload(4000); 
		IWDG_Enable();  			//enable the watchdog
		IWDG_ReloadCounter(); // reload the value
}

 
 
void watchdog(void)
{
	IWDG_ReloadCounter(); // reload the value     
}

void SCD40_get_value(uint16_t co2,uint16_t temperaute, uint16_t humidity)
{
	int_co2_str.co2_int	= co2 + int_co2_str.co2_offset / 10;	

	if(hum_exists == 0)
	{// if no humidity, use SCD40
		HumSensor.temperature_c = temperaute + HumSensor.offset_t;		
		HumSensor.humidity = humidity + HumSensor.offset_h;		
		HumSensor.temperature_f = HumSensor.temperature_c * 9 / 5 + 320;
	}
//	else
//	{
//		internal_temperature_c = temperaute;
//	}
	int_co2_str.temperature = temperaute;
	int_co2_str.humi = humidity;
	var[CHANNEL_CO2].value = co2;
	int_co2_str.warming_time = FALSE;
	output_manual_value_co2 = int_co2_str.co2_int;

}


// check task		
void check_Task_locked(void)
{
	char loop;
	Test[31]++;
	for(loop = 0;loop < 16;loop++)	
	{				
		if(task_test.enable[loop] == 1)
		{
			if(task_test.count[loop] != task_test.old_count[loop])
			{
				task_test.old_count[loop] = task_test.count[loop];
				task_test.inactive_count[loop] = 0;
			}
			else
				task_test.inactive_count[loop]++;
		}
		
		if(task_test.inactive_count[loop] > 20)	
		{ 	
			AT24CXX_WriteOneByte(EEP_HARDFAULT5, loop);
			Test[40] = 101;
			SoftReset();

		}	
	} 		
}