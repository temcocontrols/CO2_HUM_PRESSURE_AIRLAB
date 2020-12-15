#include "config.h"
#include "sht3x.h"
#include "temperature.h"

int16 ctest[20];
uint8 update_flag = 0;
unsigned char PRODUCT_ID;  
static void vFlashTask( void *pvParameters );  
static void vCOMMTask( void *pvParameters );

//static void vUSBTask( void *pvParameters );

static void vNETTask( void *pvParameters );


 void vLCDtask(void *pvParameters);
 
static void vMSTP_TASK(void *pvParameters );
void uip_polling(void);

static void watchdog_init(void);
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

//u8 global_key = KEY_NON;

static void debug_config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

int main(void)
{
	uint8 rtc_state = 0, rtc_state1 = 1;
  uint16 j;	
 	uint8 i; 
	int16 offset = 0;
// 	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);
// 	debug_config(); 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
 	delay_init(72);	
  
	watchdog_init();
	qSubSerial3 = xQueueCreate(UART3_MAX_BUF, 1);
	
	qSubSerial = xQueueCreate(SUB_BUF_LEN, 1);
	
	xMutex = xQueueCreateMutex();
	IicMutex = xQueueCreateMutex();
	qKey = xQueueCreate(5, 2);
	if(( qSubSerial3 == NULL )||( qSubSerial == NULL )  ||( xMutex == NULL )||( IicMutex == NULL ))    
	{
		while(1);
	}
	if(read_eeprom(EEP_CLEAR_EEP) == 99)
	{
		for(j=66;j<2047;j++)
		{
		 write_eeprom(j,0xff);
		 watchdog();
		}
		write_eeprom(EEP_UPDATE_STATUS,0);
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
	if(isColorScreen == false)
		start_back_light(backlight_keep_seconds);

//	print("EEP Init Done!\r\n");
   	
 	mass_flash_init();
	
	
//	print("FLASH Init Done!\r\n");
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
//	TIM6_Int_Init(100, 7199);
   	
//	uart3_modbus_init();
	
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
	{
		//RTC_Init();
		co2_autocal_disable = read_eeprom(EEP_CO2_AUTOCAL_SW);
	}
	rtc_state = 1;
	i = 0;
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
		while(tapdev_init())	//初始化ENC28J60错误
		{								   
	// 		print("ENC28J60 Init Error!\r\n");
			delay_ms(100);
		}
	}
// 	print("ENC28J60 Init Done!\r\n");
//	watchdog();  
	
	Lcd_Show_String(3, 0, DISP_NOR, (uint8 *)"Net is Done");
	print("CO2_NET\n\r");
 	Lcd_Show_String(4, 0, DISP_NOR, (uint8 *)"Done");
	delay_ms(100);
	watchdog(); 
	
	ctest[0] = read_eeprom(EEP_RESTART_NUM);

	if(ctest[0] == 0xff)
	{
		ctest[0] = 1;
		AT24CXX_WriteOneByte(EEP_RESTART_NUM, ctest[0]);
	}
	else
	{
		AT24CXX_WriteOneByte(EEP_RESTART_NUM, ctest[0] + 1);
	}
	
	
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PM25))  
		xTaskCreate( vNETTask, ( signed portCHAR * ) "NET",  configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 1 , NULL );
  
  xTaskCreate( vMSTP_TASK, ( signed portCHAR * ) "MSTP", configMINIMAL_STACK_SIZE + 512  , NULL, tskIDLE_PRIORITY + 6, NULL );
 	xTaskCreate( vCOMMTask, ( signed portCHAR * ) "COMM", configMINIMAL_STACK_SIZE + 128, NULL, tskIDLE_PRIORITY + 6, NULL );

	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
	{
		xTaskCreate( Co2_task,   ( signed portCHAR * ) "Co2Task", configMINIMAL_STACK_SIZE+128, NULL, tskIDLE_PRIORITY + 6, NULL);
		xTaskCreate( Alarm_task,   ( signed portCHAR * ) "AlarmTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3,  NULL);
//		vStartScanTask(tskIDLE_PRIORITY + 1);
	}
	if(PRODUCT_ID == STM32_PM25)
	{
		xTaskCreate( PM25_task, ( signed portCHAR * ) "PM25Task", configMINIMAL_STACK_SIZE+25, NULL, tskIDLE_PRIORITY + 6, NULL);
	}
	
	if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) )
		xTaskCreate(vUpdate_Pressure_Task, (signed portCHAR *)"Update_Pressure_Task", configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 4, NULL);
	else
		xTaskCreate(vUpdate_Temperature_Task, (signed portCHAR *)"Update_Temperature_Task", configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 3, NULL);
	 
	xTaskCreate(vStartPIDTask, (signed portCHAR *)"vStartPIDTask", configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 3, NULL);
 
 	xTaskCreate( vFlashTask, ( signed portCHAR * ) "FLASH", configMINIMAL_STACK_SIZE + 1500, NULL, tskIDLE_PRIORITY + 5, NULL );
  
    xTaskCreate( vOutPutTask		,( signed portCHAR * ) "OutPut" , configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL );
 
    xTaskCreate( vKEYTask, ( signed portCHAR * ) "KEY", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL );
	//#if defined (COLOR_SCREEN)
	if(isColorScreen == true)
   xTaskCreate( vLCDtask, ( signed portCHAR * ) "LCD", configMINIMAL_STACK_SIZE+512, NULL, tskIDLE_PRIORITY + 5, NULL );
	//#endif
    vStartMenuTask(tskIDLE_PRIORITY + 3);
  	
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
	
	for(;;)
	{
		
		//lcdCounter++;
		if(CurrentState.Index == MenuIdle)
		{
			if(isLcdFirstTime)
			{
				isLcdFirstTime = false;
				Lcd_Full_Screen(0);
			}
//			switch(screenArea1)
//			{
//				case SCREEN_AREA_TEMP:
//					disp_icon(55, 55, sunicon, 10, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//				break;
//				
//				case SCREEN_AREA_HUMI:
//					disp_icon(55, 55, moonicon, 10, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//				break;
//				
////				if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
////				{
//					case SCREEN_AREA_CO2:
//						disp_icon(55, 55, athome, 10, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//					break;
//				//}
//			}
//			switch(screenArea2)
//			{
//				case SCREEN_AREA_TEMP:
//					disp_icon(55, 55, sunicon, 10+HUM_POS, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//				break;
//				
//				case SCREEN_AREA_HUMI:
//					disp_icon(55, 55, moonicon, 10+HUM_POS, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//				break;
//				
////				case SCREEN_AREA_PM25:
////					disp_icon(55, 55, pm25icon, 10+HUM_POS, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
////				break;
//				
//				
////				if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
////				{
////					case SCREEN_AREA_CO2:
////						disp_icon(55, 55, athome, 10+HUM_POS, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
////					break;
////				}
//			}
//			if((enableScroll==false)&& ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) ))
//			{
//				switch(screenArea3)
//				{
//					case SCREEN_AREA_TEMP:
//						disp_icon(55, 55, sunicon, 90+HUM_POS, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//					break;
//					
//					case SCREEN_AREA_HUMI:
//						disp_icon(55, 55, moonicon, 90+HUM_POS, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//					break;
//					
//					case SCREEN_AREA_CO2:
//							disp_icon(55, 55, athome, 170, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//					break;
//				}
//			}
//			else
//				disp_str(FORM15X30, 33+CO2_POS,THIRD_CH_POS+CO2_POSY_OFFSET*9,"CO2:",TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
			if(screenArea1 == SCREEN_AREA_TEMP)
			{
				if(isFirstLineChange)
				{
					for(i = 0; i<7; i++)
					{
						disp_ch(0,THERM_METER_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
					}
					disp_icon(14, 14, degree_o, 30+THERM_METER_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
					if(deg_c_or_f == DEGREE_C)
						Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
					else
						Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
					isFirstLineChange = false;
				}
				//if( lastTemp != HumSensor.temperature_c)
				{
					disp_icon(14, 14, degree_o, 30+THERM_METER_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
					if(deg_c_or_f == DEGREE_C)
						Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
					else
						Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
					if(Run_Timer>FIRST_TIME)
					{
						if(deg_c_or_f == DEGREE_C)
							lastTemp = HumSensor.temperature_c;
						else
							lastTemp = HumSensor.temperature_f;
					}
				}
			}
			else if(screenArea1 == SCREEN_AREA_HUMI)
			{
				if(isFirstLineChange)
				{
					disp_ch(0,30+THERM_METER_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
					for(i = 0; i<7; i++)
					{
						disp_ch(0,THERM_METER_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
					}
					isFirstLineChange = false;
				}
				//if(lastHumi != HumSensor.humidity)
				{
					
					Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
					if(Run_Timer>FIRST_TIME)
						lastHumi = HumSensor.humidity;
				}
			}
			else if(screenArea1 == SCREEN_AREA_CO2)
			{
				if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485)||(PRODUCT_ID == STM32_PM25) )
				{
					if(isFirstLineChange)
					{
						disp_ch(0,30+THERM_METER_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						for(i = 0; i<7; i++)
						{
							disp_ch(0,THERM_METER_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						}
						Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, var[CHANNEL_CO2].value, TOP_AREA_DISP_UNIT_C);
						//Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, pm25_sensor.AQI, TOP_AREA_DISP_UNIT_C);
						isFirstLineChange = false;
					}
					//if( var[CHANNEL_CO2].value != lastCO2)
					{
						Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, var[CHANNEL_CO2].value, TOP_AREA_DISP_UNIT_C);
						//Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, pm25_sensor.AQI, TOP_AREA_DISP_UNIT_C);
						if(Run_Timer>FIRST_TIME)
							lastCO2 = var[CHANNEL_CO2].value;
					}
				}
			}
			else if(screenArea1 == SCREEN_AREA_PM25)
			{
				if(PRODUCT_ID == STM32_PM25)
				{
					Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, pm25_sensor.AQI, TOP_AREA_DISP_UNIT_C);
				}
			}
			else if(screenArea1 == SCREEN_AREA_PRESSURE)
			{
				Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, 0, TOP_AREA_DISP_UNIT_C);
			}
			
			if(screenArea2 == SCREEN_AREA_TEMP)
			{
				if(isSecondLineChange)
				{
					for(i = 0; i<7; i++)
					{
						disp_ch(0,HUM_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
					}
					disp_icon(14, 14, degree_o, 30+HUM_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
					if(deg_c_or_f == DEGREE_C)
						Top_area_display(TOP_AREA_DISP_ITEM_HUM, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
					else
						Top_area_display(TOP_AREA_DISP_ITEM_HUM, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
					isSecondLineChange = false;
				}
				//if( lastTemp != HumSensor.temperature_c)
				{
					
					disp_icon(14, 14, degree_o, 30+HUM_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
					if(deg_c_or_f == DEGREE_C)
						Top_area_display(TOP_AREA_DISP_ITEM_HUM, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
					else
						Top_area_display(TOP_AREA_DISP_ITEM_HUM, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
					if(Run_Timer>FIRST_TIME)
						lastTemp = HumSensor.temperature_c;
				}
			}
			else if(screenArea2 == SCREEN_AREA_HUMI)
			{
				if(isSecondLineChange)
				{
					disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
					for(i = 0; i<7; i++)
					{
						disp_ch(0,HUM_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
					}
					Top_area_display(TOP_AREA_DISP_ITEM_HUM, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
					isSecondLineChange = false;
				}
				//if(lastHumi != HumSensor.humidity)
				{
					//disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
					
					Top_area_display(TOP_AREA_DISP_ITEM_HUM, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
					if(Run_Timer>FIRST_TIME)
						lastHumi = HumSensor.humidity;
				}
			}
			else if(screenArea2 == SCREEN_AREA_CO2)
			{
				if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485)||(PRODUCT_ID == STM32_PM25) )
				{
					if(isSecondLineChange)
					{
						disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						for(i = 0; i<7; i++)
						{
							disp_ch(0,HUM_POS,0+40*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						}
						Top_area_display(TOP_AREA_DISP_ITEM_HUM, var[CHANNEL_CO2].value, TOP_AREA_DISP_UNIT_C);
						//Top_area_display(TOP_AREA_DISP_ITEM_HUM, pm25_weight_25, TOP_AREA_DISP_UNIT_C);
						isSecondLineChange = false;
					}
					//if( var[CHANNEL_CO2].value != lastCO2)
					{
						//disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						
						Top_area_display(TOP_AREA_DISP_ITEM_HUM, var[CHANNEL_CO2].value, TOP_AREA_DISP_UNIT_C);
						//Top_area_display(TOP_AREA_DISP_ITEM_HUM, pm25_weight_25, TOP_AREA_DISP_UNIT_C);
						if(Run_Timer>FIRST_TIME)
							//lastCO2 = var[CHANNEL_CO2].value;
							lastCO2 = pm25_weight_25;
					}
				}
//				else if( PRODUCT_ID == STM32_PM25)
//				{
//					Top_area_display(TOP_AREA_DISP_ITEM_HUM, pm25_weight_25, TOP_AREA_DISP_UNIT_C);
//					//Top_area_display(TOP_AREA_DISP_ITEM_HUM, 25, TOP_AREA_DISP_UNIT_C);
//				}
			}
			else if(screenArea2 == SCREEN_AREA_PM25)
			{
				if(PRODUCT_ID == STM32_PM25)
					Top_area_display(TOP_AREA_DISP_ITEM_HUM, pm25_weight_25, TOP_AREA_DISP_UNIT_C);
			}
			else if(screenArea2 == SCREEN_AREA_PRESSURE)
			{
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
						Top_area_display(TOP_AREA_DISP_ITEM_HUM, 0, TOP_AREA_DISP_UNIT_C);
				}
			}
			
			if(screenArea3 == SCREEN_AREA_TEMP)
			{
				if(isThirdLineChange)
				{
					for(i=0;i<12;i++)
					{
						disp_ch(FORM15X30, 33+CO2_POS,0+20*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
					}
					if(deg_c_or_f == DEGREE_C)
						Top_area_display(TOP_AREA_DISP_ITEM_CO2, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
					else
						Top_area_display(TOP_AREA_DISP_ITEM_CO2, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
					isThirdLineChange = false;
				}
				//disp_icon(14, 14, degree_o, 30+HUM_POS ,UNIT_POS+20,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
				//if( lastTemp != HumSensor.temperature_c)
				{
					
					if(deg_c_or_f == DEGREE_C)
						Top_area_display(TOP_AREA_DISP_ITEM_CO2, HumSensor.temperature_c, TOP_AREA_DISP_UNIT_C);
					else
						Top_area_display(TOP_AREA_DISP_ITEM_CO2, HumSensor.temperature_f, TOP_AREA_DISP_UNIT_F);
					if(Run_Timer>FIRST_TIME)
						lastTemp = HumSensor.temperature_c;
				}
			}
			else if(screenArea3 == SCREEN_AREA_HUMI)
			{
				if(isThirdLineChange)
				{
					for(i=0;i<12;i++)
					{
						disp_ch(FORM15X30, 33+CO2_POS,0+20*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
					}
					Top_area_display(TOP_AREA_DISP_ITEM_CO2, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
					isThirdLineChange = false;
				}
				//disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR
				//if(lastHumi != HumSensor.humidity)
				{
					
					Top_area_display(TOP_AREA_DISP_ITEM_CO2, HumSensor.humidity, TOP_AREA_DISP_UNIT_C);
					if(Run_Timer>FIRST_TIME)
						lastHumi = HumSensor.humidity;
				}
			}
			else if(screenArea3 == SCREEN_AREA_CO2)
			{
				if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
				{
					if(isThirdLineChange)
					{
						for(i=0;i<12;i++)
						{
							disp_ch(FORM15X30, 33+CO2_POS,0+20*i,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
						}
						if(!enableScroll)
							disp_icon(55, 55, athome, 170, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
						Top_area_display(TOP_AREA_DISP_ITEM_CO2, var[CHANNEL_CO2].value, TOP_AREA_DISP_UNIT_C);
						isThirdLineChange = false;
					}
					//disp_ch(0,30+HUM_POS,UNIT_POS+20,' ',TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
					//if( var[CHANNEL_CO2].value != lastCO2)
					{
						if(!enableScroll)
							disp_icon(55, 55, athome, 170, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
						Top_area_display(TOP_AREA_DISP_ITEM_CO2, var[CHANNEL_CO2].value, TOP_AREA_DISP_UNIT_C);
						if(Run_Timer>FIRST_TIME)
							lastCO2 = var[CHANNEL_CO2].value;
					}
				}
			}
			else if(screenArea3 == SCREEN_AREA_PRESSURE)
			{
				Top_area_display(TOP_AREA_DISP_ITEM_CO2, 0, TOP_AREA_DISP_UNIT_C);
			}
//			clear_line(1);
//			clear_lines();
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
 
void vCOMMTask(void *pvParameters )
{
	modbus_init(); 
	reply_done =receive_delay_time;
	print("COMM Task\r\n");
	delay_ms(100);
	
	for( ;; )
	{
		
		if (dealwithTag)
		{  
		 dealwithTag--;
		  if(dealwithTag == 1)//&& !Serial_Master )	
			dealwithData();
		}
		if(serial_receive_timeout_count>0)  
		{
				serial_receive_timeout_count -- ; 
				if(serial_receive_timeout_count == 0)
				{
					serial_restart();
				}
		}
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
		watchdog();
		vTaskDelay(10 / portTICK_RATE_MS);
		
	}
	
}

 


 
 
void vFlashTask( void *pvParameters )
{ 
	uint8 i;
	modbus.write_ghost_system = 0;
	print("Flash Task\r\n");
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
//		stack_detect(&test[6]);
		
		vTaskDelay(1000 / portTICK_RATE_MS);
	}	
}
  










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
 
	AIS = MAX_INS + 1;
	AOS = MAX_AOS + 1;
	BOS = 0;
	AVS = MAX_AVS + 1;
  
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
	print("MSTP Task\r\n");
	delay_ms(100);
	
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
//	while(tapdev_init())	//初始化ENC28J60错误
//	{								   
////	 	print("ENC28J60 Init Error!\r\n");
////		delay_ms(100);
//		;
//	};		
	//print("ENC28J60 Init OK!\r\n");
//	uip_init();							//uIP初始化
//	
//	uip_ipaddr(ipaddr, 192, 168, 0, 163);	//设置本地设置IP地址
//	uip_sethostaddr(ipaddr);					    
//	uip_ipaddr(ipaddr, 192, 168, 0, 4); 	//设置网关IP地址(其实就是你路由器的IP地址)
//	uip_setdraddr(ipaddr);						 
//	uip_ipaddr(ipaddr, 255, 255, 255, 0);	//设置网络掩码
//	uip_setnetmask(ipaddr);

//	print("IP:192.168.0.163\r\n"); 
//	print("MASK:255.255.255.0\r\n"); 
//	print("GATEWAY:192.168.0.4\r\n"); 	
//	
//	uip_listen(HTONS(1200));			//监听1200端口,用于TCP Server
//	uip_listen(HTONS(80));				//监听80端口,用于Web Server
//	tcp_client_reconnect();	   		    //尝试连接到TCP Server端,用于TCP Client
//	print("N e t Task\r\n");
	delay_ms(100);
	
  for( ;; )
	{
		watchdog();
		uip_polling();	//处理uip事件，必须插入到用户程序的循环体中 
		
		if((IP_Change == 1)||(update == 1))
		{
			count++ ;
			if(count == 10)
			{
				count = 0 ;
				IP_Change = 0 ;	
//				//if(!tapdev_init()) print("Init fail\n\r");				
//				while(tapdev_init())	//初始化ENC28J60错误
//				{								   
//				//	print("ENC28J60 Init Error!\r\n");
//				delay_ms(50);
//				};	
				if(update == 1)
				{	
					Lcd_Full_Screen(0);
					Lcd_Show_String(1, 6, 0, (uint8 *)"Updating...");
					Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
				}
				else
				{
					Lcd_Full_Screen(0);
					Lcd_Show_String(1, 6, 0, (uint8 *)"Restarting...");
					Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
				}
				
				SoftReset();
			}
			
		}
//		stack_detect(&test[0]);
		
//		if(tcp_server_tsta != tcp_server_sta)		//TCP Server状态改变
//		{															 
//			if(tcp_server_sta & (1 << 7))
//				print("TCP Server Connected   \r\n");
//			else
//				print("TCP Server Disconnected\r\n"); 
//			
// 			if(tcp_server_sta & (1 << 6))			//收到新数据
//			{
//   			print("TCP Server RX:%s\r\n", tcp_server_databuf);//打印数据
//				tcp_server_sta &= ~(1 << 6);		//标记数据已经被处理	
//			}
//			tcp_server_tsta = tcp_server_sta;
//		}
//		
//		if(global_key == KEY_1)						//TCP Server 请求发送数据
//		{
//			global_key = KEY_NON;
//			if(tcp_server_sta & (1 << 7))			//连接还存在
//			{
//				sprint((char*)tcp_server_databuf, "TCP Server OK\r\n");	 
//				tcp_server_sta |= 1 << 5;			//标记有数据需要发送
//			}
//		}
//		
//		if(tcp_client_tsta != tcp_client_sta)		//TCP Client状态改变
//		{
//			if(tcp_client_sta & (1 << 7))
//				print("TCP Client Connected   \r\n");
//			else
//				print("TCP Client Disconnected\r\n");
//			
// 			if(tcp_client_sta & (1 << 6))			//收到新数据
//			{
//    			print("TCP Client RX:%s\r\n", tcp_client_databuf);//打印数据
//				tcp_client_sta &= ~(1 << 6);		//标记数据已经被处理	
//			}
//			tcp_client_tsta = tcp_client_sta;
//		}
//		
//		if(global_key == KEY_2)						//TCP Client 请求发送数据
//		{
//			global_key = KEY_NON;
//			if(tcp_client_sta & (1 << 7))			//连接还存在
//			{
//				sprint((char*)tcp_client_databuf, "TCP Client OK\r\n");	 
//				tcp_client_sta |= 1 << 5;			//标记有数据需要发送
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



//uip事件处理函数
//必须将该函数插入用户主循环,循环调用.
void uip_polling(void)
{
	u8 i;
	static struct timer periodic_timer, arp_timer;
	static u8 timer_ok = 0;	 
	if(timer_ok == 0)		//仅初始化一次
	{
		timer_ok = 1;
		timer_set(&periodic_timer, CLOCK_SECOND / 2); 	//创建1个0.5秒的定时器 
		timer_set(&arp_timer, CLOCK_SECOND * 10);	   	//创建1个10秒的定时器 
	}
	
	uip_len = tapdev_read();							//从网络设备读取一个IP包,得到数据长度.uip_len在uip.c中定义
	if(uip_len > 0)							 			//有数据
	{   
		//处理IP数据包(只有校验通过的IP包才会被接收) 
		if(BUF->type == htons(UIP_ETHTYPE_IP))			//是否是IP包? 
		{
			uip_arp_ipin();								//去除以太网头结构，更新ARP表
			uip_input();   								//IP包处理			
			//当上面的函数执行后，如果需要发送数据，则全局变量 uip_len > 0
			//需要发送的数据在uip_buf, 长度是uip_len  (这是2个全局变量)		    
			if(uip_len > 0)								//需要回应数据
			{
				uip_arp_out();							//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();							//发送数据到以太网
			}
		}
		else if (BUF->type == htons(UIP_ETHTYPE_ARP))	//处理arp报文,是否是ARP请求包?
		{
			uip_arp_arpin();
			
 			//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len(这是2个全局变量)
 			if(uip_len > 0)
				tapdev_send();							//需要发送数据,则通过tapdev_send发送	 
		}
	}
	else if(timer_expired(&periodic_timer))				//0.5秒定时器超时
	{
		timer_reset(&periodic_timer);					//复位0.5秒定时器 
		
		//轮流处理每个TCP连接, UIP_CONNS缺省是40个  
		for(i = 0; i < UIP_CONNS; i++)
		{
			 uip_periodic(i);							//处理TCP通信事件
			
	 		//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
	 		if(uip_len > 0)
			{
				uip_arp_out();							//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();							//发送数据到以太网
			}
		}
		
#if UIP_UDP	//UIP_UDP 
		//轮流处理每个UDP连接, UIP_UDP_CONNS缺省是10个
		for(i = 0; i < UIP_UDP_CONNS; i++)
		{
			uip_udp_periodic(i);						//处理UDP通信事件
			
	 		//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
			if(uip_len > 0)
			{
				uip_arp_out();							//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();							//发送数据到以太网
			}
		}
#endif 
		//每隔10秒调用1次ARP定时器函数 用于定期ARP处理,ARP表10秒更新一次，旧的条目会被抛弃
		if(timer_expired(&arp_timer))
		{
			timer_reset(&arp_timer);
			uip_arp_timer();
		}
	}
}


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
		
 		if ((PRODUCT_ID == STM32_HUM_RS485) ||(PRODUCT_ID == STM32_HUM_NET))
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
					temp[2] = 0 ;
					temp[3] = 183 ;
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
	
	if(isColorScreen == true)
	{
//		screenArea1 = read_eeprom(EEP_SCREEN_AREA_1);
//		if(screenArea1 == 0xff)
//		{
//			if(PRODUCT_ID == STM32_PM25)
//				screenArea1 = SCREEN_AREA_PM25;
//			else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
//				screenArea1 = SCREEN_AREA_PRESSURE;
//			else
//				screenArea1 = SCREEN_AREA_TEMP;
//			write_eeprom(EEP_SCREEN_AREA_1,screenArea1); 
//		}
//		
//		screenArea2 = read_eeprom(EEP_SCREEN_AREA_2);
//		if(screenArea2 == 0xff)
//		{
//			if(PRODUCT_ID == STM32_PM25)
//				screenArea1 = SCREEN_AREA_PM25;
//			else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
//				screenArea2 = SCREEN_AREA_PRESSURE;
//			else
//				screenArea2 = SCREEN_AREA_HUMI;
//			write_eeprom(EEP_SCREEN_AREA_2,screenArea2); 
//		}
//		
//		screenArea3 = read_eeprom(EEP_SCREEN_AREA_3);
//		if(screenArea3 == 0xff)
//		{
//			if((PRODUCT_ID == STM32_PM25)||(PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
//				screenArea3 = SCREEN_AREA_NONE;
//			else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
//				screenArea3 = SCREEN_AREA_PRESSURE;
//			else
//				screenArea3 = 2;
//			write_eeprom(EEP_SCREEN_AREA_3,screenArea3); 
//		}
		screenArea1 = read_eeprom(EEP_SCREEN_AREA_1);
		if(screenArea1 == 0xff)
		{
			screenArea1 = SCREEN_AREA_TEMP;
			write_eeprom(EEP_SCREEN_AREA_1,screenArea1); 
		}
		if(PRODUCT_ID == STM32_PM25)
				screenArea1 = SCREEN_AREA_PM25;
		else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				screenArea1 = SCREEN_AREA_PRESSURE;
		
		
		screenArea2 = read_eeprom(EEP_SCREEN_AREA_2);
		if(screenArea2 == 0xff)
		{
			screenArea2 = SCREEN_AREA_HUMI;
			write_eeprom(EEP_SCREEN_AREA_2,screenArea2); 
		}
		if(PRODUCT_ID == STM32_PM25)
				screenArea2 = SCREEN_AREA_PM25;
		else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				screenArea2 = SCREEN_AREA_PRESSURE;
		
		
		screenArea3 = read_eeprom(EEP_SCREEN_AREA_3);
		if(screenArea3 == 0xff)
		{
			screenArea3 = 2;
			write_eeprom(EEP_SCREEN_AREA_3,screenArea3); 
		}
		if((PRODUCT_ID == STM32_PM25)||(PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
				screenArea3 = SCREEN_AREA_NONE;
		else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				screenArea3 = SCREEN_AREA_PRESSURE;
				
		
		enableScroll = read_eeprom(EEP_ENABLE_SCROLL);
		if(enableScroll > 1)
		{
			if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
				enableScroll = false;
			else
				enableScroll = true;
			write_eeprom(EEP_ENABLE_SCROLL, enableScroll);
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


