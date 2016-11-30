#include "config.h"

static uint8 item_index = 0;
static uint8 pre_item_index = 0;

static uint16 set_value;

#define MAX_HUM_ITEMS	2
static uint8 const code item_name[MAX_HUM_ITEMS][16] = 
{
	"1.Calibration: ",
	"2.Heat: ",
};

static uint8 const code OnOff[2][4] = 
{
	"OFF",
	"ON",
};

void Hum_Conf_init(void)
{
	uint8 i;
	Lcd_Full_Screen(0);
	for(i = 0; i < MAX_HUM_ITEMS; i++)
	{
		switch(i)
		{
			case 0:
				sprintf((char *)text, "%s%u.%u%%", item_name[i], HumSensor.humidity / 10, HumSensor.humidity % 10);
				break;
			case 1:
				sprintf((char *)text, "%s%s", item_name[i], OnOff[hum_heat_status]);
				break;
		}

		if(i == item_index)
			Lcd_Show_String(i, 0, DISP_INV, text);
		else
			Lcd_Show_String(i, 0, DISP_NOR, text);
	}

	in_sub_menu = FALSE;
}

void Hum_Conf_display(void)
{
	if(in_sub_menu == FALSE)
	{
		if(pre_item_index != item_index)
		{
			Lcd_Clear_Row(item_index);
			Lcd_Clear_Row(pre_item_index);

			switch(pre_item_index)
			{
				case 0:
					sprintf((char *)text, "%s%u.%u%%", item_name[pre_item_index], HumSensor.humidity / 10, HumSensor.humidity % 10);
					break;
				case 1:
					sprintf((char *)text, "%s%s", item_name[pre_item_index], OnOff[hum_heat_status]);
					break;
			}
			Lcd_Show_String(pre_item_index, 0, DISP_NOR, text);

			switch(item_index)
			{
				case 0:
					sprintf((char *)text, "%s%u.%u%%", item_name[item_index], HumSensor.humidity / 10, HumSensor.humidity% 10);
					break;
				case 1:
					sprintf((char *)text, "%s%s", item_name[item_index], OnOff[hum_heat_status]);
					break;
			}
			Lcd_Show_String(item_index, 0, DISP_INV, text);

			pre_item_index = item_index;
		}
		else
		{
			sprintf((char *)text, "%u.%u%%", HumSensor.humidity / 10, HumSensor.humidity % 10);
			Lcd_Show_String(0, 15, DISP_NOR, (uint8 *)int_space);
			if(item_index == 0) // calibration, update display
				Lcd_Show_String(0, 15, DISP_INV, text);
			else
				Lcd_Show_String(0, 15, DISP_NOR, text);
		}
	}
	else
	{
		if(value_change == TRUE)
		{
			switch(item_index)
			{
				case 0:
					sprintf((char *)text, "%u.%u%%", set_value / 10, set_value % 10);
					Lcd_Show_String(item_index, 15, DISP_NOR,(uint8 *) "      ");
					Lcd_Show_String(item_index, 15, DISP_INV, text);
					break;
				case 1:
					Lcd_Show_String(item_index, 8, DISP_NOR, (uint8 *)"   ");
					Lcd_Show_String(item_index, 8, DISP_INV, (uint8 *)OnOff[set_value]);
					break;
			}
			
			value_change = FALSE;	
		}
	}
}

void Hum_Conf_keycope(uint16 key_value)
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
				switch(item_index)
				{
					case 0:
						sprintf((char *)text, "%s%u.%u%%", item_name[item_index],  HumSensor.humidity / 10,  HumSensor.humidity % 10);
						break;
					case 1:
						sprintf((char *)text, "%s%s", item_name[item_index], OnOff[hum_heat_status]);
						break;
				}
				Lcd_Show_String(item_index, 0, DISP_INV, text);

				in_sub_menu = FALSE;
			}
			break;
		case KEY_RIGHT_MASK:
			if(in_sub_menu == FALSE)
			{
				sprintf((char *)text, "%s", item_name[item_index]);
				Lcd_Show_String(item_index, 0, DISP_NOR, text);

				switch(item_index)
				{
					case 0:
						set_value =  HumSensor.humidity;
						sprintf((char *)text, "%u.%u%%", set_value / 10, set_value % 10);
						Lcd_Show_String(item_index, 15, DISP_INV, text);
						break;
					case 1:
						set_value = hum_heat_status;
						Lcd_Show_String(item_index, 8, DISP_INV,(uint8 *) OnOff[set_value]);
						break;
				}
				
				in_sub_menu = TRUE;
			}
			else
			{
				switch(item_index)
				{
					case 0:
						sprintf((char *)text, "%s%u.%u%%", item_name[item_index], set_value / 10, set_value % 10);
						external_operation_value = set_value;
						external_operation_flag = HUM_CALIBRATION; 
						HumSensor.humidity = set_value;
						Run_Timer = 0;
						break;
					case 1:
						sprintf((char *)text, "%s%s", item_name[item_index], OnOff[set_value]);
						external_operation_value = (uint8)set_value;
						external_operation_flag = HUM_HEATER;
						
//						hum_heat_status = set_value;
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
					item_index = MAX_HUM_ITEMS - 1;
			}
			else
			{
				switch(item_index)
				{
					case 0:
						if(set_value < 1000)
							set_value++;
						else
							set_value = 0;
						break;
					case 1:
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
				item_index %= MAX_HUM_ITEMS;
			}
			else
			{
				switch(item_index)
				{
					case 0:
						if(set_value)
							set_value--;
						else
							set_value = 1000;
						break;
					case 1:
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



