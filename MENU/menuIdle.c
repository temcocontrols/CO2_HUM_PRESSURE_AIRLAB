#include "config.h"

#define	NODES_POLL_PERIOD	30

uint8 co2_check_status = 0;//0:ini value  1 to 5: read co2 1-5 times  6:read co2 fail 7: read co2 OK   
static uint8 display_around_time_ctr = NODES_POLL_PERIOD;
static uint8 disp_index = 0;
static uint8 warming_state = TRUE;
static uint8 force_refresh = TRUE;
extern uint8 rx_icon;
extern uint8 tx_icon;

uint8_t lcd_i2c_sensor_index = 0;
uint8_t flag_setpoint;
int8_t index_setpoint;
uint8_t max_setpoint;
uint8_t icon_setpoint[3];
uint8_t count_enter_idle;

uint8_t check_idle_setp(void)
{
	uint8 i = 0;
	if((screenArea1 >= SCREEN_AREA_TEMSETP) && (screenArea1 <= SCREEN_AREA_CO2SETP))
	{
		icon_setpoint[0] = screenArea1;
		i++;
	}
	else
		icon_setpoint[0] = 0;
	if((screenArea2 >= SCREEN_AREA_TEMSETP) && (screenArea2 <= SCREEN_AREA_CO2SETP))
	{		
		icon_setpoint[1] = screenArea2;
		i++;
	}
	else
		icon_setpoint[1] = 0;
	if((screenArea3 >= SCREEN_AREA_TEMSETP) && (screenArea3 <= SCREEN_AREA_CO2SETP))
	{
		icon_setpoint[2] = screenArea3;
		i++;		
	}
	else
		icon_setpoint[2] = 0;
	return i;
}

char get_curret_setp(uint8_t index)
{
	uint8_t i;
	if(index < 0)	return -1;
	if(index > 2) return -1;
	for (i = 0; i < 3;i++)
	{
		if(icon_setpoint[i] != 0 && (i == index))
			return i;
			
	} 
	return -1;
}

void Idle_init(void)
{
	//ifndef OLD_CO2  added by chelesa
	uint8 db_ctr;
	db_ctr = 1;
//#endif
	intial_setpoint();
	
	Lcd_Full_Screen(0);
	lcd_i2c_sensor_index = 0;
	exit_request_password();
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
	{
//		#if defined (COLOR_SCREEN)
//		
//		#endif
		if(internal_co2_exist == TRUE) // internal sensor exists 
		{
			warming_state = FALSE;
			#if 1//defined (DOT_MATRIX_SCREEN)
			
			if(isColorScreen == false)
				Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)internal_text);
			#endif
			if(int_co2_str.warming_time == TRUE)
			{
				#if 1//defined (DOT_MATRIX_SCREEN)
				
				if(isColorScreen == false)
				{
				Lcd_Show_String(0, strlen((char *)internal_text) + 2, DISP_INV, (uint8 *)"read CO2");
				print_big_number(EXCEPTION_PPM, 0);
				}
				#endif
				warming_state = TRUE;
			}
			else
			{
				#if 1//defined (DOT_MATRIX_SCREEN)
							
				if(isColorScreen == false)
				{
				print_big_number(EXCEPTION_PPM, 0);
				}
				#endif
			}
			#if 1//defined (DOT_MATRIX_SCREEN)
					
			if(isColorScreen == false)
			{
			Lcd_Show_String(1, 18, DISP_NOR, (uint8 *)ppm_text);
			print_online_status(TRUE);
			}
			#endif
		}
		else // not internal sensor 
		{
//			if(db_ctr == 1)	// there is not sensor exists in the system
			{
				warming_state = FALSE;
				#if 1//defined (DOT_MATRIX_SCREEN)
				
				if(isColorScreen == false)
				{
				Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)internal_text);
				}
				#endif
				co2_int = EXCEPTION_PPM;
				#if 1//defined (DOT_MATRIX_SCREEN)
				
				if(isColorScreen == false)
				{
				print_big_number(co2_int, 0);
				Lcd_Show_String(1, 18, DISP_NOR, (uint8 *)ppm_text);
				print_online_status(FALSE);
				}
				#endif
			}
//			else if(db_ctr >= 2) // there is only one external sensor in the system
//			{
//				warming_state = FALSE;
//				sprintf((char *)text, "%s%u:  ", external_text, (uint16)(scan_db[1].id));
//				Lcd_Show_String(0, 0, DISP_NOR, text); // will be changed in the display routine
//				if(ext_co2_str[0].warming_time == TRUE)
//				{
//					Lcd_Show_String(0, strlen((char *)text), DISP_INV, (uint8 *)warming_text);
//					ext_co2_str[0].co2_int = EXCEPTION_PPM;
//					warming_state = TRUE;
//				}
//				print_big_number(ext_co2_str[0].co2_int, 0);
//				Lcd_Show_String(1, 18, DISP_NOR, (uint8 *)ppm_text);
//				print_online_status(TRUE);
//			}
		}
		disp_index = 0;

		previous_internal_co2_exist = internal_co2_exist;
		pre_db_ctr = db_ctr;
		pre_alarm_level = 0xff;
		#if 1//defined (DOT_MATRIX_SCREEN)
		
		if(isColorScreen == false)
		{
		print_alarm_status(alarm_state);
		}
		#endif
		force_refresh = TRUE;
	} 
	else if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
	{
		
		if(isColorScreen == false)
		{
		Lcd_Show_String(0, 0, DISP_NOR,(uint8 *)"Pressure:");	 
		strcpy((char *)text, (char *)"Unit:"); 
		strcat((char *)text,  (char *)Prs_Unit[Pressure.unit]);
		Lcd_Show_String(4, 11, DISP_NOR, text); 
		}
		Pressure.val_temp = 0;
	}

}

 
void get_data_format(u8 loc,float num,char *s)
{
	u8 i,s_len,s_start,buf_start;
	
	if(loc == 0)
		sprintf(s,"%9.0f",num);
	else if(loc == 1)
		sprintf(s,"%9.1f",num);
	else if(loc == 2)
		sprintf(s,"%9.2f",num);
	else if(loc == 3)
		sprintf(s,"%9.3f",num);
	else if(loc == 4)
		sprintf(s,"%9.4f",num);
	else if(loc == 5)
		sprintf(s,"%9.5f",num);
	else if(loc == 6)
		sprintf(s,"%9.6f",num);
	else if(loc == 7)
		sprintf(s,"%9.0f",num);
	else
		sprintf(s,"%f",num);
	
	for(i=0;i<9;i++)
	{
		if(s[i]!=0x20) break;
	}
	s_len = 9 - i;   					//数据长度
	s_start = i;     					//数据起始位置
	buf_start = i - i / 2; 				//重新排置后的起始位置
	
	for(i=0;i<s_len;i++) 				//数据左移
	{
		s[buf_start + i] = s[s_start + i];
	}
	for(i=buf_start + s_len;i<9;i++ ) 	//补" "
	{
		s[i] = 0x20;
	} 
}
 

void Idle_display(void)
{
	static uint16 previous_co2 = EXCEPTION_PPM;
	u8 temp = 0; 
	float ftemp;
	
	if(isColorScreen == false)
	{
	}
	check_setpoint_count();
}

void intial_setpoint(void)
{
	flag_setpoint = 0;
	index_setpoint = -1;
	count_enter_idle = 0;
	max_setpoint = check_idle_setp();
}
void start_setpoint_count(void)
{
	count_enter_idle = 10;
}

void check_setpoint_count(void)
{
	if(flag_setpoint > 0)
	{
		if(count_enter_idle > 0)
			count_enter_idle--;
		else
		{
			intial_setpoint();			
		}
	}
}

void Idle_keycope(uint16 key_value)
{
	start_back_light(backlight_keep_seconds);
	switch(key_value& KEY_SPEED_MASK)
	{
		case KEY_NON:
			// do nothing
			break;
		case KEY_UP_MASK:
			if(flag_setpoint == 0)
			{
				if(lcd_i2c_sensor_index < 2)	lcd_i2c_sensor_index++;
				else
					lcd_i2c_sensor_index = 0;
			}
			else
			{
				char i;
				start_setpoint_count();
				i = get_curret_setp(index_setpoint);
				if(i >= 0)
				{
					if(icon_setpoint[i] == SCREEN_AREA_TEMSETP)
					{
						PID[0].EEP_SetPoint += 10;
						write_eeprom(EEP_PID1_SETPOINT,PID[0].EEP_SetPoint);
						write_eeprom(EEP_PID1_SETPOINT+1,PID[0].EEP_SetPoint >> 8);
					}
					if(icon_setpoint[i] == SCREEN_AREA_HUMSETP)
					{
						PID[1].EEP_SetPoint += 10;
						write_eeprom(EEP_PID2_SETPOINT,PID[1].EEP_SetPoint);
						write_eeprom(EEP_PID2_SETPOINT+1,PID[1].EEP_SetPoint >> 8);
					}
					if(icon_setpoint[i] == SCREEN_AREA_CO2SETP)
					{
						PID[2].EEP_SetPoint += 10;
						write_eeprom(EEP_PID3_SETPOINT,PID[2].EEP_SetPoint);
						write_eeprom(EEP_PID3_SETPOINT+1,PID[2].EEP_SetPoint >> 8);
					}
				}
			}
			// do nothing
			break;
		case KEY_DOWN_MASK:
			if(flag_setpoint == 0)
			{
				if(lcd_i2c_sensor_index > 0)	lcd_i2c_sensor_index--;
				else
					lcd_i2c_sensor_index = 2;
			}
			else
			{
				char i;
				start_setpoint_count();
				i = get_curret_setp(index_setpoint);
				if(i >= 0)
				{
					if(icon_setpoint[i] == SCREEN_AREA_TEMSETP)
					{
						PID[0].EEP_SetPoint -= 10;
						write_eeprom(EEP_PID1_SETPOINT,PID[0].EEP_SetPoint);
						write_eeprom(EEP_PID1_SETPOINT+1,PID[0].EEP_SetPoint >> 8);
					}
					if(icon_setpoint[i] == SCREEN_AREA_HUMSETP)
					{
						PID[1].EEP_SetPoint -= 10;
						write_eeprom(EEP_PID2_SETPOINT,PID[1].EEP_SetPoint);
						write_eeprom(EEP_PID2_SETPOINT+1,PID[1].EEP_SetPoint >> 8);
					}
					if(icon_setpoint[i] == SCREEN_AREA_CO2SETP)
					{
						PID[2].EEP_SetPoint -= 10;
						write_eeprom(EEP_PID3_SETPOINT,PID[2].EEP_SetPoint);
						write_eeprom(EEP_PID3_SETPOINT+1,PID[2].EEP_SetPoint >> 8);
					}
				}
			}
			break;
		case KEY_LEFT_MASK:
			if(max_setpoint > 0)
			{
				start_setpoint_count();
				flag_setpoint = 1;
				if(index_setpoint < 2)
					index_setpoint++;
				else
					index_setpoint = 0;
			}
			break;
		case KEY_RIGHT_MASK:
			// go into main menu
			flag_setpoint = 0;
			update_menu_state(MenuMain);
			break;
	}
}



