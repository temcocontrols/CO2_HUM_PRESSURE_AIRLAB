#include "config.h"

uint16 test[20];
//#define Co2STACK_SIZE	portMINIMAL_STACK_SIZE//1024
//#define	AlarmSTACK_SIZE	portMINIMAL_STACK_SIZE//512 

//U8_T subnet_idle;
//extern xSemaphoreHandle sem_subnet_tx;
//xQueueHandle qSubSerial;

CO2_STR   int_co2_str;
//CO2_STR   ext_co2_str[MAX_EXT_CO2];

U16_T co2_int = EXCEPTION_PPM;
//U16_T pre_int_co2 = 0;

U8_T internal_co2_exist = TRUE;

U8_T alarm_state = STOP_ALARM;
U8_T alarm_status = OFF;
U8_T pre_alarm_on_time = 2;
U8_T pre_alarm_off_time = 2;
//U16_T previous_co2 = 0;
U8_T pre_alarm_level = 0xff;
U8_T alarm_delay_time = 5;
U8_T alarm_delay_time_ctr;

U8_T slope_check_times = SLOPE_CHECK_TIMES;
bit first_into_steady = TRUE;
U16_T co2_slope_detect_value = 200;
U8_T int_co2_filter = 5;

bit co2_refresh_flag = TRUE;

U8_T slave_id;
U8_T poll_index = 0;

U8_T internal_co2_module_type = MAYBE_OGM200;

U8_T const code get_co2_command_OGM200[4] = {0x02, 0xff, 0x41, 0x03};
U8_T const code get_co2_command_TEMCO[4] = {0x02, 0x00, 0xb2, 0x0d}; 

uint8 internal_co2_bad_comms = 0;

 uint16 co2_data_temp;
u8 read_co2_ctr = 0;
static u8 co2_sensor_status = 0;

int Uart2_putc(int ch )
{ 
	USART_ClearFlag(USART2, USART_FLAG_TC);
	USART_SendData(USART2, (uint8_t) ch);

	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {;}	
     
    return ch;
}
static void request_internal_co2(void)
{
	U8_T length = 0;

 	if(internal_co2_exist == FALSE)
 		return;

//	if(cSemaphoreTake(sem_subnet_tx, 5) == pdFALSE)
//		return;
//	if(wait_for_subnet_idle(5) == FALSE)
//		return; 

//	if(internal_co2_module_type == MAYBE_OGM200)
//		internal_co2_module_type = MAYBE_TEMCO_CO2;
//	else if(internal_co2_module_type == MAYBE_TEMCO_CO2)
//		internal_co2_module_type = MAYBE_OGM200;
	 
	if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200))
	{
//		set_sub_serial_baudrate(9600);
 		sub_send_string((U8_T *)get_co2_command_OGM200, 4); 
		set_subnet_parameters(SEND, 12);
		
		if((length = wait_subnet_response(10)) != 0)
		{
			U8_T i;
			
			for(i = 0; i < length; i++)
			{
				xQueueReceive(qSubSerial, subnet_response_buf+i, 0);
			 
			} 
			receive_internal_co2_ppm(subnet_response_buf); 
		}
		 
	}
	else if((internal_co2_module_type == MAYBE_TEMCO_CO2) || (internal_co2_module_type == TEMCO_CO2))
	{  
//		set_sub_serial_baudrate(19200);
// 		delay_ms(100);
//      sub_send_string((U8_T *)get_co2_command_TEMCO, 4);
	 
// 		set_subnet_parameters(SEND, 23); 
		
		
//		Uart2_putc(0xFF);
//		Uart2_putc(0X03);
//		Uart2_putc(0X00);
//		Uart2_putc(0X6D);
//		Uart2_putc(0X00);
//		Uart2_putc(0X01);
//		Uart2_putc(0X00);
//		Uart2_putc(0X09);
		
   		read_from_slave(REG_CO2_VALUE);
  		length = DEAL_TEMCO_SENSOR();
		set_subnet_parameters(SEND, 0);
	}

//	if(length = wait_subnet_response(10))
//	{
//		U8_T i;
//		for(i = 0; i < length; i++)
//			xQueueReceive(qSubSerial, subnet_response_buf+i, 0);
//			
//		receive_internal_co2_ppm(subnet_response_buf);
//	}
//	else
	if(length == 0)
	{
		if(int_co2_str.fail_counter < INTERNAL_CO2_EXIST_CTR)
			int_co2_str.fail_counter++;
		else
		{
			int_co2_str.co2_int = EXCEPTION_PPM;
			if(current_online[scan_db[0].id / 8] & (1 << (scan_db[0].id % 8)))
			{
				current_online[scan_db[0].id / 8] &= ~(1 << (scan_db[0].id % 8));
				current_online_ctr--;
			}
		}
	}
	else
	{
 		int_co2_str.warming_time = FALSE;
		int_co2_str.fail_counter = 0;
		if((current_online[scan_db[0].id / 8] & (1 << (scan_db[0].id % 8))) == 0x00)
		{
			current_online[scan_db[0].id / 8] |= (1 << (scan_db[0].id % 8));
			current_online_ctr++;
		} 
		if(Run_Timer > FIRST_TIME)
			int_co2_str.pre_co2_int = Sys_Filter(co2_data_temp,int_co2_str.pre_co2_int,int_co2_filter);
		else
			int_co2_str.pre_co2_int = co2_data_temp;
		int_co2_str.co2_int = int_co2_str.pre_co2_int + int_co2_str.co2_offset;
	}
// automatic read the co2 sensor	
	if(length == 0)
	{ 
		if(read_co2_ctr < 20) read_co2_ctr++;
		else
		{
			internal_co2_module_type++;
			if(internal_co2_module_type >= SENSOR_TYPE_ALL)  internal_co2_module_type = 1; 
			if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200))
			{
				set_sub_serial_baudrate(9600); 
			}
			else if((internal_co2_module_type == MAYBE_TEMCO_CO2) || (internal_co2_module_type == TEMCO_CO2))
			{  
				set_sub_serial_baudrate(19200);
			}
			co2_sensor_status = 1;
			read_co2_ctr = 0;
		}
	}
	else
	{
		if(co2_sensor_status)
		{
			co2_sensor_status = 0;
			AT24CXX_WriteOneByte(EEP_CO2_MODULE_TYPE,internal_co2_module_type); 
		}
		read_co2_ctr = 0;
	}
//	test[19] = read_co2_ctr;
//	set_sub_serial_baudrate(19200);
	set_subnet_parameters(SEND, 0);
	
//	cSemaphoreGive(sem_subnet_tx); 
}



 

void co2_request(void)
{
	if((output_auto_manual & 0x04) == 0x04)
	{
		int_co2_str.co2_int = output_manual_value_co2;
		co2_int = output_manual_value_co2;
	}
	else
	{
		output_manual_value_co2 = int_co2_str.co2_int;
//		poll_index++;
//		poll_index %= db_ctr;
//		if(poll_index != 0)  
// 			request_external_co2(poll_index); 
		
   		request_internal_co2();
	}
	var[CHANNEL_CO2]. value = int_co2_str.co2_int;
	
}

void receive_internal_co2_ppm(U8_T *p)
{
//	uint16 int_co2_temp;

	if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200))
	{
#ifdef ECC_ENABLE
		U8_T ecc_check;
#endif
	
		if(p[0] != 0x02) // STX
			return;
	
		if(p[2] != 0x41) // CMD REPLY
			return;
	
		if(p[3] != 0x06) // DATA LENGTH
			return;
		
		if(p[11] != 0x03)// ETX
			return;
	
#ifdef ECC_ENABLE
		ecc_check = p[3]^p[4]^p[5]^p[6]^p[7]^p[8]^p[9];
		if(ecc_check != p[10])
			return;
#endif
	
		co2_data_temp = 1000*(p[4] - '0') + (U16_T)100*(p[5] - '0') + 10*(p[6] - '0') + (p[7] - '0');// ignore the p[8]&p[9]
//		internal_co2_module_type = OGM200;
	}
//	else if((internal_co2_module_type == MAYBE_TEMCO_CO2) || (internal_co2_module_type == TEMCO_CO2))
//	{
//		if(p[0] != 0x23)
//			return;

//		if(p[22] != 0x0a)
//			return;

//		int_co2_temp = 10000*(p[2] - '0') + 1000*(p[3] - '0') + (U16_T)100*(p[4] - '0') + 10*(p[5] - '0') + (p[6] - '0');
//		internal_co2_module_type = TEMCO_CO2;
//	}

//	if(abs(pre_int_co2 - int_co2_temp) < co2_slope_detect_value) // steady ppm
//	{
//		if(slope_check_times)
//		{
//			slope_check_times--;
//			first_into_steady = TRUE;
//		}
//		else
//		{
//			if(first_into_steady == TRUE)
//			{
//				first_into_steady = FALSE;
// 				int_co2_str.warming_time = FALSE;
//			}
//			else
//			{
//				int_co2_temp = ((uint32)pre_int_co2 * int_co2_filter + int_co2_temp) / (int_co2_filter + 1);
//			}
//			pre_int_co2 = int_co2_temp;
// 			int_co2_str.co2_int = int_co2_temp ;//+ int_co2_str.co2_offset;
//		}
//	}
//	else // pulse
//	{
//		slope_check_times = SLOPE_CHECK_TIMES;
//		pre_int_co2 = int_co2_temp;
//	}

//	int_co2_str.fail_counter = 0;
//	if((current_online[scan_db[0].id / 8] & (1 << (scan_db[0].id % 8))) == 0x00)
//	{
//		current_online[scan_db[0].id / 8] |= (1 << (scan_db[0].id % 8));
//		current_online_ctr++;
//	}
}



void co2_reset(void)
{
	CO2_RESET_PIN = 1;
	delay_ms(100);
	CO2_RESET_PIN = 0;
}
static void IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);//PA2/PA3

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD, GPIO_Pin_0| GPIO_Pin_3); 
}
void co2_init(void)
{
	U8_T i;
	IO_Init();
	alarm_delay_time = AT24CXX_ReadOneByte(EEP_ALARM_DELAY_TIME);
	if(alarm_delay_time == 0xff) alarm_delay_time = 5;
	internal_co2_module_type =AT24CXX_ReadOneByte(EEP_CO2_MODULE_TYPE); 
	if(internal_co2_module_type >= SENSOR_TYPE_ALL)
		internal_co2_module_type = OGM200;
	if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200))
	{
		set_sub_serial_baudrate(9600); 
	}
	else if((internal_co2_module_type == MAYBE_TEMCO_CO2) || (internal_co2_module_type == TEMCO_CO2))
	{  
		set_sub_serial_baudrate(19200);
	}
	
	
	// check if the internal co2 sensor is valid
	if(internal_co2_exist == TRUE)
		int_co2_str.warming_time = TRUE;

	int_co2_str.co2_int = EXCEPTION_PPM;
	// Should be read from flash
//	int_co2_str.co2_offset = 0;
	int_co2_str.alarm_state = STOP_ALARM;
//	int_co2_str.alarm_setpoint = DEFAULT_ALARM_SP;
//	int_co2_str.pre_alarm_setpoint = DEFAULT_PRE_ALARM_SP;
	int_co2_str.fail_counter = 0;

//	for(i = 0; i < MAX_EXT_CO2; i++)
//	{
//		ext_co2_str[i].co2_int = EXCEPTION_PPM;
//		ext_co2_str[i].co2_offset = 0;
//		ext_co2_str[i].alarm_state = STOP_ALARM;
//		ext_co2_str[i].alarm_setpoint = EXCEPTION_PPM;
//		ext_co2_str[i].pre_alarm_setpoint = EXCEPTION_PPM;
//		ext_co2_str[i].fail_counter = 0;
//		ext_co2_str[i].warming_time = TRUE;
//	}

//	if((output_auto_manual & 0x04) == 0x04)
//	{
//		co2_int = output_manual_value_co2;
//		int_co2_str.co2_int = output_manual_value_co2;
//	}
	
	int_co2_filter =  read_eeprom(EEP_CO2_FILTER ); 
	int_co2_str.alarm_setpoint = ((uint16)read_eeprom(EEP_INT_ALARM_SETPOINT + 1) << 8) | read_eeprom(EEP_INT_ALARM_SETPOINT);
	int_co2_str.pre_alarm_setpoint = ((uint16)read_eeprom(EEP_INT_PRE_ALARM_SETPOINT + 1) << 8) | read_eeprom(EEP_INT_PRE_ALARM_SETPOINT);
	int_co2_str.co2_offset = (int16)(((uint16)read_eeprom(EEP_INT_CO2_OFFSET + 1) << 8)	| read_eeprom(EEP_INT_CO2_OFFSET));

	
	co2_reset();
}

static void alarm_on(void)
{
  	ALARM_ON();
	alarm_status = 1;
}

static void alarm_off(void)
{
	ALARM_OFF();
	alarm_status = 0;
}

//static U8_T set_external_alarm(U8_T id, U8_T status)
//{
//	U8_T buf[8];
//	U16_T crc_check;
//
//	if(cSemaphoreTake(sem_subnet_tx, 5) == pdFALSE)
//		return FALSE;
//
//	buf[0] = id;
//	buf[1] = WRITE_VARIABLES;
//	buf[2] = HIGH_BYTE(SLAVE_MODBUS_PIC_OUTPUT);
//	buf[3] = LOW_BYTE(SLAVE_MODBUS_PIC_OUTPUT); // start address
//
//	buf[4] = 0;
//	switch(status)	// write output byte
//	{
//		case NO_SENSOR_ALARM:
//		case CONTINUOUS_ALARM:
//			buf[5] = (1 << 3);
//			break;
//		case PRE_ALARM:
//			buf[5] = (1 << 2);
//			break;
//		case STOP_ALARM:
//			buf[5] = (1 << 1);
//			break;
//	}
//
//	crc_check = crc16(buf, 6); // crc16
//	buf[6] = HIGH_BYTE(crc_check);
//	buf[7] = LOW_BYTE(crc_check);
//
//	slave_id = buf[0];
//	sub_send_string(buf, 8);
//	set_subnet_parameters(RECEIVE, 8);
//
//	// check flag
//	return TRUE;
//}

void co2_alarm(void)
{
	static U8_T pre_alarm_ctr = 0;
	static U8_T index = 0;

	if(!(alarm_state & ALARM_MANUAL))
	{
		U8_T alarm_temp, i;
		if(index == 0) // internal sensor
		{
			if(internal_co2_exist == TRUE)
			{
				if(int_co2_str.warming_time == TRUE)
					alarm_temp = STOP_ALARM;
				else if(int_co2_str.co2_int == EXCEPTION_PPM)
					alarm_temp = NO_SENSOR_ALARM;
				else if(int_co2_str.co2_int >= int_co2_str.alarm_setpoint)
					alarm_temp = CONTINUOUS_ALARM;
				else if(int_co2_str.co2_int >= int_co2_str.pre_alarm_setpoint)
					alarm_temp = PRE_ALARM;
				else
					alarm_temp = STOP_ALARM;
			}
			else
				alarm_temp = STOP_ALARM;

			int_co2_str.alarm_state = alarm_temp;
		}
//		else
//		{
//  			U16_T alarm_output_word;
//			if(ext_co2_str[index - 1].co2_int == EXCEPTION_PPM)
//			{
//				if(ext_co2_str[index - 1].warming_time == TRUE)
//					alarm_temp = STOP_ALARM;
//				else
//					alarm_temp = NO_SENSOR_ALARM;
//			}
//			else if(ext_co2_str[index - 1].co2_int >= ext_co2_str[index - 1].alarm_setpoint)
//				alarm_temp = CONTINUOUS_ALARM;
//			else if(ext_co2_str[index - 1].co2_int >= ext_co2_str[index - 1].pre_alarm_setpoint)
//				alarm_temp = PRE_ALARM;
//			else
//				alarm_temp = STOP_ALARM;

////			if(set_external_alarm(scan_db[index].id, alarm_temp) == TRUE)
////				ext_co2_str[index - 1].alarm_state = alarm_temp;

//			switch(alarm_temp)	// write output byte
//			{
//				case NO_SENSOR_ALARM:
//				case CONTINUOUS_ALARM:
//					alarm_output_word = (1 << 3);
//					break;
//				case PRE_ALARM:
//					alarm_output_word = (1 << 2);
//					break;
//				case STOP_ALARM:
//					alarm_output_word = (1 << 1);
//					break;
//			}

//			ext_co2_str[index - 1].alarm_state = alarm_temp;
////			write_parameters_to_nodes(index, SLAVE_MODBUS_PIC_OUTPUT, alarm_output_word);
//		}

		index++;
		index %= db_ctr;

		for(i = 0; i < db_ctr; i++)
		{
			if(i == 0)
				alarm_state = int_co2_str.alarm_state;
//			else
//				alarm_state = (alarm_state > ext_co2_str[i - 1].alarm_state) ? alarm_state : ext_co2_str[i - 1].alarm_state;
		}
	}

	switch(alarm_state & (~ALARM_MANUAL))
	{
		case NO_SENSOR_ALARM:
		case CONTINUOUS_ALARM:
			if(alarm_delay_time_ctr)
				alarm_delay_time_ctr--;
			else
				alarm_on();
			break;
		case PRE_ALARM:
			if(alarm_delay_time_ctr)
			{
				alarm_delay_time_ctr--;
			}
			else
			{
				if(alarm_status == ON)
				{
					pre_alarm_ctr++;
					if(pre_alarm_ctr >= pre_alarm_on_time)
					{
						alarm_off(); // change to off
						pre_alarm_ctr = 0;
					}
				}
				else // OFF
				{
					pre_alarm_ctr++;
					if(pre_alarm_ctr >= pre_alarm_off_time)
					{
						alarm_on();
						pre_alarm_ctr = 0;
					}
				}
			}
			break;
		case STOP_ALARM:
			alarm_off();
			alarm_delay_time_ctr = alarm_delay_time;
			break;
	}
}

void Co2_task(void *pvParameters )
{  
	portTickType xDelayPeriod = (portTickType)2000 / portTICK_RATE_MS;
	sub_net_init();
	
	co2_init();
	int_co2_str.co2_int = EXCEPTION_PPM;
	print("CO2 Task\r\n");
	delay_ms(100);
	
	while(1)
	{
		vTaskDelay(xDelayPeriod);
		co2_request();
//		stack_detect(&test[3]);
		
//		if(db_ctr < 3)
//		{
//			vTaskDelay(200);
//		}
//		else if(db_ctr < 6)
//		{
//			vTaskDelay(100);
//		}
//		else
//		{
//			vTaskDelay(50);
//		}

//		taskYIELD();
	}
}

void Alarm_task(void *pvParameters )
{
	portTickType xDelayPeriod = (portTickType)1000 / portTICK_RATE_MS;
	print("alarm Task\r\n");
	delay_ms(100);
	
	while(1)
	{ 
 		co2_alarm();
//		taskYIELD();
		update_message_context();
		vTaskDelay(xDelayPeriod); 
//		stack_detect(&test[4]);
	}
}


//void vStartCo2Task(unsigned char uxPriority)
//{
//	
//	xTaskCreate(Co2_task,       ( signed portCHAR * ) "Co2Task", configMINIMAL_STACK_SIZE, NULL, uxPriority, NULL);
//	xTaskCreate(Alarm_task,   ( signed portCHAR * ) "AlarmTask", configMINIMAL_STACK_SIZE, NULL, uxPriority,  NULL);
//	 
//}


