#include "menu.h"

#define	MenuSTACK_SIZE		portMINIMAL_STACK_SIZE//1024
#define	ScrollingSTACK_SIZE	portMINIMAL_STACK_SIZE//1024
#define	CursorSTACK_SIZE	portMINIMAL_STACK_SIZE//512
xTaskHandle Handle_Menu, Handle_Scrolling, Handle_Cursor;


//extern xQueueHandle qKey;

uint8 text[50]; 
uint8 int_text[21];
//uint16 set_value;
uint8 const   internal_text[] = "  Zone0:";
uint8 const   external_text[] = "Zone";
uint8 const   ppm_text[] = "ppm";
//uint8 const code co2_text[] = "CO2:";
uint8 const   int_space[] = "     ";
//uint8 const code online_text[] = " ONLINE";
//uint8 const code offline_text[] = " OFFLINE";
//uint8 warming_text[13] = "Read CO2     ";

bit menu_system_start = FALSE;
bit in_sub_menu = FALSE;
bit value_change = FALSE;
bit previous_internal_co2_exist;
uint8 pre_db_ctr = 0;


uint8 menu_password = FALSE;
uint8 use_password = FALSE;
uint8 password_buffer[4];
uint8 user_password[4] = {'1', '2', '3', '4'};
uint8 password_index;

uint32 menu_block_timer_start, menu_block_timer_end;
uint8 menu_block_seconds = MENU_BLOCK_SECONDS_DEFAULT;
uint8 backlight_keep_seconds = BACKLIGHT_KEEP_SECONDS_DEFAULT;
xQueueHandle xMutex,IicMutex;
struct _MENU_STATE_ CurrentState;

struct _MENU_STATE_ code StateArray[MenuEnd] = 
{
	{
		MenuIdle,
		Idle_keycope,
		Idle_init,
		Idle_display,
		0,
	},
		{
			MenuMain,
			Co2_Main_keycope,
			Co2_Main_init,
			Co2_Main_display,
			MENU_BLOCK_SECONDS_DEFAULT,
		},
			{
				MenuCo2_Conf,
				Co2_Conf_keycope,
				Co2_Conf_init,
				Co2_Conf_display,
				MENU_BLOCK_SECONDS_DEFAULT,
			},
				{
					MenuCo2Option,
					Co2_Opt_keycope,
					Co2_Opt_init,
					Co2_Opt_display,
					MENU_BLOCK_SECONDS_DEFAULT,
				},
			{
				MenuTemp_Conf,
				Temp_Conf_keycope,
				Temp_Conf_init,
				Temp_Conf_display,
				MENU_BLOCK_SECONDS_DEFAULT,
			},
			{
				MenuHum_Conf,
				Hum_Conf_keycope,
				Hum_Conf_init,
				Hum_Conf_display,
				MENU_BLOCK_SECONDS_DEFAULT,
			},
			{
				MenuMisc,
				Misc_keycope,
				Misc_init,
				Misc_display,
				MENU_BLOCK_SECONDS_DEFAULT,
			},
			{
				MenuPresConf,
				About_keycope,
				About_init,
				About_display,
				MENU_BLOCK_SECONDS_DEFAULT,
			},
};

void update_menu_state(uint8 MenuId)
{
	if(MenuId == MenuIdle)
	{
		start_scrolling();
	}
	else
	{
		stop_scrolling();
		menu_block_timer_start = xTaskGetTickCount();
	}

	memcpy((void *)&CurrentState, (void *)&StateArray[MenuId], sizeof(struct _MENU_STATE_));
	 
	CurrentState.InitAction();
}

void show_system_info(void)
{
	// ip address
	sprintf((char *)text, "IP:   %u.%u.%u.%u", (uint16)modbus.ip_addr[0], (uint16)modbus.ip_addr[1], (uint16)modbus.ip_addr[2], (uint16)modbus.ip_addr[3]);
	Lcd_Show_String(0, 0, DISP_NOR, text);
	// subnet mask address
	sprintf((char *)text, "MASK: %u.%u.%u.%u", (uint16)modbus.mask_addr[0], (uint16)modbus.mask_addr[1], (uint16)modbus.mask_addr[2], (uint16)modbus.mask_addr[3]);
	Lcd_Show_String(1, 0, DISP_NOR, text);
	// tcp port
	sprintf((char *)text, "GATE: %u.%u.%u.%u", (uint16)modbus.gate_addr[0], (uint16)modbus.gate_addr[1], (uint16)modbus.gate_addr[2], (uint16)modbus.gate_addr[3]);
	Lcd_Show_String(2, 0, DISP_NOR, text);
	// tcp port
	sprintf((char *)text, "PORT: %u", ((uint16)modbus.listen_port));
	Lcd_Show_String(3, 0, DISP_NOR, text);
	// MAC address
	sprintf((char *)text, "MAC:%02X:%02X:%02X:%02X:%02X:%02X", (uint16)modbus.mac_addr[0], (uint16)modbus.mac_addr[1], (uint16)modbus.mac_addr[2], (uint16)modbus.mac_addr[3], (uint16)modbus.mac_addr[4], (uint16)modbus.mac_addr[5]);
	Lcd_Show_String(4, 0, DISP_NOR, text);
}

void menu_init(void)
{
//	U16_T key_temp;
	// clear the key events which happened within the menu init routine
//	while(cQueueReceive(qKey, &key_temp, 0) == pdTRUE)
//	{
//		watchdog_reset();
//	}

	menu_system_start = TRUE;
	update_menu_state(MenuIdle);
}
extern void watchdog(void);
void MenuTask(void *pvParameters)
{
	static U8_T refresh_screen_timer = 0;
	
	U16_T key_temp;
	portTickType xDelayPeriod = (portTickType)100 / portTICK_RATE_MS;
//	U8_T i;
//	Lcd_Initial();
//	show_system_info();
//	vTaskDelay(300);
//	update_message_context();
	menu_init();
	dis_hum_info = 0;
	print("Menu Task\r\n");
//	xMutex = xQueueCreateMutex();
	delay_ms(100);
	
	while(1)
	{
		
 		if(xQueueTakeMutexRecursive( xMutex, portMAX_DELAY )==pdPASS)
		{
			if(xQueueReceive(qKey, &key_temp, 20) == pdTRUE)
			{
				CurrentState.KeyCope(key_temp);
				if(CurrentState.BlockTime)
					menu_block_timer_start = xTaskGetTickCount();
				CurrentState.DisplayPeriod();
				 
			}
			
			if(menu_system_start == TRUE)
			{
				refresh_screen_timer++;
				if (refresh_screen_timer >= 10)
				{
					CurrentState.DisplayPeriod();
					refresh_screen_timer = 0;
				}
				
				if(CurrentState.BlockTime)
				{
					menu_block_timer_end = xTaskGetTickCount();
					if((menu_block_timer_end - menu_block_timer_start) >= (CurrentState.BlockTime * SWTIMER_COUNT_SECOND))
						update_menu_state(MenuIdle);
				}
			}
			xQueueGiveMutexRecursive( xMutex );
	//		scrolling_message();
		}
		poll_back_light();
		
		vTaskDelay(xDelayPeriod);//50ms
//		stack_detect(&test[9]);
	}	
}

void exit_request_password(void)
{
	cursor_off();
	menu_password = FALSE;
}

void ScrollingTask(void *pvParameters)
{
	portTickType xDelayPeriod = (portTickType)100 / portTICK_RATE_MS; 
 	update_message_context();
	print("Scrolling Task\r\n");
	delay_ms(100);
	
	while(1)
	{

		if(dis_hum_info == 1)
		{  
			sprintf((char *)text,"pts:%u sn:%u", HumSensor.counter,HumSensor.sn);
			Lcd_Show_String(4, 0, DISP_NOR, text);   
		}
		else if(xQueueTakeMutexRecursive( xMutex, portMAX_DELAY )==pdPASS)
		{
			scrolling_message();
			xQueueGiveMutexRecursive( xMutex );
		}
// 		stack_detect(&test[11]);
//		taskYIELD();

    vTaskDelay(xDelayPeriod);		
	}	
}

void CursorTask(void *pvParameters)
{
	portTickType xDelayPeriod = (portTickType)400 / portTICK_RATE_MS;
	print("Cursor Task\r\n");
	delay_ms(100);
	
	while(1)
	{
		 if(xQueueTakeMutexRecursive( xMutex, portMAX_DELAY )==pdPASS)
		{
			update_cursor(); 
			xQueueGiveMutexRecursive( xMutex );
		}
		vTaskDelay(xDelayPeriod);
//		stack_detect(&test[10]);
//		taskYIELD();
	}
}

void vStartMenuTask(unsigned char uxPriority)
{ 
	xTaskCreate(MenuTask,   ( signed portCHAR * ) "MenuTask"  , configMINIMAL_STACK_SIZE+256 , NULL, uxPriority, NULL);
		
  xTaskCreate(CursorTask, ( signed portCHAR * ) "CursorTask", configMINIMAL_STACK_SIZE, NULL, uxPriority, NULL);
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485)||(PRODUCT_ID == STM32_PM25) )
		xTaskCreate(ScrollingTask, ( signed portCHAR * ) "ScrollingTask", configMINIMAL_STACK_SIZE+128 , NULL, uxPriority, NULL);
	 
}

//void vStartScrollingTask(unsigned char uxPriority)
//{ 
//	 xTaskCreate(ScrollingTask, ( signed portCHAR * ) "ScrollingTask", configMINIMAL_STACK_SIZE , NULL, uxPriority, NULL);
//}

void print(char *p)
{
//	uint8 length,i;
//	vTaskSuspendAll();
//	length  = strlen(p) + 1;
//	TXEN = SEND;
//	for(i=0;i<length;i++)
//	{
//		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
//		USART_SendData(USART1, *(p + i));
//	}
//	TXEN = RECEIVE;	
//	xTaskResumeAll();
	
}
//void my_print(char *p)
//{
//	print(p);
//}


