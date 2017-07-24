
#include "config.h"

static uint8 item_index = 0;
static uint8 pre_item_index = 0;

static uint16 set_value = 0;

#define MAX_PRESS_ITEMS	3
static uint8 const code item_name[MAX_PRESS_ITEMS][17] = 
{
	"1.Model: ", 
	"2.Unit: ",
	"3.ZERO: ",
};
static uint8 const code item_name_Zero[][6] = 
{
	"  N  ", 
	"  Y  ", 
};
 

void About_init(void)
{ 
	uint8 i;
	if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) ) 
	{					
		Lcd_Full_Screen(0);
		
		for(i = 0; i < MAX_PRESS_ITEMS; i++)
		{
			strcpy((char *)text, (char *)item_name[i]);
			switch(i)
			{
				case 0: // 
					if(Pressure.SNR_Model == PRS_26PCGFA)
					{ 
						strcat((char *)text, "26PCGFA"); 
					}
					else if(Pressure.SNR_Model == PRS_26PCDFA)
					{ 
						strcat((char *)text, "26PCDFA"); 
					}
					else if(Pressure.SNR_Model == MPXV7002DP)
					{ 
						strcat((char *)text, "MPXV7002DP"); 
					}
					else if(Pressure.SNR_Model == MPXV7007DP)
					{ 
						strcat((char *)text, "MPXV7007DP"); 
					}
					else if(Pressure.SNR_Model == DLVR_L01D)
					{
						strcat((char *)text, "DLVR_L01D"); 
					}
					else if(Pressure.SNR_Model == MS4515)
					{ 
						strcat((char *)text, "MS4515"); 
					}
					break;
				case 1:	// external temperature
					   
					strcat((char *)text, (char *)Prs_Unit[Pressure.unit]);  
				
					break;
				 
				case 2:	// Select to display on normal screen 
						strcat((char *)text, "Y/N ?"); 
					break;
			}

			if(i == item_index)
				Lcd_Show_String(i, 0, DISP_INV, text);
			else
				Lcd_Show_String(i, 0, DISP_NOR, text);
		} 
		in_sub_menu = FALSE;
	} 
}

void About_display(void)
{
	if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) ) 
	{					
		if(in_sub_menu == FALSE)
		{
			if(pre_item_index != item_index)
			{
				Lcd_Clear_Row(item_index);
				Lcd_Clear_Row(pre_item_index);

				strcpy((char *)text, (char *)item_name[pre_item_index]);
				switch(pre_item_index)
				{
					case 0:
						if(Pressure.SNR_Model == PRS_26PCGFA)
						{ 
							strcat((char *)text, "26PCGFA"); 
						}
						else if(Pressure.SNR_Model == PRS_26PCDFA)
						{ 
							strcat((char *)text, "26PCDFA"); 
						}
						else if(Pressure.SNR_Model == MPXV7002DP)
						{ 
							strcat((char *)text, "MPXV7002DP"); 
						}
						else if(Pressure.SNR_Model == MPXV7007DP)
						{ 
							strcat((char *)text, "MPXV7007DP"); 
						}
						else if(Pressure.SNR_Model == DLVR_L01D)
						{
							strcat((char *)text, "DLVR_L01D"); 
						}
						else if(Pressure.SNR_Model == MS4515)
						{
							strcat((char *)text, "MS4515"); 
						}
						break;
					case 1:
						strcat((char *)text, (char *)Prs_Unit[Pressure.unit]);  
						break;
					case 2: 
						strcat((char *)text, "Y/N ?"); 
						break;
					 
				}
				Lcd_Show_String(pre_item_index, 0, DISP_NOR, text);

				strcpy((char *)text, (char *)item_name[item_index]);
				switch(item_index)
				{
					 case 0:
						if(Pressure.SNR_Model == PRS_26PCGFA)
						{ 
							strcat((char *)text, "26PCGFA"); 
						}
						else if(Pressure.SNR_Model == PRS_26PCDFA)
						{ 
							strcat((char *)text, "26PCDFA"); 
						}
						else if(Pressure.SNR_Model == MPXV7002DP)
						{ 
							strcat((char *)text, "MPXV7002DP"); 
						}
						else if(Pressure.SNR_Model == MPXV7007DP)
						{ 
							strcat((char *)text, "MPXV7007DP"); 
						}
						else if(Pressure.SNR_Model == DLVR_L01D)
						{
							strcat((char *)text, "DLVR_L01D"); 
						} 
						else if(Pressure.SNR_Model == MS4515)
						{ 
							strcat((char *)text, "MS4515"); 
						}
						break;
					case 1:
						strcat((char *)text, (char *)Prs_Unit[Pressure.unit]);  
						break;
					case 2: 
						strcat((char *)text, "Y/N ?"); 
						break;
				}
				Lcd_Show_String(item_index, 0, DISP_INV, text);

				pre_item_index = item_index;
			} 
		}
		else
		{
			if(value_change == TRUE)
			{
				switch(item_index)
				{  
					case 1:    
						strcpy((char *)text, (char *)Prs_Unit[set_value]);
						Lcd_Show_String(item_index, strlen((char *)item_name[item_index]), DISP_INV, text); 
						break;
					case 2:   
						strcpy((char *)text, (char *)item_name_Zero[set_value]);  
						Lcd_Show_String(item_index, strlen((char *)item_name[item_index]), DISP_INV, text);						 
						break;  
				}
				
				value_change = FALSE;	
			}
		}
	}
}

void About_keycope(uint16 key_value)
{
	if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) ) 
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
						 
						case 1:
							strcat((char *)text, (char *)Prs_Unit[Pressure.unit]);  
							break;
						case 2: 
							strcat((char *)text, "Y/N ?"); 
							break;
					}
					Lcd_Show_String(item_index, 0, DISP_INV, text);

					in_sub_menu = FALSE;
				}
				break;
			case KEY_RIGHT_MASK:
				if(in_sub_menu == FALSE)
				{
					strcpy((char *)text, (char *)item_name[item_index]);
					Lcd_Show_String(item_index, 0, DISP_NOR, text);
					switch(item_index)
					{ 
						case 1:  
								set_value = Pressure.unit;
								strcpy((char *)text,(char *) Prs_Unit[Pressure.unit]);
								Lcd_Show_String(item_index, strlen((char *)item_name[item_index]), DISP_INV, text);
						
						break;
						case 2:  
							set_value = 0; 
							strcpy((char *)text, (char *)item_name_Zero[set_value]);  
							Lcd_Show_String(item_index, strlen((char *)item_name[item_index]), DISP_INV, text);						 
							break;
						 
					}
				
					in_sub_menu = TRUE;
				}
				else
				{
					strcpy((char *)text,(char *) item_name[item_index]);
					switch(item_index)
					{
						 
						case 1:
							Pressure.unit = set_value;
							Pressure.unit_change = 1;
							write_eeprom(EEP_PRESSURE_UNIT,set_value);
							strcat((char *)text, (char *)Prs_Unit[Pressure.unit]);  
							break;
						case 2: 
							strcat((char *)text, " Y/N "); 
							if(set_value)
							{ 			 
								Pressure.org_val_offset += (0 - Pressure.org_val );
								Pressure.org_val =  0;
								write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,Pressure.org_val_offset);
								write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,Pressure.org_val_offset >> 8);
							}
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
						item_index = MAX_PRESS_ITEMS - 1;
				}
				else
				{
					switch(item_index)
					{
						 
						case 1:
							set_value++;
							if(set_value >= Unit_End) 
								set_value = 0;						
							break;
						case 2: 
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
					item_index %= MAX_PRESS_ITEMS;
				}
				else
				{
					switch(item_index)
					{
						 
						case 1:
							if(set_value)
								set_value--;
							else
								set_value = Unit_End - 1;						
							break;
						case 2: 
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
}



