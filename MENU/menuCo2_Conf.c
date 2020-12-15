#include "config.h"

uint8 current_co2_sensor = 0;
static uint8 item_index = 0;
static uint8 pre_item_index = 0;

uint8 const   online_text[] = " ONLINE";
uint8 const   offline_text[] = " OFFLINE";

void display_zone_text(uint8 index)
{
	//if(isColorScreen==false)
		{
		index %= MAX_ROW;
		if((index == 0) && (item_index < 5)) // zone0, internal sensor
		{
			if(internal_co2_exist == TRUE)
				sprintf((char *)text, "%s%s", internal_text, online_text);
			else
				sprintf((char *)text, "%s%s", internal_text, offline_text);
		}
		else // external sensors
		{
			// read location name from external sensor
			sprintf((char *)text, "%s%u: %s", external_text, (uint16)scan_db[index + item_index / 5 * 5].id, online_text);
		}

		Lcd_Clear_Row(index);
		if(index == (item_index % MAX_ROW))
			Lcd_Show_String(index, 0, DISP_INV, text);
		else
			Lcd_Show_String(index, 0, DISP_NOR, text);
	}
}

void Co2_Conf_init(void)
{
	uint8 i, display_lines;

	Lcd_Full_Screen(0);

	if(((db_ctr % MAX_ROW) == 0) || (item_index < (db_ctr / 5 * 5)))
		display_lines = MAX_ROW;
	else
		display_lines = db_ctr % MAX_ROW;

	for(i = 0; i < display_lines; i++)
		display_zone_text(i);

	current_co2_sensor = item_index;
	previous_internal_co2_exist = internal_co2_exist;
	pre_db_ctr = db_ctr;
}

void Co2_Conf_display(void)
{
	if(pre_item_index != item_index)
	{
		if( ((pre_item_index % MAX_ROW == 4) && (item_index % MAX_ROW == 0))
		 || ((pre_item_index % MAX_ROW == 0) && (item_index % MAX_ROW == 4))
		 || ((item_index == 0) && (pre_item_index == db_ctr - 1))
		 || ((item_index == db_ctr - 1) && (pre_item_index == 0)) )
		{
			Co2_Conf_init();
		}
		else
		{
			display_zone_text(pre_item_index);
			display_zone_text(item_index);
		}
		pre_item_index = item_index;
		current_co2_sensor = item_index;
	}
	else if((pre_db_ctr != db_ctr) || (previous_internal_co2_exist != internal_co2_exist))
	{
		Co2_Conf_init();
	}
}

void Co2_Conf_keycope(uint16 key_value)
{
	start_back_light(backlight_keep_seconds);
	switch(key_value & KEY_SPEED_MASK)
	{
		case KEY_NON:
			// do nothing
			break;
		case KEY_LEFT_MASK:
			// return to main menu
			update_menu_state(MenuMain);
			break;
		case KEY_RIGHT_MASK:
			// enter the sub menu of co2 attributes settings
			update_menu_state(MenuCo2Option);
			break;
		case KEY_UP_MASK:
			if(item_index)
				item_index--;
			else
				item_index = db_ctr - 1;
			break;
		case KEY_DOWN_MASK:
			item_index++;
			item_index %= db_ctr;
			break;
	}
}



