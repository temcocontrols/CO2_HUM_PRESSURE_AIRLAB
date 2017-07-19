#include "config.h"

#define MAX_MAIN_ITEMS	6
static uint8 const code item_name[MAX_MAIN_ITEMS][16] = 
{
	"1.CO2",
	"2.Temperature",
	"3.Humidity",
	"4.Miscellaneous",
	"5.Pressure",
	"6.PM25"
};

static uint8 const code item[] = {MenuCo2_Conf, MenuTemp_Conf, MenuHum_Conf, MenuMisc, MenuPresConf}; 

static uint8 item_index = 0;
static uint8 pre_item_index = 0;

void Co2_Main_init(void)
{
	uint8 i, display_lines;

	if(use_password == FALSE)
		menu_password = TRUE;

	Lcd_Full_Screen(0);
	if(((MAX_MAIN_ITEMS % MAX_ROW) == 0) || (item_index < (MAX_MAIN_ITEMS / 5 * 5)))
		display_lines = MAX_ROW;
	else
		display_lines = MAX_MAIN_ITEMS % MAX_ROW;
	for(i = 0; i < display_lines; i++)
	{
		u8 temp; 
		temp = i + item_index / 5 * 5;
		if((i == item_index% MAX_ROW) && (menu_password == TRUE))
			Lcd_Show_String(i, 0, DISP_INV, (uint8 *)item_name[temp]);
		else
			Lcd_Show_String(i, 0, DISP_NOR, (uint8 *)item_name[temp]);
	}

	if(menu_password == FALSE)
	{
		Lcd_Show_String(1, 5, DISP_INV, (uint8 *)"  PASSWORD:  ");
		Lcd_Show_String(2, 5, DISP_INV, (uint8 *)"    ****     ");
		password_buffer[0] = '*';
		password_buffer[1] = '*';
		password_buffer[2] = '*';
		password_buffer[3] = '*';
		password_index = 0;
		cursor_on(2, 9, '*', ' ');
	}
}

void Co2_Main_display(void)
{
	if(menu_password == FALSE)
	{
		if(value_change == TRUE)
		{
			cursor.on_byte = password_buffer[password_index];
			value_change = FALSE;
		}
	}
	else
	{
		if(pre_item_index != item_index)
		{
			Lcd_Show_String(pre_item_index% MAX_ROW, 0, DISP_NOR, (uint8 *)item_name[pre_item_index]);
			Lcd_Show_String(item_index% MAX_ROW, 0, DISP_INV, (uint8 *)item_name[item_index]);
			pre_item_index = item_index;
		}
	}
}

void Co2_Main_keycope(uint16 key_value)
{
	start_back_light(backlight_keep_seconds);
	if(menu_password == FALSE)
	{
		switch(key_value & KEY_SPEED_MASK)
		{
			case KEY_NON:
				// do nothing
				break;
			case KEY_UP_MASK:
				if((password_buffer[password_index] >= '9') || (password_buffer[password_index] < '0') || (password_buffer[password_index] == '*'))
					password_buffer[password_index] = '0';
				else
					password_buffer[password_index]++;

				value_change = TRUE;
				break;
			case KEY_DOWN_MASK:
				if((password_buffer[password_index] <= '0') || (password_buffer[password_index] > '9') || (password_buffer[password_index] == '*'))
					password_buffer[password_index] = '9';
				else
					password_buffer[password_index]--;

				value_change = TRUE;
				break;
			case KEY_LEFT_MASK:
				if(password_index == 0)
				{
					cursor_off();
					update_menu_state(MenuIdle);
				}
				else
				{
					Lcd_Write_Char(cursor.row, cursor.line--, password_buffer[password_index--], DISP_INV);
					cursor.on_byte = password_buffer[password_index];
				}
				break;
			case KEY_RIGHT_MASK:
				if(password_index < (MAX_PASSWORD_DIGITALS - 1))
				{
					Lcd_Write_Char(cursor.row, cursor.line++, password_buffer[password_index++], DISP_INV);
					cursor.on_byte = password_buffer[password_index];
				}
				else
				{
					if((password_buffer[0] == user_password[0]) && (password_buffer[1] == user_password[1]) 
					&& (password_buffer[2] == user_password[2]) && (password_buffer[3] == user_password[3]))
					{
						cursor_off();
						menu_password = TRUE;
					}
					Co2_Main_init();
				}
				break;
		}
	}
	else
	{
		switch(key_value & KEY_SPEED_MASK)
		{
			case KEY_NON:
				// do nothing
				break;
			case KEY_UP_MASK:
				if(item_index)
					item_index--;
				else
					item_index = 4;
				break;
			case KEY_DOWN_MASK:
				item_index++;
				item_index %= 5;
				break;
			case KEY_LEFT_MASK:
				update_menu_state(MenuIdle);
				break;
			case KEY_RIGHT_MASK:
				// enter sub menu
//				update_menu_state(item[item_index]);
				if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
				{
					if(item[item_index] != MenuPresConf)
						update_menu_state(item[item_index]);
				}
				else if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				{	
					if((item[item_index] != MenuCo2_Conf)&&(item[item_index] != MenuHum_Conf)&&(item[item_index] != MenuTemp_Conf))
						update_menu_state(item[item_index]);
				}
				else if (PRODUCT_ID == STM32_PM25)
				{	
					if((item[item_index] != MenuCo2_Conf)&&(item[item_index] != MenuHum_Conf)&&(item[item_index] != MenuTemp_Conf)&&(item[item_index] != MenuPresConf))
						update_menu_state(item[item_index]);
				}
				else// defined HUM_SENSOR
				{
					if((item[item_index] != MenuCo2_Conf)&&(item[item_index] != MenuPresConf))
						update_menu_state(item[item_index]);
				}
				break;
		}
	}
}



