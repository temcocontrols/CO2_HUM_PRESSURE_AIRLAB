#include "config.h"
void co2_init(void);
void co2_reset(void);
uint8 old_sensor_check = 0;
uint8 co2_autocal_disable;
extern uint8 update_flag;
extern uint8 mhz19_cal_h;
extern uint8 mhz19_cal_l;
uint16 test[20];
uint16 co2_asc;
//#define Co2STACK_SIZE	portMINIMAL_STACK_SIZE//1024
//#define	AlarmSTACK_SIZE	portMINIMAL_STACK_SIZE//512 

//U8_T subnet_idle;
//extern xSemaphoreHandle sem_subnet_tx;
//xQueueHandle qSubSerial;
uint8 comm_good;

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

U8_T internal_co2_module_type = 1;//MAYBE_OGM200;

U8_T scd30_co2_cmd_status = SCD30_CMD_CONTINUE_MEASUREMENT;

U8_T const  get_co2_command_OGM200[4] = {0x02, 0xff, 0x41, 0x03};
U8_T const  get_co2_command_TEMCO[4] = {0x02, 0x00, 0xb2, 0x0d}; 
U8_T const  get_co2_command_MHZ19B[9] = {0xFF, 0x01, 0x86, 0x00,0x00,0x00,0x00,0x00,0X79};
U8_T const  disable_co2_autocal_MHZ19B[9] = {0xff,0x01,0x79,0x00,0x00,0x00,0x00,0x00,0x86};
U8_T const  enable_co2_autocal_MHZ19B[9] =  {0xff,0x01,0x79,0xa0,0x00,0x00,0x00,0x00,0xe6};
U8_T const cal_co2_MHZ19B_zero[9] = {0xff,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78};

uint8 const sensirion_co2_cmd_ReadMeasurement[8] = {0x61,0x03,0x00,0x28,0x00,0x06,0x4c,0x60};
uint8 const sensirion_co2_cmd_StartContinueM[8]  = {0x61,0x06,0x00,0x36,0x00,0x00,0x60,0x64};
uint8 const sensirion_co2_cmd_GetDataReady[8]    = {0x61,0x03,0x00,0x27,0x00,0x01,0x3d,0xa1};
uint8 const sensirion_co2_cmd_AutoSelfCalibration_Enable[8] = {0x61,0x06,0x00,0x3A,0x00,0x01,0x61,0xa7};   //Tx:61 06 00 3A 00 01 61 A7
uint8 const sensirion_co2_cmd_AutoSelfCalibration_Disable[8] = {0x61,0x06,0x00,0x3A,0x00,0x00,0xa0,0x67};  //Tx:61 06 00 3A 00 00 A0 67
uint8 const sensirion_co2_cmd_GetASC[8] = {0x61,0x03,0x00,0x39,0x00,0x01,0x5d,0xa7};
uint8 const sensirion_co2_cmd_Reset[8] = {0x61, 0x06, 0x00, 0x34, 0x00, 0x01, 0x00, 0x64};
uint8 sensirion_co2_cmd_ForcedCalibration[8] = {0x61,0x06,0x00,0x39,0x00,0x00,0x00,0x00};

//U8_T cal_co2_MHZ19B_span[9];
uint8 internal_co2_bad_comms = 0;

 uint16 co2_data_temp;
u8 read_co2_ctr = 0;
static u8 co2_sensor_status = 0;

extern float Datasum(uint8 FloatByte1, uint8 FloatByte2, uint8 FloatByte3, uint8 FloatByte4);


extern void watchdog(void);
extern uint8 co2_check_status;
static void request_internal_co2(void)
{
	uint8 i = 0,j;
	U8_T length = 0;
	uint16 co2_temp = 0;

 	//if(internal_co2_exist == FALSE)
 	//	return;

//	if(cSemaphoreTake(sem_subnet_tx, 5) == pdFALSE)
//		return;
//	if(wait_for_subnet_idle(5) == FALSE)
//		return; 

//	if(internal_co2_module_type == MAYBE_OGM200)
//		internal_co2_module_type = MAYBE_TEMCO_CO2;
//	else if(internal_co2_module_type == MAYBE_TEMCO_CO2)
//		internal_co2_module_type = MAYBE_OGM200;
#if 1//OLD_CO2
	if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200))
	{
//		set_sub_serial_baudrate(9600);
		for(i=0;i<5;i++)
		{
			set_subnet_parameters(SEND, 12);
			sub_send_string((U8_T *)get_co2_command_OGM200, 4); 
			
			
			if((length = wait_subnet_response(10)) != 0)
			{
				U8_T i;
				
				for(i = 0; i < length; i++)
				{
					xQueueReceive(qSubSerial, subnet_response_buf+i, 0);
				 
				} 
				receive_internal_co2_ppm(subnet_response_buf); 
				break;
			}
			//watchdog();
			delay_ms(100);
		}
		 
	}
	
	else 
#endif
		if(internal_co2_module_type == MH_Z19B)
	{
		set_sub_serial_baudrate(9600);
		if(update_flag == 3)//disable auto co2 calibration
		{
			//update_flag = 0;
			sub_send_string((U8_T *)disable_co2_autocal_MHZ19B, 9); 
		}
		else if(update_flag == 4)//enable auto co2 calibration
		{
			//update_flag = 0;
			sub_send_string((U8_T *)enable_co2_autocal_MHZ19B, 9); 
		}
    else if(update_flag == 5)//calibrate co2 sensor to 400 ppm
		{
			update_flag = 0;
			sub_send_string((U8_T *)cal_co2_MHZ19B_zero, 9);
			int_co2_str.co2_offset = 0;	
      write_eeprom(EEP_INT_CO2_OFFSET, (uint8)(int_co2_str.co2_offset));
			write_eeprom(EEP_INT_CO2_OFFSET + 1, (uint8)(int_co2_str.co2_offset >> 8));			
		}
				
		else
			sub_send_string((U8_T *)get_co2_command_MHZ19B, 9); 
		
		set_subnet_parameters(SEND, 9);
		if((length = wait_subnet_response(20)) != 0)
		{
			U8_T i;
			
			for(i = 0; i < length; i++)
			{ 
				xQueueReceive(qSubSerial, subnet_response_buf+i, 0);

			} 
			receive_internal_co2_ppm(subnet_response_buf); 
		}
		 
	}
	else if( internal_co2_module_type == SCD30)
	{
		uint8 j,count;
		set_sub_serial_baudrate(19200);
		if(scd30_co2_cmd_status == SCD30_CMD_CONTINUE_MEASUREMENT)
		{
			sub_send_string((U8_T *)sensirion_co2_cmd_StartContinueM, 8);
			set_subnet_parameters(SEND, 8);
		}
		if(scd30_co2_cmd_status == SCD30_CMD_GET_DATA_READY)
		{
			sub_send_string((U8_T *)sensirion_co2_cmd_GetDataReady, 8);
			set_subnet_parameters(SEND, 7);
		}
		if(scd30_co2_cmd_status == SCD30_CMD_READ_ASC)
		{
			sub_send_string((U8_T *)sensirion_co2_cmd_GetASC, 8);
			set_subnet_parameters(SEND, 7);
		}
		if(scd30_co2_cmd_status == SCD30_CMD_READ_MEASUREMENT)
		{
			sub_send_string((U8_T *)sensirion_co2_cmd_ReadMeasurement, 8);
			set_subnet_parameters(SEND, 17);
		}
		if(scd30_co2_cmd_status == SCD30_SOFT_RESET)
		{
			sub_send_string((U8_T *)sensirion_co2_cmd_Reset, 8);
			set_subnet_parameters(SEND, 8);
		}
		if(scd30_co2_cmd_status == SCD30_SET_FRC)
		{
			init_crc16();
			for(j=0;j<6;j++)
			{
				crc16_byte(sensirion_co2_cmd_ForcedCalibration[j]);
				sensirion_co2_cmd_ForcedCalibration[6]= CRChi;
				sensirion_co2_cmd_ForcedCalibration[7]= CRClo;
			}
			sub_send_string((U8_T *)sensirion_co2_cmd_ForcedCalibration, 8);
			set_subnet_parameters(SEND, 8);
		}
		
		count = 0;
		while(((length = sensirion_rev_cnt) != subnet_rec_package_size) && (count++ < 10))
		{
			watchdog();
			delay_ms(20);
		}
		//Test[11] = count;
		if(sensirion_rev_cnt >= 17)
		{Test[12]++;
			old_sensor_check = 1;
			if((subnet_response_buf[0] == 0x61) && (subnet_response_buf[1] == 0x03) && (subnet_response_buf[2] == 0x0c))
			{
				u16 crc_val;
				float temp_f;
				u16 checksum;
				crc_val = crc16(subnet_response_buf,15);Test[13]++;
				checksum = ((uint16)subnet_response_buf[15] << 8)	+ subnet_response_buf[16];
				if(crc_val == checksum)
				{Test[14]++;
					temp_f = Datasum(subnet_response_buf[3],subnet_response_buf[4],subnet_response_buf[5],subnet_response_buf[6]);
					co2_data_temp = (uint16)temp_f;	
					int_co2_str.co2_int	= co2_data_temp  + int_co2_str.co2_offset / 10;		
					Test[15] = co2_data_temp;
					//if(co2_bkcal_onoff == CO2_BKCAL_ON) co2_data_org += co2_bkcal_value;
					scd30_co2_cmd_status = SCD30_CMD_READ_ASC;//CMD_GET_DATA_READY;
					internal_co2_module_type = SCD30;
				}	
			}
		}
		sensirion_rev_cnt = 0;	
		memset(subnet_response_buf,0,18);
		delay_ms(1000);
	}

#if 1
	if(internal_co2_module_type != SCD30)
	{
		if((length == 0)||(old_sensor_check == 0))
		{
			if(co2_check_status < SENSOR_TYPE_ALL)
			{
				co2_check_status++;
			}
			if(co2_check_status == SENSOR_TYPE_ALL)
			{
				co2_check_status = NONE;
				if(internal_co2_module_type == MH_Z19B)
				{	
					internal_co2_module_type = SCD30;
					AT24CXX_WriteOneByte(EEP_CO2_MODULE_TYPE, internal_co2_module_type);
				
				}
				else
				{
					internal_co2_module_type = MH_Z19B;//5;
					AT24CXX_WriteOneByte(EEP_CO2_MODULE_TYPE, internal_co2_module_type);
					
				}
				//internal_co2_module_type = 1;
				co2_reset();
				if(internal_co2_module_type == 1)
				{
					for(i=0;i<10;i++)
					{
						watchdog();
						delay_ms(1000);
					}
				}
			}			
			if(int_co2_str.fail_counter < INTERNAL_CO2_EXIST_CTR)
				int_co2_str.fail_counter++;
			else
			{
				int_co2_str.co2_int = EXCEPTION_PPM;
			}
		}
		else
		{
			co2_check_status = 7;
			int_co2_str.warming_time = FALSE;
			int_co2_str.fail_counter = 0;
			if((co2_data_temp >0) && (co2_data_temp < 10000))
			{
				if( internal_co2_module_type == SCD30)
				{// SCD30 ����Ҫfilter
					int_co2_str.pre_co2_int = co2_data_temp;
				}
				else
				{
					if(Run_Timer > FIRST_TIME)
					{
						if(co2_data_temp > int_co2_str.pre_co2_int)
						{
		//					if(co2_data_temp - int_co2_str.pre_co2_int > 1000)
		//						int_co2_str.pre_co2_int += 10;
		//          else
								int_co2_str.pre_co2_int = Sys_Filter(co2_data_temp,int_co2_str.pre_co2_int,int_co2_filter);
						}
						else	
							int_co2_str.pre_co2_int = Sys_Filter(co2_data_temp,int_co2_str.pre_co2_int,int_co2_filter);
					}
					else
					{
							int_co2_str.pre_co2_int = co2_data_temp;
					}
				}
			}
			if((int_co2_str.pre_co2_int > 10000) || (int_co2_str.pre_co2_int < 0))
			{
				//Test[1] = co2_data_temp;
			}
			if((int_co2_str.pre_co2_int + int_co2_str.co2_offset > 10000) || (int_co2_str.pre_co2_int + int_co2_str.co2_offset < 0))
			{  // if co2 value is wrong, reboot co2 moudle
				
				co2_reset();
			}
			else
				int_co2_str.co2_int = int_co2_str.pre_co2_int + int_co2_str.co2_offset;
			if((int_co2_str.co2_int > 10000) || (int_co2_str.co2_int < 0))
			{
				//Test[2] = int_co2_str.pre_co2_int;
				//Test[3] = int_co2_str.co2_offset;
			}		
		}
	// automatic read the co2 sensor	
		if(length == 0)
		{
			if(read_co2_ctr < 20)
				read_co2_ctr++;
			else
			{
				if((internal_co2_module_type == 1) || (internal_co2_module_type = 0))
					internal_co2_module_type = MH_Z19B;
				else if(internal_co2_module_type == MH_Z19B)
					internal_co2_module_type = SCD30;
	//			if(internal_co2_module_type >= SENSOR_TYPE_ALL)  
	//				internal_co2_module_type = MH_Z19B; 
				if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200) || (internal_co2_module_type == MH_Z19B))
				{
					set_sub_serial_baudrate(9600); 
				}
				else if((internal_co2_module_type == MAYBE_TEMCO_CO2) || (internal_co2_module_type == TEMCO_CO2)
					||(internal_co2_module_type == SCD30))
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
				//AT24CXX_WriteOneByte(EEP_CO2_MODULE_TYPE,internal_co2_module_type); 
			}
			read_co2_ctr = 0;
		}
	}
#endif
//	test[19] = read_co2_ctr;
//	set_sub_serial_baudrate(19200);
	//set_subnet_parameters(SEND, 0);   // 2019.12.19 added sensirion scd30, disable this to test
	
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
	unsigned char i, checksum;

	if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200))
	{
#ifdef ECC_ENABLE
		U8_T ecc_check;
#endif
	  old_sensor_check = 0;
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
	  old_sensor_check = 1;
		co2_data_temp = 1000*(p[4] - '0') + (U16_T)100*(p[5] - '0') + 10*(p[6] - '0') + (p[7] - '0');// ignore the p[8]&p[9]
//		internal_co2_module_type = OGM200;
	}
	else if(internal_co2_module_type == MH_Z19B)
	{
    old_sensor_check = 1;
		if(p[0] != 0xff)
			return;
		if((p[1] == 0x86) || (p[1] == 0x79))
		{
			checksum = 0;
		  if(p[1] == 0x86)
			{
				for(i=1;i<8;i++)
				{

					checksum += p[i];
				}
				checksum = 0xff - checksum;
				checksum += 1;
				if(checksum == p[8])
				{
					co2_data_temp = p[2] * 256 + p[3];
//					ctest[5] = co2_data_temp;
				}
				//old_sensor_check = 1;
			}
			if(p[1] == 0x79) 
			{
				if(update_flag == 4)//auto cal enable
				{					
					co2_autocal_disable = 0;
					update_flag = 0;
				}
				if(update_flag == 3)//auto cal disable
				{					
					co2_autocal_disable = 1;
					update_flag = 0;
				}
				write_eeprom(EEP_CO2_AUTOCAL_SW , co2_autocal_disable);
			}
	  }
		else
			return;
	}
	else if(internal_co2_module_type == SCD30)
	{
		
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
//	internal_co2_module_type = SCD40;  // for test SCD40
//	if((internal_co2_module_type == MH_Z19B)||(SCD30 == internal_co2_module_type)
//		||(SCD40 == internal_co2_module_type))
//	{
//		CO2_RESET_PIN = 0;
//		delay_ms(100);
//		CO2_RESET_PIN = 1;
//	}
//	else
//	{
//		CO2_RESET_PIN = 1;
//		delay_ms(100);
//		CO2_RESET_PIN = 0;
//	}
	CO2_RESET_PIN = 0;
	delay_ms(100);
	CO2_RESET_PIN = 1;
}
void IO_Init(void)
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
	internal_co2_module_type = AT24CXX_ReadOneByte(EEP_CO2_MODULE_TYPE); 
	if(internal_co2_module_type >= SENSOR_TYPE_ALL)
		internal_co2_module_type = MH_Z19B;
	if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200) || (internal_co2_module_type == MH_Z19B))
	{
		set_sub_serial_baudrate(9600); 
	}
	else if((internal_co2_module_type == MAYBE_TEMCO_CO2) || (internal_co2_module_type == TEMCO_CO2)||(internal_co2_module_type == SCD30))
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

//	for(i = 0; i < MAX_EXT_CO2; i++)ds
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
	if(internal_co2_module_type == 1)
		{
			for(i=0;i<10;i++)
			{
				watchdog();
				delay_ms(1000);
			}
		}
}

static void alarm_on(void)
{
	if(alarmEnable)
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
#if OLD_CO2
		index++;
		index %= db_ctr;

		for(i = 0; i < db_ctr; i++)
		{
			if(i == 0)
				alarm_state = int_co2_str.alarm_state;
//			else
//				alarm_state = (alarm_state > ext_co2_str[i - 1].alarm_state) ? alarm_state : ext_co2_str[i - 1].alarm_state;
		}
#endif
		alarm_state = int_co2_str.alarm_state;
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
	static uint16 cnt;
	portTickType xDelayPeriod = (portTickType)2000 / portTICK_RATE_MS;
	sub_net_init();
	
	co2_init();
	int_co2_str.co2_int = EXCEPTION_PPM;
	print("CO2 Task\r\n");
	delay_ms(100);
	task_test.enable[3] = 1;
	while(1) 
	{Test[40] = 3;task_test.count[3]++;
		if(internal_co2_module_type != SCD40) 
		{
			co2_request();
			if(cnt<900)
				cnt++;
			if(cnt>=900)
			{
				if(isColorScreen == false)
					Lcd_Initial();
	//			else
	//				LCDtest();
				update_flag = 7;
				cnt = 0;
			}
		}
		delay_ms(2000);

	}
}

uint8 alarm_status1;
void co2_self_control(void)
{
//	if(self_control_timer)
//		self_control_timer--;
//	else
	if(co2_asc == 0)
	{
		if(aq_value > 250)
			alarm_status1 = (alarm_status1 & 0x01) | 0x08;
		else if(aq_value > 150)
			alarm_status1 = (alarm_status1 & 0x01) | 0x04;
		else
			alarm_status1 = (alarm_status1 & 0x01) | 0x02;
	
	}
	else
	{		
		if(co2_asc >= int_co2_str.alarm_setpoint)
			alarm_status1 = (alarm_status1 & 0x01) | 0x08;
		else if(co2_asc >= int_co2_str.pre_alarm_setpoint)
			alarm_status1 = (alarm_status1 & 0x01) | 0x04;
		else
			alarm_status1 = (alarm_status1 & 0x01) | 0x02;
	}
}

void refresh_led_alarm(void)
{
		static uint8 pre_alarm_status = 0;
		static uint8 comm_ctr = 4;
	
		co2_self_control();
		if(comm_ctr)
		{
			comm_ctr--;
			if(comm_ctr == 0)
			{
				comm_ctr = 4; 
				if(comm_good == TRUE)
					alarm_status1 |= 0x01;
				else
					alarm_status1 &= ~0x01;

				comm_good = FALSE;
			}
		}
		
		if(pre_alarm_status != alarm_status1)
		{		
			refresh_alarm_status_led(alarm_status1);
			pre_alarm_status = alarm_status1;
		}
		refresh_net_status_led(alarm_status1); 
}

void Alarm_task(void *pvParameters )
{
	portTickType xDelayPeriod = (portTickType)1000 / portTICK_RATE_MS;
	print("alarm Task\r\n");
	delay_ms(100);
	LED_Init();task_test.enable[4] = 1;
	while(1)
	{Test[40] = 4;task_test.count[4]++;
 		co2_alarm();
		refresh_led_alarm();
//		taskYIELD();
		update_message_context();
		vTaskDelay(xDelayPeriod); 
//		stack_detect(&test[4]);
	}
}


//-----------AQ-------------------
#define COUNTER_TIME  2
#define	SMAPLE_AVG	  1
#define  NOT_STABLE	0
unsigned char signal_stable_flag = 0;
void AQ_Alarm_task(void *pvParameters )
{
	portTickType xDelayPeriod = (portTickType)200 / portTICK_RATE_MS;
	static unsigned int  start_check = 0;
	static bit first_time = 0;
	static unsigned int xdata  old_value = 0;
	static unsigned int xdata  new_value= 0;
	static unsigned int xdata  core = 0;
	static unsigned int xdata  count = 0;
	static bit detect_aq = 0; 
	
	uint16 count_1s = 0;
	print("AQ_Alarm_task Task\r\n");
	
	delay_ms(100);
	LED_Init();//task_test.enable[4] = 1;
	while(1)
	{
		Test[30]++;
				// AQ sensor
		//-----------------------CALCULATE AQ VALUE ---------------------
	 if(sub_product == 2) // AQ
	 {
		aq_value_temp = get_ad_val(AQ_AD)/4;
		
		if(detect_aq == 0)					 //when we first run this route . we store the value for compare.
		{									 // when the signal is stable , we no need do this route any more.
			if(first_time == 0)
			{
				core += aq_value_temp;
				count++ ;
				if(count > COUNTER_TIME)
				{
					first_time = 1;
					old_value =  core/ COUNTER_TIME;
					core = 0;
					count= 0;
				}	
			}
			else
			{
					 core += aq_value_temp;			 //to detect the signal is stable
					 count++ ;
					 if(count > COUNTER_TIME) 
					 {
							new_value =  core/ COUNTER_TIME;
						if( abs(new_value - old_value)/SMAPLE_AVG < 5)
						{
							signal_stable_flag = 1;
							detect_aq = 1;	
						}
						else
						{
							old_value = new_value;
						}
						core = 0;
						count= 0;

					 }
		
				if(((old_value-aq_value_temp)<8) || ((old_value-aq_value_temp)<8))
				{
					signal_stable_flag = 1;
					detect_aq = 1;
				}
				else 
				old_value =  aq_value_temp;
			}
			 } 		
		if ((signal_stable_flag == NOT_STABLE)&&(start_check<388))   // if the signal isn't stable ,we will wait for 6 minutes.
				start_check++;   //test								   // if the signal is stable , we won't do this route again.
		else
		{
				if(run_check_aq==INIT_AQ)
				{				
					check_Air_Quality();
					run_check_aq=NO_AQ_CHECK;
				}
				else if(run_check_aq==NO_AQ_CHECK)
				{	
					run_check_aq=AQ_CHECK;
				}
				else if(run_check_aq==AQ_CHECK)
				{
					check_Air_Quality();
					run_check_aq=NO_AQ_CHECK;
				}
		} 	
	}		
//-------------------------------------------------		
		
		if(count_1s++ % 5 == 0)  // 1s
			refresh_led_alarm();
		vTaskDelay(xDelayPeriod); 
	}
}




