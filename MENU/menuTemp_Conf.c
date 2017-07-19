#include "config.h"


static uint8 item_index = 0;
static uint8 pre_item_index = 0;

static int16 set_value;

#define MAX_TEMP_ITEMS	4
static uint8 const code item_name[MAX_TEMP_ITEMS][17] = 
{
	"1.Int: ",
	"2.Ext: ",
	"3.Unit: ",
	"4.Sensor disp.: ",
};
 

void Temp_Conf_init(void)
{
	uint8 i;
	Lcd_Full_Screen(0);
	 
	for(i = 0; i < MAX_TEMP_ITEMS; i++)
	{ 
		strcpy((char *)text, (char *)item_name[i]);
		switch(i)
		{
			case 0: // internal temperature
				if(deg_c_or_f == DEGREE_C)
				{	
//					sprintf((char *)text, "%s%u.%u~", item_name[i], internal_temperature_c / 10, internal_temperature_c % 10);
					itoa(internal_temperature_c, int_text, 1);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)"~");
				}
				else
				{	
//					sprintf((char *)text, "%s%u.%u|", item_name[i], internal_temperature_f / 10, internal_temperature_f % 10);
					itoa(internal_temperature_f, int_text, 1);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)"|");
				}
				break;
			case 1:	// external temperature
				if(deg_c_or_f == DEGREE_C)
				{	
//					sprintf((char *)text, "%s%u.%u|", item_name[i], HumSensor.temperature_c / 10, HumSensor.temperature_c % 10);
					itoa( HumSensor.temperature_c, int_text, 1);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)"~");
				}
				else
				{	
//					sprintf((char *)text, "%s%u.%u|", item_name[i], HumSensor.temperature_f / 10, HumSensor.temperature_f % 10);
					itoa( HumSensor.temperature_f, int_text, 1);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)"|");
				} 
				break;
			case 2:	// Unit select
				if(deg_c_or_f == DEGREE_C)
				{	
//					sprintf((char *)text, "%s~", item_name[i]);
					strcat((char *)text, (char *)"~");
				}
				else
				{	
//					sprintf((char *)text, "%s|", item_name[i]);
					strcat((char *)text, (char *)"|");
				}
				break;
			case 3:	// Select to display on normal screen
				if(temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
				{	
//					sprintf((char *)text, "%sInt.", item_name[i]);
					strcat((char *)text, (char *)"Int.");
				}
				else
				{	
//					sprintf((char *)text, "%sExt.", item_name[i]);
					strcat((char *)text, (char *)"Ext.");
				}
				break;
		}
		 
		if(i == item_index)
			Lcd_Show_String(i, 0, DISP_INV, text);
		else
			Lcd_Show_String(i, 0, DISP_NOR, text);
	}
	in_sub_menu = FALSE;
}

void Temp_Conf_display(void)
{    
	if(in_sub_menu == FALSE)
	{
		if(pre_item_index != item_index)
		{
			strcpy((char *)text, (char *)item_name[pre_item_index]);
			switch(pre_item_index)
			{
				case 0:
					if(deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s%u.%u~", item_name[pre_item_index], internal_temperature_c / 10, internal_temperature_c % 10);
						itoa(internal_temperature_c, int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"~");
					}	
					else
					{	
//						sprintf((char *)text, "%s%u.%u|", item_name[pre_item_index], internal_temperature_f / 10, internal_temperature_f % 10);
						itoa(internal_temperature_f, int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 1:
					if(deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s%u.%u~", item_name[pre_item_index], HumSensor.temperature_c / 10, HumSensor.temperature_c % 10);
						itoa(HumSensor.temperature_c , int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"~");
					}
					else
					{	
//						sprintf((char *)text, "%s%u.%u|", item_name[pre_item_index], HumSensor.temperature_f / 10, HumSensor.temperature_f % 10);
						itoa(HumSensor.temperature_f , int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 2:
					if(deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s~", item_name[pre_item_index]);
					 
						strcat((char *)text, (char *)"~");
					}
					else
					{	
//						sprintf((char *)text, "%s|", item_name[pre_item_index]);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 3:
					if(temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
					{	
//						sprintf((char *)text, "%sInt.", item_name[pre_item_index]);
						strcat((char *)text, (char *)"Int.");
					}
					else
					{	
//						sprintf((char *)text, "%sExt.", item_name[pre_item_index]);
						strcat((char *)text, (char *)"Ext.");
					}
					break;
			}
			Lcd_Clear_Row(pre_item_index);
			Lcd_Show_String(pre_item_index, 0, DISP_NOR, text);
			
			strcpy((char *)text, (char *)item_name[item_index]);
			switch(item_index)
			{
				case 0:
					if(deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s%u.%u~", item_name[pre_item_index], internal_temperature_c / 10, internal_temperature_c % 10);
						itoa(internal_temperature_c, int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"~");
					}	
					else
					{	
//						sprintf((char *)text, "%s%u.%u|", item_name[pre_item_index], internal_temperature_f / 10, internal_temperature_f % 10);
						itoa(internal_temperature_f, int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 1:
					if(deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s%u.%u~", item_name[pre_item_index], HumSensor.temperature_c / 10, HumSensor.temperature_c % 10);
						itoa(HumSensor.temperature_c , int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"~");
					}
					else
					{	
//						sprintf((char *)text, "%s%u.%u|", item_name[pre_item_index], HumSensor.temperature_f / 10, HumSensor.temperature_f % 10);
						itoa(HumSensor.temperature_f , int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 2:
					if(deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s~", item_name[pre_item_index]);
					 
						strcat((char *)text, (char *)"~");
					}
					else
					{	
//						sprintf((char *)text, "%s|", item_name[pre_item_index]);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 3:
					if(temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
					{	
//						sprintf((char *)text, "%sInt.", item_name[pre_item_index]);
						strcat((char *)text, (char *)"Int.");
					}
					else
					{	
//						sprintf((char *)text, "%sExt.", item_name[pre_item_index]);
						strcat((char *)text, (char *)"Ext.");
					}
					break;
			}
			Lcd_Clear_Row(item_index);
			Lcd_Show_String(item_index, 0, DISP_INV, text);

			pre_item_index = item_index;
		}
		else
		{
			 
//			// internal temperature
//			if(deg_c_or_f == DEGREE_C)
//			{	
////				sprintf((char *)text, "%u.%u~", internal_temperature_c /10, internal_temperature_c % 10);
//				itoa(internal_temperature_c, int_text, 1);
//				strcpy((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"~ ");
//			}	
//			else
//			{	
////				sprintf((char *)text, "%u.%u|", internal_temperature_f /10, internal_temperature_f % 10);
//				itoa(internal_temperature_f, int_text, 1);
//				strcpy((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"| "); 
//			}
//			Lcd_Show_String(0, 7, DISP_NOR, (uint8 *)"      ");
//			
//			if(item_index == 0)
//				Lcd_Show_String(0, 7, DISP_INV, text);
//			else
//				Lcd_Show_String(0, 7, DISP_NOR, text);


//			// external temperature
//			if(deg_c_or_f == DEGREE_C)
//			{	
////				sprintf((char *)text, "%u.%u~", HumSensor.temperature_c /10, HumSensor.temperature_c % 10);
//				itoa(HumSensor.temperature_c , int_text, 1);
//				strcpy((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"~");
//			}	
//			else
//			{	
////				sprintf((char *)text, "%u.%u|", HumSensor.temperature_f /10, HumSensor.temperature_f % 10);
//				itoa(HumSensor.temperature_f , int_text, 1);
//				strcpy((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"|");
//			}
//			Lcd_Show_String(1, 7, DISP_NOR, (uint8 *)"        "); 
//			if(item_index == 1)
//				Lcd_Show_String(1, 7, DISP_INV, text);
//			else
//				Lcd_Show_String(1, 7, DISP_NOR, text);
//			
//			strcpy((char *)text, (char *)item_name[2]); 
//			if(deg_c_or_f == DEGREE_C)
//			{	 
//				strcat((char *)text, (char *)"~");
//			}
//			else
//			{	
//				strcat((char *)text, (char *)"|");
//			}
//			Lcd_Clear_Row(2);
//			if(item_index == 2)
//				Lcd_Show_String(2, 0, DISP_INV, text);		 
//			else
//				Lcd_Show_String(2, 0, DISP_NOR, text);	

//			strcpy((char *)text, (char *)item_name[3]);
//			if(temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
//			{	
////						sprintf((char *)text, "%sInt.", item_name[pre_item_index]);
//				strcat((char *)text, (char *)"Int.");
//			}
//			else
//			{	
////						sprintf((char *)text, "%sExt.", item_name[pre_item_index]);
//				strcat((char *)text, (char *)"Ext.");
//			}  
//			Lcd_Clear_Row(3);
//			if(item_index == 3)
//				Lcd_Show_String(3, 0, DISP_INV, text);
//			else
//				Lcd_Show_String(3, 0, DISP_NOR, text);
//			
			
		}
	} 
	else
	{
		if(value_change == TRUE)
		{
			switch(item_index)
			{
				case 0:
				case 1:
					if(deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%u.%u~", set_value /10, set_value % 10);
						itoa(set_value , int_text, 1);
						strcpy((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"~");
					}
					else
					{	
//						sprintf((char *)text, "%u.%u|", set_value /10, set_value % 10);
						itoa(set_value , int_text, 1);
						strcpy((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"|");
					}
					Lcd_Show_String(item_index, 7, DISP_NOR, (uint8 *)"      ");
					Lcd_Show_String(item_index, 7, DISP_INV, text);
					break;
				case 2:
					if(set_value == DEGREE_C)
						Lcd_Write_Char(item_index, 8, '~', DISP_INV);
					else
						Lcd_Write_Char(item_index, 8, '|', DISP_INV);
					break;
				case 3:
					if(set_value == INTERNAL_TEMPERATURE_SENSOR)
						Lcd_Show_String(item_index, 16, DISP_INV, (uint8 *)"Int.");
					else
						Lcd_Show_String(item_index, 16, DISP_INV, (uint8 *)"Ext.");
					break;
			}
			
			value_change = FALSE;	
		}
	} 
}

void Temp_Conf_keycope(uint16 key_value)
{
	start_back_light(backlight_keep_seconds);
	switch(key_value & KEY_SPEED_MASK)
	{
		case KEY_NON:
			// do nothing
			break;
		case KEY_LEFT_MASK:
			if(in_sub_menu == FALSE)
			{
				update_menu_state(MenuMain);
			}
			else
			{
				strcpy((char *)text, (char *)item_name[item_index]);
				switch(item_index)
				{
					case 0:
						if(deg_c_or_f == DEGREE_C)
						{	
	//						sprintf((char *)text, "%s%u.%u~", item_name[pre_item_index], internal_temperature_c / 10, internal_temperature_c % 10);
							itoa(internal_temperature_c, int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
						}	
						else
						{	
	//						sprintf((char *)text, "%s%u.%u|", item_name[pre_item_index], internal_temperature_f / 10, internal_temperature_f % 10);
							itoa(internal_temperature_f, int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
						}
						break;
					case 1:
						if(deg_c_or_f == DEGREE_C)
						{	
	//						sprintf((char *)text, "%s%u.%u~", item_name[pre_item_index], HumSensor.temperature_c / 10, HumSensor.temperature_c % 10);
							itoa(HumSensor.temperature_c , int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
						}
						else
						{	
	//						sprintf((char *)text, "%s%u.%u|", item_name[pre_item_index], HumSensor.temperature_f / 10, HumSensor.temperature_f % 10);
							itoa(HumSensor.temperature_f , int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
						}
						break;
					case 2:
						if(deg_c_or_f == DEGREE_C)
						{	
	//						sprintf((char *)text, "%s~", item_name[pre_item_index]);
						 
							strcat((char *)text, (char *)"~");
						}
						else
						{	
	//						sprintf((char *)text, "%s|", item_name[pre_item_index]);
							strcat((char *)text, (char *)"|");
						}
						break;
					case 3:
						if(temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
						{	
	//						sprintf((char *)text, "%sInt.", item_name[pre_item_index]);
							strcat((char *)text, (char *)"Int.");
						}
						else
						{	
	//						sprintf((char *)text, "%sExt.", item_name[pre_item_index]);
							strcat((char *)text, (char *)"Ext.");
						}
						break;
				}
				Lcd_Show_String(item_index, 0, DISP_INV, text);

				in_sub_menu = FALSE;
			}
			break;
		case KEY_RIGHT_MASK:
			if(in_sub_menu == FALSE)
			{
				Lcd_Show_String(item_index, 0, DISP_NOR, (uint8 *)item_name[item_index]);
				switch(item_index)
				{
					case 0:
						if(deg_c_or_f == DEGREE_C)
						{
							set_value = internal_temperature_c;
//							sprintf((char *)text, "%u.%u~", set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
						}
						else
						{
							set_value = internal_temperature_f;
//							sprintf((char *)text, "%u.%u|", set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
						}
						Lcd_Show_String(item_index, 7, DISP_INV, text);
						break;
					case 1:
						if(deg_c_or_f == DEGREE_C)
						{
							set_value = HumSensor.temperature_c;
//							sprintf((char *)text, "%u.%u~", set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
						}
						else
						{
							set_value = HumSensor.temperature_f;
//							sprintf((char *)text, "%u.%u|", set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
						}
						Lcd_Show_String(item_index, 7, DISP_INV, text);
						break;
					case 2:
						set_value = deg_c_or_f;
						if(set_value == DEGREE_C)
							Lcd_Write_Char(item_index, 8, '~', DISP_INV);
						else
							Lcd_Write_Char(item_index, 8, '|', DISP_INV);
						break;
					case 3:
						set_value = temperature_sensor_select;
						if(set_value == INTERNAL_TEMPERATURE_SENSOR)
							Lcd_Show_String(item_index, 16, DISP_INV, (uint8 *)"Int.");
						else
							Lcd_Show_String(item_index, 16, DISP_INV, (uint8 *)"Ext.");
						break;
				}
			
				in_sub_menu = TRUE;
			}
			else
			{
				
				switch(item_index)
				{
					case 0:
						strcpy((char *)text, (char *)item_name[item_index]);
						if(deg_c_or_f == DEGREE_C)
						{
//							sprintf((char *)text, "%s%d.%u~", item_name[item_index], set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
							
							internal_temperature_offset += set_value - internal_temperature_c;
//							start_data_save_timer();

							internal_temperature_c = set_value;
						}
						else
						{
	//						sprintf((char *)text, "%s%d.%u|", item_name[item_index], set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
							internal_temperature_offset += ((int16)set_value - internal_temperature_f) * 5 / 9;
//							start_data_save_timer();

							internal_temperature_f = set_value;
						}
						write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 0, (uint8)(internal_temperature_offset & 0x00ff));
						write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 1, (uint8)(internal_temperature_offset >> 8));
						break;
					case 1:
						strcpy((char *)text, (char *)item_name[item_index]);
						if(deg_c_or_f == DEGREE_C)
						{
		//					sprintf((char *)text, "%s%u.%u~", item_name[item_index], set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
							external_operation_value = set_value;
							external_operation_flag = TEMP_CALIBRATION;

							 
						}
						else
						{
//							sprintf((char *)text, "%s%u.%u|", item_name[item_index], set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
							external_operation_value = ((int16)set_value - 320) * 5 / 9;
							external_operation_flag = TEMP_CALIBRATION;
 
						} 
						break;
					case 2:
						deg_c_or_f = set_value;
					
						if(deg_c_or_f == DEGREE_C)
						{	
			//				sprintf((char *)text, "%u.%u~", internal_temperature_c /10, internal_temperature_c % 10);
							itoa(internal_temperature_c, int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~ ");
						}	
						else
						{	
			//				sprintf((char *)text, "%u.%u|", internal_temperature_f /10, internal_temperature_f % 10);
							itoa(internal_temperature_f, int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"| "); 
						}
						Lcd_Show_String(0, 7, DISP_NOR, (uint8 *)"      ");
						
						if(item_index == 0)
							Lcd_Show_String(0, 7, DISP_INV, text);
						else
							Lcd_Show_String(0, 7, DISP_NOR, text);


						// external temperature
						if(deg_c_or_f == DEGREE_C)
						{	
			//				sprintf((char *)text, "%u.%u~", HumSensor.temperature_c /10, HumSensor.temperature_c % 10);
							itoa(HumSensor.temperature_c , int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
						}	
						else
						{	
			//				sprintf((char *)text, "%u.%u|", HumSensor.temperature_f /10, HumSensor.temperature_f % 10);
							itoa(HumSensor.temperature_f , int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
						}
						Lcd_Show_String(1, 7, DISP_NOR, (uint8 *)"        "); 
						if(item_index == 1)
							Lcd_Show_String(1, 7, DISP_INV, text);
						else
							Lcd_Show_String(1, 7, DISP_NOR, text);
						
						
						
						strcpy((char *)text, (char *)item_name[item_index]);
						
						if(deg_c_or_f == DEGREE_C)
					//		sprintf((char *)text, "%s~", item_name[item_index]);
							strcat((char *)text, (char *)"~");
						else
					//		sprintf((char *)text, "%s|", item_name[item_index]);
							strcat((char *)text, (char *)"|");
						write_eeprom(EEP_DEG_C_OR_F, (uint8)deg_c_or_f);
						
			
						break;
					case 3:
						strcpy((char *)text, (char *)item_name[item_index]);
						temperature_sensor_select = set_value;
						if(temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
			//				sprintf((char *)text, "%sInt.", item_name[item_index]);
							strcat((char *)text, (char *)"Int.");
						else
			//				sprintf((char *)text, "%sExt.", item_name[item_index]);
							strcat((char *)text, (char *)"Ext.");
						write_eeprom(EEP_SENSOR_SELECT, temperature_sensor_select);
						break;
						
				}
				Lcd_Show_String(item_index, 0, DISP_INV, text);

				in_sub_menu = FALSE;
			}
			break;
		case KEY_UP_MASK:
			if(in_sub_menu == FALSE)
			{
				if(item_index)
					item_index--;
				else
					item_index = MAX_TEMP_ITEMS - 1;
			}
			else
			{
				switch(item_index)
				{
					case 0:
					case 1:
						if(set_value < 1500)
							set_value++;
						else
							set_value = 0;						
						break;
					case 2:
					case 3:
						if(set_value)
							set_value = 0;
						else
							set_value = 1;
						break;
				}

				value_change = TRUE;
			}
			break;
		case KEY_DOWN_MASK:
			if(in_sub_menu == FALSE)
			{
				item_index++;
				item_index %= MAX_TEMP_ITEMS;
			}
			else
			{
				switch(item_index)
				{
					case 0:
					case 1:
						if(set_value)
							set_value--;
						else
							set_value = 1500;						
						break;
					case 2:
					case 3:
						if(set_value)
							set_value = 0;
						else
							set_value = 1;
						break;
				}
				value_change = TRUE;
			}
			break;
	}
}



