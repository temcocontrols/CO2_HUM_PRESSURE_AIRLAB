#include "config.h"

static uint8 item_index = 0;
static uint8 pre_item_index = 0;

static uint16 set_value;

#define MAX_CO2_OPT_ITEMS	4
static uint8 const code item_name[MAX_CO2_OPT_ITEMS][16] = 
{
	"1.Delete node",
	"2.Spt poor: ",
	"3.Spt fair: ",
	"4.Calibration: ",
};


void Co2_Opt_init(void)
{
	uint8 i;
	Lcd_Full_Screen(0);

	for(i = 0; i < 4; i++)
	{
		switch(i)
		{
			case 0:
				sprintf((char *)text, "%s", item_name[i]);
				break;
			case 1:
				if(current_co2_sensor == 0)
					sprintf((char *)text, "%s%u", item_name[i], int_co2_str.alarm_setpoint);
//				else
//					sprintf((char *)text, "%s%u", item_name[i], ext_co2_str[current_co2_sensor - 1].alarm_setpoint);
				break;
			case 2:
				if(current_co2_sensor == 0)
					sprintf((char *)text, "%s%u", item_name[i], int_co2_str.pre_alarm_setpoint);
//				else
//					sprintf((char *)text, "%s%u", item_name[i], ext_co2_str[current_co2_sensor - 1].pre_alarm_setpoint);
				break;
			case 3:
				if(current_co2_sensor == 0)
					sprintf((char *)text, "%s%u", item_name[i], int_co2_str.co2_int);
//				else
//					sprintf((char *)text, "%s%u", item_name[i], ext_co2_str[current_co2_sensor - 1].co2_int);
				break;
		}

		if(i == item_index)
			Lcd_Show_String(i, 0, DISP_INV, text);
		else
			Lcd_Show_String(i, 0, DISP_NOR, text);
	}

	in_sub_menu = FALSE;
}

void Co2_Opt_display(void)
{
	if(in_sub_menu == FALSE)
	{
		if(pre_item_index != item_index)
		{
			switch(pre_item_index)
			{
				case 0:
					sprintf((char *)text, "%s", item_name[pre_item_index]);
					break;
				case 1:
					if(current_co2_sensor == 0)
						sprintf((char *)text, "%s%u", item_name[pre_item_index], int_co2_str.alarm_setpoint);
//					else
//						sprintf((char *)text, "%s%u", item_name[pre_item_index], ext_co2_str[current_co2_sensor - 1].alarm_setpoint);
					break;
				case 2:
					if(current_co2_sensor == 0)
						sprintf((char *)text, "%s%u", item_name[pre_item_index], int_co2_str.pre_alarm_setpoint);
//					else
//						sprintf((char *)text, "%s%u", item_name[pre_item_index], ext_co2_str[current_co2_sensor - 1].pre_alarm_setpoint);
					break;
				case 3:
					if(current_co2_sensor == 0)
						sprintf((char *)text, "%s%u", item_name[pre_item_index], int_co2_str.co2_int);
//					else
//						sprintf((char *)text, "%s%u", item_name[pre_item_index], ext_co2_str[current_co2_sensor - 1].co2_int);
					break;
			}
			Lcd_Clear_Row(pre_item_index);
			Lcd_Show_String(pre_item_index, 0, DISP_NOR, text);

			switch(item_index)
			{
				case 0:
					sprintf((char *)text, "%s", item_name[item_index]);
					break;
				case 1:
					if(current_co2_sensor == 0)
						sprintf((char *)text, "%s%u", item_name[item_index], int_co2_str.alarm_setpoint);
//					else
//						sprintf((char *)text, "%s%u", item_name[item_index], ext_co2_str[current_co2_sensor - 1].alarm_setpoint);
					break;
				case 2:
					if(current_co2_sensor == 0)
						sprintf((char *)text, "%s%u", item_name[item_index], int_co2_str.pre_alarm_setpoint);
//					else
//						sprintf((char *)text, "%s%u", item_name[item_index], ext_co2_str[current_co2_sensor - 1].pre_alarm_setpoint);
					break;
				case 3:
					if(current_co2_sensor == 0)
						sprintf((char *)text, "%s%u", item_name[item_index], int_co2_str.co2_int);
//					else
//						sprintf((char *)text, "%s%u", item_name[item_index], ext_co2_str[current_co2_sensor - 1].co2_int);
					break;
			}
			Lcd_Clear_Row(item_index);
			Lcd_Show_String(item_index, 0, DISP_INV, text);

			pre_item_index = item_index;
		}
		else
		{
			if(current_co2_sensor == 0)
				sprintf((char *)text, "%u", int_co2_str.co2_int);
//			else
//				sprintf((char *)text, "%u", ext_co2_str[current_co2_sensor - 1].co2_int);

			Lcd_Show_String(3, 15, DISP_NOR, (uint8 *)int_space);
			if(item_index == 3) // calibration, update display
				Lcd_Show_String(3, 15, DISP_INV, text);
			else
				Lcd_Show_String(3, 15, DISP_NOR, text);
		}
	}
	else
	{
		if(value_change == TRUE)
		{
			sprintf((char *)text, "%u", set_value);
			if((item_index == 1) || (item_index == 2))
			{
				Lcd_Show_String(item_index, 12, DISP_NOR, (uint8 *)int_space);
				Lcd_Show_String(item_index, 12, DISP_INV, text);
			}
			else if(item_index == 3)
			{
				Lcd_Show_String(item_index, 15, DISP_NOR, (uint8 *)int_space);
				Lcd_Show_String(item_index, 15, DISP_INV, text);
			}
			value_change = FALSE;
		}
	}
}

void Co2_Opt_keycope(uint16 key_value)
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
				update_menu_state(MenuCo2_Conf);
			}
			else
			{
				switch(item_index)
				{
					case 0:	// Delete node
							
						break;
					case 1: // Setpoint poor
						if(current_co2_sensor == 0)
							sprintf((char *)text, "%s%u", item_name[item_index], int_co2_str.alarm_setpoint);
//						else
//							sprintf((char *)text, "%s%u", item_name[item_index], ext_co2_str[current_co2_sensor - 1].alarm_setpoint);
					break;
					case 2:	// Setpoint fair
						if(current_co2_sensor == 0)
							sprintf((char *)text, "%s%u", item_name[item_index], int_co2_str.pre_alarm_setpoint);
//						else
//							sprintf((char *)text, "%s%u", item_name[item_index], ext_co2_str[current_co2_sensor - 1].pre_alarm_setpoint);
						break;
					case 3:	// Calibration
						if(current_co2_sensor == 0)
							sprintf((char *)text, "%s%u", item_name[item_index], int_co2_str.co2_int);
//						else
//							sprintf((char *)text, "%s%u", item_name[item_index], ext_co2_str[current_co2_sensor - 1].co2_int);
						break;
				}

				Lcd_Clear_Row(item_index);
				Lcd_Show_String(item_index, 0, DISP_INV, text);

				in_sub_menu = FALSE;
			}
			break;
		case KEY_RIGHT_MASK:
			// enter the sub menu of co2 attributes settings
			switch(item_index)
			{
				case 0: // delete id from database
//					if(current_co2_sensor == 0)
//					{
//						remove_id_from_db(current_co2_sensor);
//						update_menu_state(MenuCo2_Conf);
//					}
					break;
				case 1:	// setpoint poor
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index, 0, DISP_NOR, text);

						if(current_co2_sensor == 0)
							set_value = int_co2_str.alarm_setpoint;
//						else
//							set_value = ext_co2_str[current_co2_sensor - 1].alarm_setpoint;

						sprintf((char *)text, "%u", set_value);
						Lcd_Show_String(item_index, 12, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						if(current_co2_sensor == 0)
						{
							int_co2_str.alarm_setpoint = set_value;
							write_eeprom(EEP_INT_ALARM_SETPOINT, int_co2_str.alarm_setpoint);
							write_eeprom(EEP_INT_ALARM_SETPOINT + 1, int_co2_str.alarm_setpoint >>8); 
//							start_data_save_timer();
//							flash_write_int(FLASH_CO2_INTERNAL_ALARM_SETPOINT, int_co2_str.alarm_setpoint);
						}
//						else
//						{
//							ext_co2_str[current_co2_sensor - 1].alarm_setpoint = set_value;
//							write_parameters_to_nodes(current_co2_sensor, SLAVE_MODBUS_CO2_ALARM_SETPOINT, set_value);
//						}

						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Clear_Row(item_index);
						Lcd_Show_String(item_index, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 2:	// setpoint fair
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index, 0, DISP_NOR, text);

						if(current_co2_sensor == 0)
							set_value = int_co2_str.pre_alarm_setpoint;
//						else
//							set_value = ext_co2_str[current_co2_sensor - 1].pre_alarm_setpoint;

						sprintf((char *)text, "%u", set_value);
						Lcd_Show_String(item_index, 12, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						if(current_co2_sensor == 0)
						{
							int_co2_str.pre_alarm_setpoint = set_value;
							write_eeprom(EEP_INT_PRE_ALARM_SETPOINT, int_co2_str.pre_alarm_setpoint);
							write_eeprom(EEP_INT_PRE_ALARM_SETPOINT + 1, int_co2_str.pre_alarm_setpoint >>8); 
//							start_data_save_timer();
//							flash_write_int(FLASH_CO2_INTERNAL_PREALARM_SETPOINT, int_co2_str.pre_alarm_setpoint);
						}
//						else
//						{
//							ext_co2_str[current_co2_sensor - 1].pre_alarm_setpoint = set_value;
//							write_parameters_to_nodes(current_co2_sensor, SLAVE_MODBUS_CO2_PRE_ALARM_SETPOINT, set_value);
//						}

						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Clear_Row(item_index);
						Lcd_Show_String(item_index, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 3:	// calibration
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index, 0, DISP_NOR, text);

						if(current_co2_sensor == 0)
							set_value = int_co2_str.co2_int;
//						else
//							set_value = ext_co2_str[current_co2_sensor - 1].co2_int;

						sprintf((char *)text, "%u", set_value);
						Lcd_Show_String(item_index, 15, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						if(current_co2_sensor == 0)
						{
							int_co2_str.co2_offset += set_value - int_co2_str.co2_int;
							write_eeprom(EEP_INT_CO2_OFFSET, (uint8)(int_co2_str.co2_offset));
							write_eeprom(EEP_INT_CO2_OFFSET + 1, (uint8)(int_co2_str.co2_offset >> 8));
		
//							start_data_save_timer();
//							flash_write_int(FLASH_CO2_INTERNAL_OFFSET, (uint16)int_co2_str.co2_offset);
						}
//						else
//						{
//							ext_co2_str[current_co2_sensor - 1].co2_offset += set_value - ext_co2_str[current_co2_sensor - 1].co2_int;
//							write_parameters_to_nodes(current_co2_sensor, SLAVE_MODBUS_CO2_OFFSET, ext_co2_str[current_co2_sensor - 1].co2_offset);
//						}
						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Clear_Row(item_index);
						Lcd_Show_String(item_index, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
			}
			break;
		case KEY_UP_MASK:
			if(in_sub_menu == FALSE)
			{
				if(item_index)
					item_index--;
				else
					item_index = 3;
			}
			else
			{
				switch(key_value & KEY_FUNCTION_MASK)
				{
					case KEY_SPEED_1:
						set_value += SPEED_1;
						break;
					case KEY_SPEED_10:
						set_value += SPEED_10;
						break;
					case KEY_SPEED_50:
						set_value += SPEED_50;
						break;
					case KEY_SPEED_100:
						set_value += SPEED_100;
						break;
				}

				value_change = TRUE;
			}
			break;
		case KEY_DOWN_MASK:
			if(in_sub_menu == FALSE)
			{
				item_index++;
				item_index %= 4;
			}
			else
			{
				switch(key_value & KEY_FUNCTION_MASK)
				{
					case KEY_SPEED_1:
						set_value -= SPEED_1;
						break;
					case KEY_SPEED_10:
						set_value -= SPEED_10;
						break;
					case KEY_SPEED_50:
						set_value -= SPEED_50;
						break;
					case KEY_SPEED_100:
						set_value -= SPEED_100;
						break;
				}

				value_change = TRUE;
			}
			break;
	}
}



