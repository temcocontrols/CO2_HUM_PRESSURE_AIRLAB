#include "config.h"

static uint8 item_index = 0;
static uint8 pre_item_index = 0;

static uint16 set_value;

static uint8 sub_menu_index = 0;

#define IP_NO_CHANGE	0
#define IP_CHANGED		1
#define IP_ENABLE		2
#define IP_DISABLE		3
uint8 ipconfig_in_menu = IP_NO_CHANGE;
uint8 ip_changed_in_menu = 0;
uint8 subnet_changed_in_menu = 0;
uint8 gateway_changed_in_menu = 0;
uint8 port_changed_in_menu = 0; 
#define MAX_MISC_ITEMS	15
static uint8 const code item_name[MAX_MISC_ITEMS][19] = 
{
	"1.Ip mode: ",
	"2.Ip:",
	"3.Msk:",
	"4.Gat:",
	"5.Port:",
	"6.New ip: ",
	"7.Modbus ID: ",
	"8.Baudrate: ",
	"9.Date: ",
	"10.Time: ",
	"11.Alarm on sec: ",
	"12.Alarm off sec: ",
	"13.Use password: ",
	"14.Password: ",
    "15.Protocol: ",
};
static uint8 const code protocol_text[2][7] = 
{
	"Mstp",
	"Modbus", 
}; 
static uint8 const code ip_mode_text[2][7] = 
{
	"Static",
	"Dhcp",
};

static uint8 const code new_ip_status_text[4][10] = 
{
	"No-change",
	"Y/N",
	"Yes",
	"Cannel",
};

  uint8 const code baudrate_text[5][7] = 
{
	"9600",
	"19200",
	"38400",
	"57600",
	"115200"
};

static uint8 const code YesOrNo[2][4] = 
{
	"No",
	"Yes",
};

static uint8 const code PARITY_TEXT[3][3]=
{
	"-N",
	"-O",
	"-E",
};
void Misc_init(void)
{
	uint8 i, display_lines;

	if(ipconfig_in_menu == IP_DISABLE)
	{
		
		modbus.ghost_ip_mode = modbus.ip_mode;

		for(i=0; i<4; i++)
		{
			modbus.ghost_ip_addr[i] = modbus.ip_addr[i];
			modbus.ghost_mask_addr[i] = modbus.mask_addr[i];
			modbus.ghost_gate_addr[i] = modbus.gate_addr[i]; 					
		} 
		modbus.ghost_listen_port = modbus.listen_port;

		ipconfig_in_menu = IP_NO_CHANGE;
	}


	Lcd_Full_Screen(0);

	if(((MAX_MISC_ITEMS % MAX_ROW) == 0) || (item_index < (MAX_MISC_ITEMS / 5 * 5)))
		display_lines = MAX_ROW;
	else
		display_lines = MAX_MISC_ITEMS % MAX_ROW;

	for(i = 0; i < display_lines; i++)
	{
		strcpy((char *)text, (char *)item_name[i + item_index / 5 * 5]);
		switch(i + item_index / 5 * 5)
		{
			case 0:
//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], ip_mode_text[modbus.ghost_ip_mode]);
				strcat((char *)text, (char *)ip_mode_text[modbus.ghost_ip_mode]);	
				break;
			case 1:
//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / 5 * 5], (uint16)modbus.ghost_ip_addr[0], (uint16)modbus.ghost_ip_addr[1], (uint16)modbus.ghost_ip_addr[2], (uint16)modbus.ghost_ip_addr[3]);
				itoa(modbus.ghost_ip_addr[0], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text,  ".");
				itoa(modbus.ghost_ip_addr[1], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(modbus.ghost_ip_addr[2], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(modbus.ghost_ip_addr[3], int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 2:
//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / 5 * 5], (uint16)modbus.ghost_mask_addr[0], (uint16)modbus.ghost_mask_addr[1], (uint16)modbus.ghost_mask_addr[2], (uint16)modbus.ghost_mask_addr[3]);
				itoa(modbus.ghost_mask_addr[0], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(modbus.ghost_mask_addr[1], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(modbus.ghost_mask_addr[2], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(modbus.ghost_mask_addr[3], int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 3:
//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / 5 * 5], (uint16)modbus.ghost_gate_addr[0], (uint16)modbus.ghost_gate_addr[1], (uint16)modbus.ghost_gate_addr[2], (uint16)modbus.ghost_gate_addr[3]);
				itoa(modbus.ghost_gate_addr[0], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(modbus.ghost_gate_addr[1], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(modbus.ghost_gate_addr[2], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(modbus.ghost_gate_addr[3], int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 4:
//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], modbus.ghost_listen_port);
				itoa(modbus.ghost_listen_port, int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 5:	
//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], new_ip_status_text[ipconfig_in_menu]);
				strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);	
				break;
			case 6: // Modbus ID
//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)modbus.address);
				itoa(modbus.address, int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 7:
//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], baudrate_text[modbus.baud]);
				strcat((char *)text, (char *)baudrate_text[modbus.baud]);
				strcat((char *)text, (char *)PARITY_TEXT[uart1_parity]);
				break;
			case 8: // Date
//				sprintf((char *)text, "%s%04d-%02d-%02d", item_name[i + item_index / 5 * 5], (uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
//				itoa(calendar.w_year, int_text, 0);
//				strcat((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"-");
//				itoa(calendar.w_month, int_text, 0);
//				strcat((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"-");
//				itoa(calendar.w_date, int_text, 0);
				sprintf((char *)int_text, "%04d-%02d-%02d",(uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
				strcat((char *)text, (char *)int_text);
				break;
			case 9: // Time
//				sprintf((char *)text, "%s%02d:%02d", item_name[i + item_index / 5 * 5], (uint16)calendar.hour, (uint16)calendar.min);
				 
//				itoa(calendar.hour, int_text, 0);
//				strcat((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)":");
//				itoa(calendar.min, int_text, 0); 
				sprintf((char *)int_text, "%02d:%02d",(uint16)calendar.hour, (uint16)calendar.min); 
				strcat((char *)text, (char *)int_text);
				break;
			case 10:	// Ring on time
//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_on_time);
				itoa(pre_alarm_on_time, int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 11: // Ring off time
//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_off_time);
				itoa(pre_alarm_off_time, int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 12:
//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], YesOrNo[use_password]);
				strcat((char *)text, (char *)YesOrNo[use_password]);
				break;
			case 13:
//				sprintf((char *)text, "%s%c%c%c%c", item_name[i + item_index / 5 * 5], user_password[0], user_password[1], user_password[2], user_password[3]);
//				strcpy((char *)text, (char *)item_name[i + item_index / 5 * 5]);
				strncat((char *)text, (char *)user_password, 4);
				break;
			case 14:
				if(modbus.protocal == MODBUS) 
					strcat((char *)text, (char *)protocol_text[1]);
				else
					strcat((char *)text, (char *)protocol_text[0]);
				break; 
		}
		
		if(i == (item_index % MAX_ROW))
			Lcd_Show_String(i, 0, DISP_INV, text);
		else
			Lcd_Show_String(i, 0, DISP_NOR, text);
	}

	in_sub_menu = FALSE;
}

void Misc_display(void)
{
	ipconfig_in_menu = ip_changed_in_menu | subnet_changed_in_menu | gateway_changed_in_menu | port_changed_in_menu;
	if(in_sub_menu == FALSE)
	{
		if(pre_item_index != item_index)
		{
			strcpy((char *)text, (char *)item_name[pre_item_index]);
			switch(pre_item_index)
			{
				case 0:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], ip_mode_text[modbus.ghost_ip_mode]);
					strcat((char *)text, (char *)ip_mode_text[modbus.ghost_ip_mode]);	
					break;
				case 1:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / 5 * 5], (uint16)modbus.ghost_ip_addr[0], (uint16)modbus.ghost_ip_addr[1], (uint16)modbus.ghost_ip_addr[2], (uint16)modbus.ghost_ip_addr[3]);
					itoa(modbus.ghost_ip_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_ip_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_ip_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_ip_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 2:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / 5 * 5], (uint16)modbus.ghost_mask_addr[0], (uint16)modbus.ghost_mask_addr[1], (uint16)modbus.ghost_mask_addr[2], (uint16)modbus.ghost_mask_addr[3]);
					itoa(modbus.ghost_mask_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_mask_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_mask_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_mask_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 3:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / 5 * 5], (uint16)modbus.ghost_gate_addr[0], (uint16)modbus.ghost_gate_addr[1], (uint16)modbus.ghost_gate_addr[2], (uint16)modbus.ghost_gate_addr[3]);
					itoa(modbus.ghost_gate_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_gate_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_gate_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_gate_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 4:
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], modbus.ghost_listen_port);
					itoa(modbus.ghost_listen_port, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 5:	
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], new_ip_status_text[ipconfig_in_menu]);
					strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);	
					break;
				case 6: // Modbus ID
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)modbus.address);
					itoa(modbus.address, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 7:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], baudrate_text[modbus.baud]);
					strcat((char *)text, (char *)baudrate_text[modbus.baud]);
				    strcat((char *)text, (char *)PARITY_TEXT[uart1_parity]);
					break;
				case 8: // Date
	//				sprintf((char *)text, "%s%04d-%02d-%02d", item_name[i + item_index / 5 * 5], (uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
//					itoa(calendar.w_year, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)"-");
//					itoa(calendar.w_month, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)"-");
//					itoa(calendar.w_date, int_text, 0);
					sprintf((char *)int_text, "%04d-%02d-%02d",(uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
					strcat((char *)text, (char *)int_text);
					break;
				case 9: // Time
	//				sprintf((char *)text, "%s%02d:%02d", item_name[i + item_index / 5 * 5], (uint16)calendar.hour, (uint16)calendar.min);
					 
//					itoa(calendar.hour, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)":");
//					itoa(calendar.min, int_text, 0);
					sprintf((char *)int_text, "%02d:%02d",(uint16)calendar.hour, (uint16)calendar.min);
					strcat((char *)text, (char *)int_text);
					break;
				case 10:	// Ring on time
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_on_time);
					itoa(pre_alarm_on_time, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 11: // Ring off time
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_off_time);
					itoa(pre_alarm_off_time, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 12:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], YesOrNo[use_password]);
					strcat((char *)text, (char *)YesOrNo[use_password]);
					break;
				case 13:
	//				sprintf((char *)text, "%s%c%c%c%c", item_name[i + item_index / 5 * 5], user_password[0], user_password[1], user_password[2], user_password[3]);
			 
					strncat((char *)text, (char *)user_password, 4);
					break;
				case 14:
					if(modbus.protocal == MODBUS) 
						strcat((char *)text, (char *)protocol_text[1]);
					else
						strcat((char *)text, (char *)protocol_text[0]);
					break; 
			}
			Lcd_Clear_Row(pre_item_index % MAX_ROW);
			Lcd_Show_String(pre_item_index % MAX_ROW, 0, DISP_NOR, text);
			strcpy((char *)text, (char *)item_name[item_index]);
			switch(item_index)
			{
				case 0:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], ip_mode_text[modbus.ghost_ip_mode]);
					strcat((char *)text, (char *)ip_mode_text[modbus.ghost_ip_mode]);	
					break;
				case 1:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / 5 * 5], (uint16)modbus.ghost_ip_addr[0], (uint16)modbus.ghost_ip_addr[1], (uint16)modbus.ghost_ip_addr[2], (uint16)modbus.ghost_ip_addr[3]);
					itoa(modbus.ghost_ip_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_ip_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_ip_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_ip_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 2:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / 5 * 5], (uint16)modbus.ghost_mask_addr[0], (uint16)modbus.ghost_mask_addr[1], (uint16)modbus.ghost_mask_addr[2], (uint16)modbus.ghost_mask_addr[3]);
					itoa(modbus.ghost_mask_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_mask_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_mask_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_mask_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 3:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / 5 * 5], (uint16)modbus.ghost_gate_addr[0], (uint16)modbus.ghost_gate_addr[1], (uint16)modbus.ghost_gate_addr[2], (uint16)modbus.ghost_gate_addr[3]);
					itoa(modbus.ghost_gate_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_gate_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_gate_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(modbus.ghost_gate_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 4:
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], modbus.ghost_listen_port);
					itoa(modbus.ghost_listen_port, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 5:	
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], new_ip_status_text[ipconfig_in_menu]);
					strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);	
					break;
				case 6: // Modbus ID
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)modbus.address);
					itoa(modbus.address, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 7:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], baudrate_text[modbus.baud]);
					strcat((char *)text, (char *)baudrate_text[modbus.baud]);
				    strcat((char *)text, (char *)PARITY_TEXT[uart1_parity]);
					break;
				case 8: // Date
	//				sprintf((char *)text, "%s%04d-%02d-%02d", item_name[i + item_index / 5 * 5], (uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
//					itoa(calendar.w_year, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)"-");
//					itoa(calendar.w_month, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)"-");
//					itoa(calendar.w_date, int_text, 0);
					sprintf((char *)int_text, "%04d-%02d-%02d",(uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
					strcat((char *)text, (char *)int_text);
					break;
				case 9: // Time
	//				sprintf((char *)text, "%s%02d:%02d", item_name[i + item_index / 5 * 5], (uint16)calendar.hour, (uint16)calendar.min);
					 
//					itoa(calendar.hour, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)":");
//					itoa(calendar.min, int_text, 0);
					sprintf((char *)int_text, "%02d:%02d",(uint16)calendar.hour, (uint16)calendar.min);
					strcat((char *)text, (char *)int_text);
					break;
				case 10:	// Ring on time
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_on_time);
					itoa(pre_alarm_on_time, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 11: // Ring off time
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_off_time);
					itoa(pre_alarm_off_time, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 12:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], YesOrNo[use_password]);
					strcat((char *)text, (char *)YesOrNo[use_password]);
					break;
				case 13:
	//				sprintf((char *)text, "%s%c%c%c%c", item_name[i + item_index / 5 * 5], user_password[0], user_password[1], user_password[2], user_password[3]);
				 
					strncat((char *)text, (char *)user_password, 4);
					break;
				case 14:
					if(modbus.protocal == MODBUS) 
						strcat((char *)text, (char *)protocol_text[1]);
					else
						strcat((char *)text, (char *)protocol_text[0]);
					break;
			}
			Lcd_Clear_Row(item_index % MAX_ROW);
 			Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);

			pre_item_index = item_index;
		}
		else
		{
//			if((item_index >= 5) && (item_index < 10))// in the time menu page
//			{
////				sprintf((char *)text, "%s%s", item_name[5], new_ip_status_text[ipconfig_in_menu]);
//				strcpy((char *)text, (char *)item_name[5]); 
//				strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);
//				if(item_index == 5)
//					Lcd_Show_String(5 % MAX_ROW, 0, DISP_INV, text);
//				else
//					Lcd_Show_String(5 % MAX_ROW, 0, DISP_NOR, text);

//		//		sprintf((char *)text, "%s%02d:%02d", item_name[9], (uint16)calendar.hour, (uint16)calendar.min);
//				strcpy((char *)text, (char *)item_name[item_index]);
//				itoa(calendar.hour, int_text, 0);
//				strcat((char *)text, (char *)int_text);
//				itoa(calendar.min, int_text, 0);
//				strcat((char *)text, (char *)int_text);
//				if(item_index == 9)
//					Lcd_Show_String(9 % MAX_ROW, 0, DISP_INV, text);
//				else
//					Lcd_Show_String(9 % MAX_ROW, 0, DISP_NOR, text);
//			}
		}
	}
	else
	{
		if(value_change == TRUE)
		{
			switch(item_index)
			{
				case 0:
 					sprintf((char *)text, "%s", ip_mode_text[set_value]); 
					Lcd_Show_String(item_index % MAX_ROW, 11, DISP_NOR,(uint8 *)"      ");
					Lcd_Show_String(item_index % MAX_ROW, 11, DISP_INV, text);
					break;
				case 1: // IP
					sprintf((char *)text, "%03d", set_value);
					if(sub_menu_index == 0)
						Lcd_Show_String(item_index % MAX_ROW, 5, DISP_INV, text);
					else if(sub_menu_index == 1)
						Lcd_Show_String(item_index % MAX_ROW, 9, DISP_INV, text);
					else if(sub_menu_index == 2)
						Lcd_Show_String(item_index % MAX_ROW, 13, DISP_INV, text);
					else
						Lcd_Show_String(item_index % MAX_ROW, 17, DISP_INV, text);
					break;
				case 2: // MASK
				case 3:	// GATE
					sprintf((char *)text, "%03d", set_value);
					if(sub_menu_index == 0)
						Lcd_Show_String(item_index % MAX_ROW, 6, DISP_INV, text);
					else if(sub_menu_index == 1)
						Lcd_Show_String(item_index % MAX_ROW, 10, DISP_INV, text);
					else if(sub_menu_index == 2)
						Lcd_Show_String(item_index % MAX_ROW, 14, DISP_INV, text);
					else
						Lcd_Show_String(item_index % MAX_ROW, 18, DISP_INV, text);
					break;
				case 4:	// PORT
					sprintf((char *)text, "%u", set_value);
					Lcd_Show_String(item_index % MAX_ROW, 7, DISP_NOR, (uint8 *)"     ");
					Lcd_Show_String(item_index % MAX_ROW, 7, DISP_INV, text);
					break;
				case 5:
					sprintf((char *)text, "%s", new_ip_status_text[set_value]);
					Lcd_Show_String(item_index % MAX_ROW, 10, DISP_NOR, (uint8 *)"         ");
					Lcd_Show_String(item_index % MAX_ROW, 10, DISP_INV, text);
					break;
				case 6:	// ID
					sprintf((char *)text, "%u", set_value);
					Lcd_Show_String(item_index % MAX_ROW, 13, DISP_NOR, (uint8 *)"   ");
					Lcd_Show_String(item_index % MAX_ROW, 13, DISP_INV, text);
					break;
				case 7:	// BAUDRATE
					Lcd_Show_String(item_index % MAX_ROW, 12, DISP_NOR, (uint8 *)"        ");
					Lcd_Show_String(item_index % MAX_ROW, 12, DISP_INV, (uint8 *)baudrate_text[set_value]);
					break;
				case 8:	// DATE
					
					if(sub_menu_index == 0)
					{
						sprintf((char *)text, "%04d", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 8, DISP_INV, text);
					}
					else if(sub_menu_index == 1)
					{
						sprintf((char *)text, "%02d", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 13, DISP_INV, text);
					}
					else if(sub_menu_index == 2)
					{
						sprintf((char *)text, "%02d", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 16, DISP_INV, text);
					}
					break;
				case 9:	// TIME
					sprintf((char *)text, "%02d", set_value);
					if(sub_menu_index == 0)
						Lcd_Show_String(item_index % MAX_ROW, 9, DISP_INV, text);
					else if(sub_menu_index == 1)
						Lcd_Show_String(item_index % MAX_ROW, 12, DISP_INV, text);
					break;
				case 10:	// RING ON TIME
					sprintf((char *)text, "%u", set_value);
					Lcd_Show_String(item_index % MAX_ROW, 17, DISP_NOR, (uint8 *)"   ");
					Lcd_Show_String(item_index % MAX_ROW, 17, DISP_INV, text);
					break;
				case 11:	// RING OFF TIME
					sprintf((char *)text, "%u", set_value);
					Lcd_Show_String(item_index % MAX_ROW, 18, DISP_NOR, (uint8 *)"   ");
					Lcd_Show_String(item_index % MAX_ROW, 18, DISP_INV, text);
					break;
				case 12: // PASSWORD ENABLE
					Lcd_Show_String(item_index % MAX_ROW, 17, DISP_NOR, (uint8 *)"   ");
					Lcd_Show_String(item_index % MAX_ROW, 17, DISP_INV, (uint8 *)YesOrNo[set_value]);
					break;
				case 13: // PASSWORD TEXT
					cursor.on_byte = (uint8)set_value;
					break;
				case 14:
					Lcd_Show_String(item_index % MAX_ROW, 13, DISP_NOR, (uint8 *)"      ");
					Lcd_Show_String(item_index % MAX_ROW, 13, DISP_INV, (uint8 *)protocol_text[set_value]); 
					break;
			}

			value_change = FALSE;
		}
	}
}

void Misc_keycope(uint16 key_value)
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
				ipconfig_in_menu = IP_DISABLE;
				update_menu_state(MenuMain);
			}
			else
			{
				if(((item_index < 1) || (item_index >= 4)) && (item_index != 13))
				{
					switch(item_index)
					{
						case 0:
							sprintf((char *)text, "%s%s", item_name[item_index], ip_mode_text[modbus.ghost_ip_mode]);
							break;
//						case 1:
//							sprintf(text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ip_address[0], (uint16)ip_address[1], (uint16)ip_address[2], (uint16)ip_address[3]);
//							break;
//						case 2:
//							sprintf(text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)subnet_mask_address[0], (uint16)subnet_mask_address[1], (uint16)subnet_mask_address[2], (uint16)subnet_mask_address[3]);
//							break;
//						case 3:
//							sprintf(text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)gateway_address[0], (uint16)gateway_address[1], (uint16)gateway_address[2], (uint16)gateway_address[3]);
//							break;
						case 4:
//							listen_port_at_tcp_server_mode_ghost[0] = listen_port_at_tcp_server_mode[0];
//							listen_port_at_tcp_server_mode_ghost[1] = listen_port_at_tcp_server_mode[1];
							sprintf((char *)text, "%s%u", item_name[item_index], modbus.ghost_listen_port);
							break;
						case 5:
							sprintf((char *)text, "%s%s", item_name[item_index], new_ip_status_text[ipconfig_in_menu]);
							break;
						case 6:
							sprintf((char *)text, "%s%u", item_name[item_index], (uint16)modbus.address);
							break;
						case 7:
							sprintf((char *)text, "%s%s", item_name[item_index], baudrate_text[modbus.baud]);
							break;
						case 8:
							sprintf((char *)text, "%s%04d-%02d-%02d", item_name[item_index], (uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
							break;
						case 9:
							sprintf((char *)text, "%s%02d:%02d", item_name[item_index], (uint16)calendar.hour, (uint16)calendar.min);
							break;
						case 10:
							sprintf((char *)text, "%s%u", item_name[item_index], (uint16)pre_alarm_on_time);
							break;
						case 11:
							sprintf((char *)text, "%s%u", item_name[item_index], (uint16)pre_alarm_off_time);
							break;
						case 12:
							sprintf((char *)text, "%s%s", item_name[item_index], YesOrNo[use_password]);
							break;
						case 14:
							//sprintf((char *)text, "%s", item_name[item_index]);
							if(modbus.protocal == MODBUS) 
								sprintf((char *)text, "%s%s", item_name[item_index], protocol_text[1]);
							else 
								sprintf((char *)text, "%s%s", item_name[item_index], protocol_text[0]);
							break;
					}
					Lcd_Clear_Row(item_index % MAX_ROW);
					Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
					in_sub_menu = FALSE;
				}
				else // in the password setting menu
				{
					switch(item_index)
					{
						case 1:
							switch(sub_menu_index)
							{
								case 0:
									sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)modbus.ghost_ip_addr[0], (uint16)modbus.ghost_ip_addr[1], (uint16)modbus.ghost_ip_addr[2], (uint16)modbus.ghost_ip_addr[3]);
									Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
									in_sub_menu = FALSE;
									break;
								case 1:
//									ip_address_ghost[1] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % MAX_ROW, 9, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)modbus.ghost_ip_addr[1]);
									Lcd_Show_String(item_index % MAX_ROW, 9, DISP_NOR, text);
		
									set_value = modbus.ghost_ip_addr[0];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % MAX_ROW, 5, DISP_INV, text);
		
									sub_menu_index = 0;
									break;
								case 2:
//									ip_address_ghost[2] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % MAX_ROW, 13, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)modbus.ghost_ip_addr[2]);
									Lcd_Show_String(item_index % MAX_ROW, 13, DISP_NOR, text);
		
									set_value = modbus.ghost_ip_addr[1];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % MAX_ROW, 9, DISP_INV, text);
		
									sub_menu_index = 1;
									break;
								case 3:
//									ip_address_ghost[3] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % MAX_ROW, 17, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)modbus.ghost_ip_addr[3]);
									Lcd_Show_String(item_index % MAX_ROW, 17, DISP_NOR, text);
		
									set_value = modbus.ghost_ip_addr[2];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % MAX_ROW, 13, DISP_INV, text);
		
									sub_menu_index = 2;
									break;
							}
							break;
						case 2:
							switch(sub_menu_index)
							{
								case 0:
									sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)modbus.ghost_mask_addr[0], (uint16)modbus.ghost_mask_addr[1], (uint16)modbus.ghost_mask_addr[2], (uint16)modbus.ghost_mask_addr[3]);
									Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
									in_sub_menu = FALSE;
									break;
								case 1:
//									subnet_mask_address_ghost[1] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % MAX_ROW, 10, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)modbus.ghost_mask_addr[1]);
									Lcd_Show_String(item_index % MAX_ROW, 10, DISP_NOR, text);
		
									set_value = modbus.ghost_mask_addr[0];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % MAX_ROW, 6, DISP_INV, text);
		
									sub_menu_index = 0;
									break;
								case 2:
//									subnet_mask_address_ghost[2] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % MAX_ROW, 14, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)modbus.ghost_mask_addr[2]);
									Lcd_Show_String(item_index % MAX_ROW, 14, DISP_NOR, text);
		
									set_value = modbus.ghost_mask_addr[1];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % MAX_ROW, 10, DISP_INV, text);
		
									sub_menu_index = 1;
									break;
								case 3:
//									subnet_mask_address_ghost[3] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % MAX_ROW, 18, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)modbus.ghost_mask_addr[3]);
									Lcd_Show_String(item_index % MAX_ROW, 18, DISP_NOR, text);
		
									set_value = modbus.ghost_mask_addr[2];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % MAX_ROW, 14, DISP_INV, text);
		
									sub_menu_index = 2;
									break;
							}
							break;
						case 3:
							switch(sub_menu_index)
							{
								case 0:
									sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)modbus.ghost_gate_addr[0], (uint16)modbus.ghost_gate_addr[1], (uint16)modbus.ghost_gate_addr[2], (uint16)modbus.ghost_gate_addr[3]);
									Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
									in_sub_menu = FALSE;
									break;
								case 1:
//									gateway_address_ghost[1] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % MAX_ROW, 10, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)modbus.ghost_gate_addr[1]);
									Lcd_Show_String(item_index % MAX_ROW, 10, DISP_NOR, text);
		
									set_value = modbus.ghost_gate_addr[0];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % MAX_ROW, 6, DISP_INV, text);
		
									sub_menu_index = 0;
									break;
								case 2:
//									gateway_address_ghost[2] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % MAX_ROW, 14, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)modbus.ghost_gate_addr[2]);
									Lcd_Show_String(item_index % MAX_ROW, 14, DISP_NOR, text);
		
									set_value = modbus.ghost_gate_addr[1];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % MAX_ROW, 10, DISP_INV, text);
		
									sub_menu_index = 1;
									break;
								case 3:
//									gateway_address_ghost[3] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % MAX_ROW, 18, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)modbus.ghost_gate_addr[3]);
									Lcd_Show_String(item_index % MAX_ROW, 18, DISP_NOR, text);
		
									set_value = modbus.ghost_gate_addr[2];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % MAX_ROW, 14, DISP_INV, text);
		
									sub_menu_index = 2;
									break;
							}
							break;
						case 13:
							if(password_index == 0)
							{
								cursor_off();
//								sprintf((char *)text, "%s%c%c%c%c", item_name[item_index], user_password[0], user_password[1], user_password[2], user_password[3]);
								strcpy((char *)text, (char *)item_name[item_index]);
								strncat((char *)text, (char *)user_password, 4);
								Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
								in_sub_menu = FALSE;
							}
							else
							{
								password_buffer[password_index] = set_value;
								Lcd_Write_Char(cursor.row, cursor.line--, set_value, DISP_INV);
		
								password_index--;
								set_value = password_buffer[password_index];
								cursor.on_byte = set_value;
							}
							break;
					}
				}
			}
			break;
		case KEY_RIGHT_MASK:
			// enter the sub menu of miscellaneous settings
			switch(item_index)
			{
				case 0:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = modbus.ghost_ip_mode;
						in_sub_menu = TRUE;
					}
					else
					{
						if(modbus.ghost_ip_mode != set_value)
						{
							modbus.ghost_ip_mode = set_value;
							modbus.write_ghost_system = 1; 
						}

						sprintf((char *)text, "%s%s", item_name[item_index], ip_mode_text[modbus.ghost_ip_mode]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);

						in_sub_menu = FALSE;
					}
					break;
				case 1: // ip
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)modbus.ghost_ip_addr[0], (uint16)modbus.ghost_ip_addr[1], (uint16)modbus.ghost_ip_addr[2], (uint16)modbus.ghost_ip_addr[3]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = modbus.ghost_ip_addr[0];
						sprintf((char *)text, "%03d", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 5, DISP_INV, text);

						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // first byte done
						{
							modbus.ghost_ip_addr[0] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 5, DISP_NOR, text);

							set_value = modbus.ghost_ip_addr[1];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 9, DISP_INV, text);

							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // second byte done
						{
							modbus.ghost_ip_addr[1] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 9, DISP_NOR, text);

							set_value = modbus.ghost_ip_addr[2];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 13, DISP_INV, text);

							sub_menu_index = 2;
						}
						else if(sub_menu_index == 2) // third byte done
						{
							modbus.ghost_ip_addr[2] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 13, DISP_NOR, text);

							set_value = modbus.ghost_ip_addr[3];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 17, DISP_INV, text);

							sub_menu_index = 3;
						}
						else if(sub_menu_index == 3)
						{
							modbus.ghost_ip_addr[3] = set_value;
								
							sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)modbus.ghost_ip_addr[0], (uint16)modbus.ghost_ip_addr[1], (uint16)modbus.ghost_ip_addr[2], (uint16)modbus.ghost_ip_addr[3]);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);

							if((modbus.ip_addr[0] != modbus.ghost_ip_addr[0]) || (modbus.ip_addr[1] != modbus.ghost_ip_addr[1]) || (modbus.ip_addr[2] != modbus.ghost_ip_addr[2]) || (modbus.ip_addr[3] != modbus.ghost_ip_addr[3]))
								ip_changed_in_menu = 1;
							else
								ip_changed_in_menu = 0;
							
							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
			   case 2: // subnet mask address
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)modbus.ghost_mask_addr[0], (uint16)modbus.ghost_mask_addr[1], (uint16)modbus.ghost_mask_addr[2], (uint16)modbus.ghost_mask_addr[3]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = modbus.ghost_mask_addr[0];
						sprintf((char *)text, "%03d", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 6, DISP_INV, text);

						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // first byte done
						{
							modbus.ghost_mask_addr[0] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 6, DISP_NOR, text);

							set_value = modbus.ghost_mask_addr[1];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 10, DISP_INV, text);

							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // second byte done
						{
							modbus.ghost_mask_addr[1] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 10, DISP_NOR, text);

							set_value = modbus.ghost_mask_addr[2];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 14, DISP_INV, text);

							sub_menu_index = 2;
						}
						else if(sub_menu_index == 2) // third byte done
						{
							modbus.ghost_mask_addr[2] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 14, DISP_NOR, text);

							set_value = modbus.ghost_mask_addr[3];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 18, DISP_INV, text);

							sub_menu_index = 3;
						}
						else if(sub_menu_index == 3)
						{
							modbus.ghost_mask_addr[3] = set_value;	
							
							sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)modbus.ghost_mask_addr[0], (uint16)modbus.ghost_mask_addr[1], (uint16)modbus.ghost_mask_addr[2], (uint16)modbus.ghost_mask_addr[3]);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);

							if((modbus.mask_addr[0] != modbus.ghost_mask_addr[0]) || (modbus.mask_addr[1] != modbus.ghost_mask_addr[1]) || (modbus.mask_addr[2] != modbus.ghost_mask_addr[2]) || (modbus.mask_addr[3] != modbus.ghost_mask_addr[3]))
								subnet_changed_in_menu = 1;
							else
								subnet_changed_in_menu = 0;

							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
				case 3: // gateway address
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)modbus.ghost_gate_addr[0], (uint16)modbus.ghost_gate_addr[1], (uint16)modbus.ghost_gate_addr[2], (uint16)modbus.ghost_gate_addr[3]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = modbus.ghost_gate_addr[0];
						sprintf((char *)text, "%03d", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 6, DISP_INV, text);

						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // first byte done
						{
							modbus.ghost_gate_addr[0] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 6, DISP_NOR, text);

							set_value = modbus.ghost_gate_addr[1];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 10, DISP_INV, text);

							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // second byte done
						{
							modbus.ghost_gate_addr[1] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 10, DISP_NOR, text);

							set_value = modbus.ghost_gate_addr[2];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 14, DISP_INV, text);

							sub_menu_index = 2;
						}
						else if(sub_menu_index == 2) // third byte done
						{
							modbus.ghost_gate_addr[2] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 14, DISP_NOR, text);

							set_value = modbus.ghost_gate_addr[3];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 18, DISP_INV, text);

							sub_menu_index = 3;
						}
						else if(sub_menu_index == 3)
						{
							modbus.ghost_gate_addr[3] = set_value;	
							
							sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)modbus.ghost_gate_addr[0], (uint16)modbus.ghost_gate_addr[1], (uint16)modbus.ghost_gate_addr[2], (uint16)modbus.ghost_gate_addr[3]);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);

							if((modbus.gate_addr[0] != modbus.ghost_gate_addr[0]) || (modbus.gate_addr[1] != modbus.ghost_gate_addr[1]) || (modbus.gate_addr[2] != modbus.ghost_gate_addr[2]) || (modbus.gate_addr[3] != modbus.ghost_gate_addr[3]))
								gateway_changed_in_menu = 1;
							else
								gateway_changed_in_menu = 0;

							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
				case 4:	// tcp port
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);
						set_value = modbus.ghost_listen_port; 
						in_sub_menu = TRUE;
					}
					else
					{
						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
 
						modbus.ghost_listen_port =  set_value;	
						if(modbus.ghost_listen_port != modbus.listen_port)
							port_changed_in_menu = 1;
						else
							port_changed_in_menu = 0;

						in_sub_menu = FALSE;
					}
					break;
				case 5:
					if(in_sub_menu == FALSE)
					{
						if(ipconfig_in_menu != IP_NO_CHANGE)
						{
							sprintf((char *)text, "%s", item_name[item_index]);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);
	
							set_value = ipconfig_in_menu;
							in_sub_menu = TRUE;
						}
					}
					else
					{
						sprintf((char *)text, "%s%s", item_name[item_index], new_ip_status_text[set_value]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);

						ipconfig_in_menu = set_value;
						ip_changed_in_menu =0;
						subnet_changed_in_menu =0;
						gateway_changed_in_menu =0;
						port_changed_in_menu = 0;
						if(ipconfig_in_menu == IP_DISABLE)
						{
							

							Misc_init();
							
						}
						else if(ipconfig_in_menu == IP_ENABLE)
						{
							modbus.write_ghost_system = 1;
							in_sub_menu = FALSE;
							ipconfig_in_menu = IP_NO_CHANGE;
						}
					}
					break;
				case 6:	// modbus id
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);
						set_value = modbus.address;
						in_sub_menu = TRUE;
					}
					else
					{
						if(modbus.address != set_value)	
							modify_master_id_in_database(modbus.address, set_value);

						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 7:	// baudrate
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = modbus.baud;
						sprintf((char *)text, "%s", baudrate_text[set_value]);
						Lcd_Show_String(item_index % MAX_ROW, 12, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						modbus.baud = set_value;
						sprintf((char *)text, "%s%s", item_name[item_index], baudrate_text[modbus.baud]);
						strcat((char *)text, (char *)PARITY_TEXT[uart1_parity]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
						in_sub_menu = FALSE;

						switch(modbus.baud)
						{
							case 0:
								modbus.baudrate = BAUDRATE_9600 ;
								uart1_init(BAUDRATE_9600);
						
								SERIAL_RECEIVE_TIMEOUT = 6;
							break ;
							case 1:
								modbus.baudrate = BAUDRATE_19200 ;
								uart1_init(BAUDRATE_19200);	
								SERIAL_RECEIVE_TIMEOUT = 3;
							break;
							case 2:
								modbus.baudrate = BAUDRATE_38400 ;
								uart1_init(BAUDRATE_38400);
								SERIAL_RECEIVE_TIMEOUT = 2;
							break;
							case 3:
								modbus.baudrate = BAUDRATE_57600 ;
								uart1_init(BAUDRATE_57600);	
								SERIAL_RECEIVE_TIMEOUT = 1;
							break;
							case 4:
								modbus.baudrate = BAUDRATE_115200 ;
								uart1_init(BAUDRATE_115200);	
								SERIAL_RECEIVE_TIMEOUT = 1;
							break;
							default:
							break ;				
						}
						AT24CXX_WriteOneByte(EEP_BAUDRATE,modbus.baud );
						modbus_init();
					}
					break;
				case 8: // date
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%04d-%02d-%02d", item_name[item_index], (uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = calendar.w_year;
						sprintf((char *)text, "%04d", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 8, DISP_INV, text);
						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // year done
						{
							calendar.w_year = set_value;
							Time_Adjust();							
							//rtc_set_time(RTC_YEAR, TOBCD(Modbus.Time.Clk.year));

							sprintf((char *)text, "%s%02d-%02d-%02d", item_name[item_index], (uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

							set_value = calendar.w_month;
							sprintf((char *)text, "%02d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 13, DISP_INV, text);
							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // month done
						{
							calendar.w_month = set_value;	
							Time_Adjust();
							//rtc_set_time(RTC_MONTH, TOBCD(Modbus.Time.Clk.mon));

							sprintf((char *)text, "%s%02d-%02d-%02d", item_name[item_index], (uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

							set_value = calendar.w_date;
							sprintf((char *)text, "%02d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 16, DISP_INV, text);
							sub_menu_index = 2;
						}
						else if(sub_menu_index == 2) // day done
						{
							calendar.w_date = set_value;	
							Time_Adjust();
							//rtc_set_time(RTC_DATE, TOBCD(Modbus.Time.Clk.day));

							sprintf((char *)text, "%s%02d-%02d-%02d", item_name[item_index], (uint16)calendar.w_year, (uint16)calendar.w_month, (uint16)calendar.w_date);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);

							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
				case 9: // time
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%02d:%02d", item_name[item_index], (uint16)calendar.hour, (uint16)calendar.min);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = calendar.hour;
						sprintf((char *)text, "%02d", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 9, DISP_INV, text);
						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // hour done
						{
							calendar.hour = set_value;	
							Time_Adjust();
							//rtc_set_time(RTC_HOUR, TOBCD(Modbus.Time.Clk.hour));

							sprintf((char *)text, "%s%02d:%02d", item_name[item_index], (uint16)calendar.hour, (uint16)calendar.min);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

							set_value = calendar.min;
							sprintf((char *)text, "%02d", set_value);
							Lcd_Show_String(item_index % MAX_ROW, 12, DISP_INV, text);
							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // minute done
						{
							calendar.min = set_value;
							Time_Adjust();							
							//rtc_set_time(RTC_MINUTE, TOBCD(Modbus.Time.Clk.min));

							sprintf((char *)text, "%s%02d:%02d", item_name[item_index], (uint16)calendar.hour, (uint16)calendar.min);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);

							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
				case 10: // ring on time
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = pre_alarm_on_time;
						sprintf((char *)text, "%u", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 17, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						pre_alarm_on_time = set_value;	
//						start_data_save_timer();
//						flash_write_int(FLASH_PRE_ALARM_SETTING_ON_TIME, pre_alarm_on_time);
						write_eeprom(EEP_PRE_ALARM_SETTING_ON_TIME,pre_alarm_on_time);  
						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 11: // ring off time
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = pre_alarm_off_time;
						sprintf((char *)text, "%u", set_value);
						Lcd_Show_String(item_index % MAX_ROW, 18, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						pre_alarm_off_time = set_value;	
//						start_data_save_timer();
//						flash_write_int(FLASH_PRE_ALARM_SETTING_OFF_TIME, pre_alarm_off_time);
						write_eeprom(EEP_PRE_ALARM_SETTING_OFF_TIME,pre_alarm_off_time);  
						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 12:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);

						set_value = use_password;
						sprintf((char *)text, "%s", YesOrNo[set_value]);
						Lcd_Show_String(item_index % MAX_ROW, 17, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						use_password = set_value;	
//						start_data_save_timer();
//						flash_write_int(FLASH_USE_PASSWORD, use_password);
						write_eeprom(EEP_USE_PASSWORD,use_password);
						sprintf((char *)text, "%s%s", item_name[item_index], YesOrNo[use_password]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 13:
					if(in_sub_menu == FALSE)
					{
 						sprintf((char *)text, "%s", item_name[item_index]);
//						strcpy((char *)text, (char *)item_name[item_index]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);
						
//						strncat((char *)text, (char *)user_password, 4);
//						text[4] = '\0';
 						sprintf((char *)text, "%c%c%c%c", user_password[0], user_password[1], user_password[2], user_password[3]);
						Lcd_Show_String(item_index % MAX_ROW, 13, DISP_INV, text);

						password_buffer[0] = user_password[0];
						password_buffer[1] = user_password[1];
						password_buffer[2] = user_password[2];
						password_buffer[3] = user_password[3];
						password_index = 0;
						set_value = password_buffer[password_index];
						cursor_on(item_index % MAX_ROW, 13, (uint8)set_value, ' ');

						in_sub_menu = TRUE;
					}
					else
					{
						if(password_index < MAX_PASSWORD_DIGITALS)
						{
							password_buffer[password_index++] = set_value;
							Lcd_Write_Char(cursor.row, cursor.line++, set_value, DISP_INV);
						}

						if(password_index == MAX_PASSWORD_DIGITALS)
						{
							cursor_off();
							user_password[0] = password_buffer[0];
							user_password[1] = password_buffer[1];
							user_password[2] = password_buffer[2];
							user_password[3] = password_buffer[3];
//							start_data_save_timer();
//							flash_write_int(FLASH_USER_PASSWORD0, user_password[0]);
//							flash_write_int(FLASH_USER_PASSWORD1, user_password[1]);
//							flash_write_int(FLASH_USER_PASSWORD2, user_password[2]);
//							flash_write_int(FLASH_USER_PASSWORD3, user_password[3]);
							write_eeprom(EEP_USER_PASSWORD0, user_password[0]);
							write_eeprom(EEP_USER_PASSWORD1, user_password[1]);
							write_eeprom(EEP_USER_PASSWORD2, user_password[2]);
							write_eeprom(EEP_USER_PASSWORD3, user_password[3]);	
 							//sprintf((char *)text, "%s%c%c%c%c", item_name[item_index], user_password[0], user_password[1], user_password[2], user_password[3]);
 							strcpy((char *)text, (char *)item_name[item_index]);
							strncat((char *)text, (char *)user_password, 4);
							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
//							Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, (uint8 *)"test");//text);

							in_sub_menu = FALSE;
						}
						else
						{
							set_value = password_buffer[password_index];
							cursor.on_byte = set_value;
						}
					}
					break;
				case 14:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_NOR, text);
						if(modbus.protocal == BAC_MSTP)
							set_value = 0;
						else
							set_value = 1;
						sprintf((char *)text, "%s", protocol_text[set_value]);
						Lcd_Show_String(item_index % MAX_ROW, 13, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						if(set_value) modbus.protocal = MODBUS;
						else
							modbus.protocal = BAC_MSTP;
//						start_data_save_timer();
//						flash_write_int(FLASH_USE_PASSWORD, use_password);
						AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, modbus.protocal);
						sprintf((char *)text, "%s%s", item_name[item_index], protocol_text[set_value]);
						Lcd_Show_String(item_index % MAX_ROW, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break; 
			}
			break;
		case KEY_UP_MASK:
			if(in_sub_menu == FALSE)
			{
				if((item_index %  MAX_ROW) == 0)	
				{
					if(item_index)
						item_index--;
					else
						item_index =  MAX_MISC_ITEMS - 1;

					Misc_init();
					pre_item_index = item_index;
				}
				else
				{
					if(item_index)
						item_index--;
					else
						item_index = MAX_MISC_ITEMS - 1;	
				}
//				if(item_index)
//					item_index--;
//				else
//					item_index = MAX_MISC_ITEMS - 1;
//				item_index %= MAX_MISC_ITEMS;

//				if((item_index % MAX_ROW) == 0)
//				{
//					Misc_init();
//					pre_item_index = item_index;
//				}
			}
			else
			{
				switch(item_index)
				{
					case 0:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 1:	// ip address
					case 2:	// subnet mask address
					case 3:	// gateway address
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
							set_value += SPEED_1;
						else
							set_value = (((set_value + SPEED_10) < 255) ? (set_value + SPEED_10) : 1);

						set_value %= 255;
						break;
					case 4:	// tcp port
						switch(key_value & KEY_FUNCTION_MASK)
						{
							case KEY_SPEED_100:
								set_value += SPEED_100;
								break;
							case KEY_SPEED_50:
								set_value += SPEED_50;
								break;
							case KEY_SPEED_10:
								set_value += SPEED_10;
								break;
							case KEY_SPEED_1:
							default:
								set_value += SPEED_1;
								break;
						}
						break;
					case 5:
						if(set_value != IP_ENABLE)
							set_value = IP_ENABLE;
						else
							set_value = IP_DISABLE;
						break;
					case 6: // modbus id setting, check in the database
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
							set_value += SPEED_1;
						else
							set_value = (((set_value + SPEED_10) < 255) ? (set_value + SPEED_10) : 1);

						set_value %= 255;
						set_value = check_master_id_in_database(set_value, 1);
						break;
					case 7:	// bardrate
						if(set_value >= 4)
							set_value = 0;
						else
							set_value++;
						break;
					case 8:
						set_value++;
						if(sub_menu_index == 0)
						{
							if(set_value > 2099) set_value = 2099;
							 //set_value %= 100;
						}
						else if(sub_menu_index == 1)
						{
							if(set_value == 13)
								set_value = 1;
						}
						else if(sub_menu_index == 2)
						{
							switch(calendar.w_month)
							{
								case 1:
								case 3:
								case 5:
								case 7:
								case 8:
								case 10:
								case 12:
									if(set_value > 31)
										set_value = 1;
									break;
								case 4:
								case 6:
								case 9:
								case 11:
									if(set_value > 30)
										set_value = 1;
									break;
								case 2:
									if(calendar.w_year % 4)
									{
										if(set_value > 29)
											set_value = 1;
									}
									else
									{
										if(set_value > 28)
											set_value = 1;
									}
									break;
							}
						}
						break;
					case 9:
						set_value++;
						if(sub_menu_index == 0)
							set_value %= 24;
						else if(sub_menu_index == 1)
							set_value %= 60;
						break;
					case 10:
					case 11:
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
							set_value += SPEED_1;
						else
							set_value = (((set_value + SPEED_10) < 255) ? (set_value + SPEED_10) : 0);
						set_value %= 256;
						
						if(set_value == 0)
							set_value = 1;

						break;
					case 12:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 13:
						if((set_value >= '9') || (set_value < '0'))
							set_value = '0';
						else
							set_value++;
						break;
					case 14:
						if(set_value == 1)
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
				item_index %= MAX_MISC_ITEMS;

				if((item_index % MAX_ROW) == 0)
				{
					Misc_init();
					pre_item_index = item_index;
				}
			}
			else
			{
				switch(item_index)
				{
					case 0:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 1:	// ip address
					case 2:	// subnet mask address
					case 3:	// gateway address
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
						{
							if(set_value)
								set_value -= SPEED_1;
							else
								set_value = 255;
						}
						else
							set_value = ((set_value > SPEED_10) ? (set_value - SPEED_10) : 255);
						break;
					case 4:	// tcp port
						switch(key_value & KEY_FUNCTION_MASK)
						{
							case KEY_SPEED_100:
								set_value -= SPEED_100;
								break;
							case KEY_SPEED_50:
								set_value -= SPEED_50;
								break;
							case KEY_SPEED_10:
								set_value -= SPEED_10;
								break;
							case KEY_SPEED_1:
							default:
								set_value -= SPEED_1;
								break;
						}
						break;
					case 5:
						if(set_value != IP_DISABLE)
							set_value = IP_DISABLE;
						else
							set_value = IP_ENABLE;
						break;
					case 6: // modbus id setting, check in the database
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
						{
							if(set_value)
								set_value -= SPEED_1;
							else
								set_value = 254;
						}
						else
							set_value = ((set_value > SPEED_10) ? (set_value - SPEED_10) : 254);

						set_value = check_master_id_in_database(set_value, 0);
						break;
					case 7:	// bardrate
						if(set_value == 0)
							set_value = 4;
						else
							set_value --;
						break;
					case 8:
						if(sub_menu_index == 0)
						{
							if(set_value > 2000)
								set_value--;
							else
								set_value = 2099;
						}
						else if(sub_menu_index == 1)
						{
							if(set_value > 1)
								set_value--;
							else
								set_value = 12;
						}
						else if(sub_menu_index == 2)
						{
							switch(calendar.w_month)
							{
								case 1:
								case 3:
								case 5:
								case 7:
								case 8:
								case 10:
								case 12:
									if(set_value > 1)
										set_value--;
									else
										set_value = 31;
									break;
								case 4:
								case 6:
								case 9:
								case 11:
									if(set_value > 1)
										set_value--;
									else
										set_value = 30;
									break;
								case 2:
									if(calendar.w_year % 4)
									{
										if(set_value > 1)
											set_value--;
										else
											set_value = 29;
									}
									else
									{
										if(set_value > 1)
											set_value--;
										else
											set_value = 28;
									}
									break;
							}
						}
						break;
					case 9:
						if(sub_menu_index == 0)
						{
							if(set_value > 0)
								set_value--;
							else
								set_value = 23;
						}
						else if(sub_menu_index == 1)
						{
							if(set_value > 0)
								set_value--;
							else
								set_value = 59;
						}
						break;
					case 10:
					case 11:
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
						{
							if(set_value)
								set_value -= SPEED_1;
							else
								set_value = 255;
						}
						else
							set_value = ((set_value > SPEED_10) ? (set_value - SPEED_10) : 255);
						
						if(set_value == 0)
							set_value = 255;

						break;
					case 12:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 13:
						if((set_value <= '0') || (set_value > '9'))
							set_value = '9';
						else
							set_value--;
						break;
					case 14:
						if(set_value == 1)
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




