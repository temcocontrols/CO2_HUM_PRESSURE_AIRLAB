#include "config.h"

//u8 const CMD_inquire[CMD_LENTH]   = {0xAA, 0xB4, 0x02, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF, 0xFF,   00, 0xAB}; 

//u8 const CMD_set_mode[CMD_LENTH]  = {0xAA, 0xB4, 0x02, 01, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xA1, 0x60, 0x05, 0xAB};

//u8 const CMD_set_period[CMD_LENTH]=	{0xAA, 0xB4, 0x08, 01, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xA1, 0x60, 0x0B, 0xAB};

//u8 const CMD_get_value[CMD_LENTH] = {0xAA, 0xB4, 0x04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF, 0xFF, 0x02, 0xAB};	

enum{
	CMD_HEAD = 0,
	CMD_NUM,
	CMD_DATA1,
	CMD_DATA2,
	CMD_DATA3,
	CMD_DATA4, 
	CMD_DATA5,
	CMD_DATA6,
	CMD_DATA14 = 15,
	CMD_DATA15 = 16,
	CMD_SUM,
	CMD_END,	 
};
enum{
CMD_RPL_SUM =8,
CMD_RPL_END =9,
};
	
u8 CMD_PM25[CMD_LENTH];

STR_PM25 pm25_sensor;
#define CMD_READ		0
#define CMD_SET			1

#define REPORT_MODE		0
#define INQUIRE_MODE	1

#define CMD_INQUIRE		0x02
#define CMD_SET_PERIOD	0x08
#define CMD_GET_VALUE	0x04
void Send_PM25_CMD(u16 ID,u8 CMD, u8 operate,u8 para)
{
	u8 i;
	CMD_PM25[CMD_HEAD] = 0xAA;
	CMD_PM25[CMD_NUM]  = 0xB4;
	
	CMD_PM25[CMD_DATA1]= CMD;
	CMD_PM25[CMD_DATA2]= operate;
	CMD_PM25[CMD_DATA3]= para;
	for(i = CMD_DATA4;i<CMD_DATA14;i++) CMD_PM25[i] = 0;
	CMD_PM25[CMD_DATA14] = ID >> 8;	//BroadCast ID
	CMD_PM25[CMD_DATA15] = ID;	//BroadCast ID
	
	CMD_PM25[CMD_SUM] = 0;
	for(i=CMD_DATA1;i<=CMD_DATA15;i++) CMD_PM25[CMD_SUM] += CMD_PM25[i];
	
	CMD_PM25[CMD_END] = 0xAB;
	
	sub_send_string((U8_T *)CMD_PM25, CMD_LENTH); 
}


u8 Process_Rece_Data(u8 *p)
{
	u8 check_sum;
	u8 i; 
	
	if(p[CMD_HEAD] != 0xAA)	return 0;
	
	if((p[CMD_NUM] != 0xC5)&&(p[CMD_NUM] != 0xC0))  return 0; 
	
	check_sum = 0;
	for(i=CMD_DATA1;i<=CMD_DATA6;i++)
		check_sum += p[i];
	if(check_sum != p[CMD_RPL_SUM]) return 0;
	
	if(p[CMD_RPL_END] != 0xAB)	return 0;
	
	if(p[CMD_NUM] == 0xC5)
	{
		switch(p[CMD_DATA1])
		{
			case CMD_INQUIRE:		//READ mode
				pm25_sensor.id = (p[CMD_DATA5] << 8) + p[CMD_DATA6];
				pm25_sensor.mode = p[CMD_DATA3]; 
				break;
			case CMD_SET_PERIOD:	//set sleep time
				pm25_sensor.period_spare =  p[CMD_DATA3]; 
				break;
			
			default: break;
		}
	}
	else	//read sensor value
	{
		int16 itemp;
		if((pm25_sensor.auto_manual&0xfe)==0)
		{
			itemp = (p[CMD_DATA2] << 8) + p[CMD_DATA1]; 
			itemp += pm25_sensor.pm25_offset;
			
			pm25_sensor.pre_pm25 = Sys_Filter(itemp,pm25_sensor.pre_pm25,pm25_sensor.PM25_filter);
			
			pm25_sensor.pm25 = pm25_sensor.pre_pm25;
			
			if(pm25_sensor.pm25 < 0) pm25_sensor.pm25 = 0;
		}
		if(((pm25_sensor.auto_manual>>1)&0xfe)==0)
		{
			itemp = (p[CMD_DATA4] << 8) + p[CMD_DATA3]; 
			itemp += pm25_sensor.pm10_offset;
			
			pm25_sensor.pre_pm10 = Sys_Filter(itemp,pm25_sensor.pre_pm10,pm25_sensor.PM10_filter);
			
			pm25_sensor.pm10 = pm25_sensor.pre_pm10;
			
			if(pm25_sensor.pm10 < 0) pm25_sensor.pm10 = 0;
		}
	}
	return 1;
}

void pm25_initial(void)
{
	set_sub_serial_baudrate(9600);
	pm25_sensor.pm25 = 0;
	pm25_sensor.pm10 = 0;
	pm25_sensor.id = 0;
	pm25_sensor.mode = 0;
	pm25_sensor.period = read_eeprom(EEP_SENSOR_WORK_PERIOD);
	if(pm25_sensor.period > MAX_WORK_PERIOD) pm25_sensor.period = 1;
	pm25_sensor.period_spare = 0xff;
	pm25_sensor.status = 0;
	pm25_sensor.cmd_status = 0;
	pm25_sensor.menu.menu_set = read_eeprom(EEP_MENU_SET);
	pm25_sensor.menu.scroll_set = read_eeprom(EEP_SCROLL_SET);
	pm25_sensor.menu.seconds = read_eeprom(EEP_MENU_SWITCH_SECONDS);
	pm25_sensor.pm25_offset = read_eeprom(EEP_PM25_OFFSET) |(int16)(read_eeprom(EEP_PM25_OFFSET+1)<<8);
	pm25_sensor.pm10_offset = read_eeprom(EEP_PM10_OFFSET) |(int16)(read_eeprom(EEP_PM10_OFFSET+1)<<8);
	pm25_sensor.PM25_filter = read_eeprom(EEP_PM25_FILTER);
	pm25_sensor.PM10_filter = read_eeprom(EEP_PM10_FILTER);
}
void pm25_reset_factory(void)
{
	write_eeprom(EEP_SENSOR_WORK_PERIOD,1); 
	write_eeprom(EEP_MENU_SET,7);
	write_eeprom(EEP_SCROLL_SET,7);
	write_eeprom(EEP_MENU_SWITCH_SECONDS,5);
	write_eeprom(EEP_PM25_OFFSET,0);
	write_eeprom(EEP_PM25_OFFSET+1,0);
	write_eeprom(EEP_PM10_OFFSET,0);
	write_eeprom(EEP_PM10_OFFSET+1,0); 
	write_eeprom(EEP_PM25_FILTER,DEFAULT_FILTER);
	write_eeprom(EEP_PM10_FILTER,DEFAULT_FILTER);
}
uint8 wait_pm25_response(u8 nDoubleTick)
{ 
	uint8 i;
	
	for(i=0;i<nDoubleTick;i++)
	{
		if(get_uart2_length() >= PM25_PEL_LENTH) 
			return 1; 
		delay_ms(10); 
	}
	return 0;
}
int8 pm25_request(void)
{  
	if(pm25_sensor.id == 0)	//inquire the pm25 sensor id
	{
		Send_PM25_CMD(0xffff,CMD_INQUIRE,CMD_READ,0); 	
		pm25_sensor.cmd_status = 1;
	}
	else if(pm25_sensor.mode != INQUIRE_MODE)
	{
		Send_PM25_CMD(pm25_sensor.id,CMD_INQUIRE,CMD_SET,INQUIRE_MODE); 
		pm25_sensor.cmd_status = 2;
	}
	else if(pm25_sensor.period != pm25_sensor.period_spare)//set sleep period
	{
		Send_PM25_CMD(pm25_sensor.id,CMD_SET_PERIOD,CMD_SET,pm25_sensor.period); 
		pm25_sensor.cmd_status = 3;
	}
	else
	{
		Send_PM25_CMD(pm25_sensor.id,CMD_GET_VALUE,CMD_READ,0); 
		pm25_sensor.cmd_status = 4;
	}
	  
	serial1_restart(); 
	
	if( wait_pm25_response(110) != 0)
	{ 
		Process_Rece_Data(subnet_response_buf);  
	}
	else
	{
		return 0;
	}	 
	return 1;
}

void PM25_task(void *pvParameters)
{  
	portTickType xDelayPeriod = (portTickType)1000 / portTICK_RATE_MS;
	uint8 PM25_read_ctr,timer_counter; 
	pm25_initial();  
	PM25_read_ctr = 0; 
	timer_counter = 0;
	delay_ms(100);
	
	while(1)
	{ 
		if((timer_counter % 5) == 0)
		{	
			timer_counter = 0;
			if(pm25_request())
			{
				PM25_read_ctr = 0;
				pm25_sensor.status = 1;  
				get_aqi_value(pm25_sensor.pm25, &pm25_sensor.AQI,&pm25_sensor.level); 
			}
			else
			{
				if(PM25_read_ctr < 20) PM25_read_ctr++;
				else
					pm25_sensor.status = 0; 
			}
		}
		
		if((timer_counter % pm25_sensor.menu.seconds) == 0)
		{
			static u8 i = 0;
			u8 j =0; 
			
			for(;;)
			{
				pm25_sensor.menu.display = pm25_sensor.menu.menu_set & (0x01<<(i++%3));
				
				if(pm25_sensor.menu.display != 0) break;
				
				j++;
				if(j == 3)
				{
					pm25_sensor.menu.display = 1; //the default display
					break;
				}
			}
		}
		 
		timer_counter++;
		update_message_context();
		vTaskDelay(xDelayPeriod);
	}
}



