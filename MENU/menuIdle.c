#include "config.h"

#define	NODES_POLL_PERIOD	30

static uint8 display_around_time_ctr = NODES_POLL_PERIOD;
static uint8 disp_index = 0;
static uint8 warming_state = TRUE;
static uint8 force_refresh = TRUE;

void Idle_init(void)
{
	Lcd_Full_Screen(0);
	exit_request_password();
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
	{
		if(internal_co2_exist == TRUE) // internal sensor exists 
		{
			warming_state = FALSE;
			Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)internal_text);
			if(int_co2_str.warming_time == TRUE)
			{
				Lcd_Show_String(0, strlen((char *)internal_text) + 2, DISP_INV, (uint8 *)warming_text);
				print_big_number(EXCEPTION_PPM, 0);
				warming_state = TRUE;
			}
			else
				print_big_number(EXCEPTION_PPM, 0);

			Lcd_Show_String(1, 18, DISP_NOR, (uint8 *)ppm_text);
			print_online_status(TRUE);
		}
		else // not internal sensor 
		{
//			if(db_ctr == 1)	// there is not sensor exists in the system
			{
				warming_state = FALSE;
				Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)internal_text);
				co2_int = EXCEPTION_PPM;
				print_big_number(co2_int, 0);
				Lcd_Show_String(1, 18, DISP_NOR, (uint8 *)ppm_text);
				print_online_status(FALSE);
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
		print_alarm_status(alarm_state);
		force_refresh = TRUE;
	} 
	else if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
	{
		Lcd_Show_String(0, 0, DISP_NOR,(uint8 *)"Pressure:");	 
		strcpy((char *)text, (char *)"Unit:"); 
		strcat((char *)text,  (char *)Prs_Unit[Pressure.unit]);
		Lcd_Show_String(4, 11, DISP_NOR, text); 
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
	
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
	{
		if((previous_internal_co2_exist != internal_co2_exist) || (pre_db_ctr != db_ctr))
			Idle_init();

//		if(db_ctr == 1) // only one device in the scan database
		{
			warming_state = FALSE;

			Lcd_Clear_Row(0);
			force_refresh = TRUE;
			Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)internal_text);
			if(internal_co2_exist == TRUE)
			{
				if(int_co2_str.warming_time == TRUE)
				{
					Lcd_Show_String(0, 8, DISP_INV, (uint8 *)warming_text);
					co2_int = EXCEPTION_PPM;
					warming_state = TRUE;
				}
				else
					co2_int = int_co2_str.co2_int;
			}
			else
				co2_int = EXCEPTION_PPM;
		}
/*		
		else if(db_ctr >= 2)
		{
			if(internal_co2_exist == TRUE) // internal sensor exists
			{
				if(display_around_time_ctr)
				{
					display_around_time_ctr--;
				}
				else
				{
					warming_state = FALSE;

					display_around_time_ctr = NODES_POLL_PERIOD;
					disp_index++;
					disp_index = disp_index % db_ctr;

					if(disp_index == 0)
					{
						Lcd_Clear_Row(0);
						force_refresh = TRUE;
						Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)internal_text);
						if(int_co2_str.warming_time == TRUE)
						{
							Lcd_Show_String(0, 8, DISP_INV, (uint8 *)warming_text);
							int_co2_str.co2_int = EXCEPTION_PPM;
							warming_state = TRUE;
						}
					}
					else
					{
						sprintf((char *)text, "%s%u:", external_text, (uint16)scan_db[disp_index].id);
						Lcd_Clear_Row(0);
						force_refresh = TRUE;
						Lcd_Show_String(0, 0, DISP_NOR, text);
						if(ext_co2_str[disp_index - 1].warming_time == TRUE)
						{
							Lcd_Show_String(0, strlen((char *)text) + 2, DISP_INV, (uint8 *)warming_text);
							warming_state = TRUE;
						}
					}
				}

				if(disp_index == 0)
					co2_int = int_co2_str.co2_int;
				else
					co2_int = ext_co2_str[disp_index - 1].co2_int;
			}
			else
			{
				if(db_ctr == 2) // has only one external co2 sensor
				{
					warming_state = FALSE;

					sprintf((char *)text, "%s%u:", external_text, (uint16)scan_db[1].id);
					Lcd_Clear_Row(0);
					force_refresh = TRUE;
					Lcd_Show_String(0, 0, DISP_NOR, text);
					if(ext_co2_str[0].warming_time == TRUE)
					{
						Lcd_Show_String(0, strlen((char *)text) + 2, DISP_INV, (uint8 *)warming_text);
						warming_state = TRUE;
					}

					co2_int = ext_co2_str[0].co2_int;
				}
				else // have more than 2 external sensors
				{
					if(display_around_time_ctr)
					{
						display_around_time_ctr--;
					}
					else
					{
						warming_state = FALSE;

						display_around_time_ctr = NODES_POLL_PERIOD;
						disp_index++;
						disp_index = disp_index % db_ctr;
						if(disp_index == 0)
							disp_index = 1;
		
						sprintf((char *)text, "%s%u:", external_text, (uint16)scan_db[disp_index].id);
						Lcd_Clear_Row(0);
						force_refresh = TRUE;
						Lcd_Show_String(0, 0, DISP_NOR, text);
						if(ext_co2_str[disp_index - 1].warming_time == TRUE)
						{
							Lcd_Show_String(0, strlen((char *)text) + 2, DISP_INV, (uint8 *)warming_text);
							warming_state = TRUE;
						}
					}

//					co2_int = ext_co2_str[disp_index - 1].co2_int;
				}
			}
		}

*/

		if(previous_co2 != co2_int)
		{
			co2_refresh_flag = TRUE;
			previous_co2 = co2_int;
		}

		if(co2_refresh_flag == TRUE) // update alarm status when co2 or setpoints changed.
		{
			co2_refresh_flag = FALSE; 
			print_big_number(co2_int, 0);
		}

		if(warming_state == FALSE)
		{
			
			if(display_state == PIC_NORMAL)
			{	
				update_temperature_display(force_refresh);
				update_humidity_display(force_refresh);
			}
			else if(display_state == PIC_WAIT_OFF_TO_ON)
			{ 
				Lcd_Show_String(0, 6, DISP_NOR, (unsigned char *)"  No Hum Sensor");  
			}
			else
			{
				Lcd_Show_String(0, 6, DISP_NOR, (unsigned char *)" Hum Initial..."); 
			}
			
			
			if(force_refresh == TRUE)
				force_refresh = FALSE;
		}
		print_alarm_status(alarm_state);
	}
	else if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
	{
		Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)"Pressure:"); 
	//	Lcd_Show_String(4, 0, DISP_INV, "                     ");
	//	Lcd_Show_String(4, 0, DISP_INV, time);
	// 	update_temperature_display(force_refresh);
		
		if(Pressure.default_unit == inWC)  			//when the default unit is inwc, it has two decimals		 
		{	
			temp = decimal_num[0][Pressure.unit];
			ftemp  = Pressure.val_temp / 100;
		}
		else if(Pressure.default_unit == Psi)  		//when the default unit is psi, it has one decimals	 
		{
			temp = decimal_num[1][Pressure.unit];
			ftemp = Pressure.val_temp / 10;
	//		temp = decimal_num[0][Pressure.unit];
	//		ftemp  = Pressure.val_temp / 100;
		}  
		get_data_format(temp,ftemp,(char *)text); 
		if(Pressure.out_rng_flag == 1)
		{
			Lcd_Show_Pres(2,0,(uint8 *)"OUT RANGE");   
		}
		else
		{ 
			Lcd_Show_Pres(2,0,text);
			strcpy((char *)text, "Unit:"); 
			strcat((char *)text, (char *)Prs_Unit[Pressure.unit]);
			Lcd_Show_String(4, 11, DISP_NOR, text);
		}
	}
	else if (PRODUCT_ID == STM32_PM25)
	{
		 
		if((pm25_sensor.menu.display&0x01))
		{
			Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)"P M 2.5 :            "); 
			itoa(pm25_sensor.pm25, text, 1);
			Lcd_Show_Pres(2,6,text);
			Lcd_Show_String(3, 16, DISP_NOR, (uint8 *)"ug/m3");
		}
		else if((pm25_sensor.menu.display>>1)&0x01)
		{ 
			Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)"P M 10 :             ");
			itoa(pm25_sensor.pm10, text, 1);
			Lcd_Show_Pres(2,6,text);
			Lcd_Show_String(3, 16, DISP_NOR, (uint8 *)"ug/m3"); 
		}
		else if((pm25_sensor.menu.display>>2)&0x01)
		{
 			Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)"AQI:                 ");
			strcpy((char *)text, (char *)"AQI:  "); 
			strcat((char *)text, (char *)AQI_LEVEL[pm25_sensor.level]);
			Lcd_Show_String(0, 0, DISP_NOR, text);
			itoa(pm25_sensor.AQI, text, 0);
			Lcd_Show_Pres(2,5,text);
		}
	}
	else //((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
	{ 
		if((display_state >= PIC_WAITING1)&& (display_state <= PIC_WAITING_END)) 
		{  
			Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)"Sensor Initial..."); 
		} 
		if(display_state == PIC_WAIT_OFF_TO_ON)
		{
			Lcd_Full_Screen(0);
			Lcd_Show_String(2, 9, DISP_NOR, (uint8 *)"  No  "); 
			Lcd_Show_String(3, 9, DISP_NOR, (uint8 *)"Sensor"); 
		}
		else if(display_state == PIC_NORMAL)
		{
	//		ftemp = (float)HumSensor.humidity/10; 
			Lcd_Show_String(0, 13, DISP_NOR,(uint8 *)"T=     ");	 
			update_temperature_display(force_refresh);
	//		get_data_format(1,ftemp ,(char *)text); 
			itoa(HumSensor.humidity, text, 1);
			Lcd_Show_Pres(2,6,text);
			Lcd_Show_String(3, 18, DISP_NOR, (uint8 *)"%"); 
			/**********show the light value***********/		
			if(humidity_version == LIGHT_SENSOR)
			{
				Lcd_Show_String(0, 0, DISP_NOR,(uint8 *)"L=    "); ;
				Lcd_Show_Data(0, 5, light.val, 0, ALIGN_RIGHT, DISP_NOR);
				Lcd_Show_String(0, 6, DISP_NOR, (uint8 *)"Lux");
			}
			if(dis_hum_info == 1)
			{  
				sprintf((char *)text,"pts:%u sn:%u", HumSensor.counter,HumSensor.sn);
				Lcd_Show_String(4, 0, DISP_NOR, text);   
			}
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
			// do nothing
			break;
		case KEY_DOWN_MASK:
			// do nothing
			break;
		case KEY_LEFT_MASK:
			// do nothing
			break;
		case KEY_RIGHT_MASK:
			// go into main menu
			update_menu_state(MenuMain);
			break;
	}
}



