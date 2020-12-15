#include "config.h"
#include "pm25.h"

//u8 const CMD_inquire[CMD_LENTH]   = {0xAA, 0xB4, 0x02, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF, 0xFF,   00, 0xAB}; 

//u8 const CMD_set_mode[CMD_LENTH]  = {0xAA, 0xB4, 0x02, 01, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xA1, 0x60, 0x05, 0xAB};

//u8 const CMD_set_period[CMD_LENTH]=	{0xAA, 0xB4, 0x08, 01, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xA1, 0x60, 0x0B, 0xAB};

//u8 const CMD_get_value[CMD_LENTH] = {0xAA, 0xB4, 0x04, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF, 0xFF, 0x02, 0xAB};	



//CDM FORMAT: Start, Adr, Cmd, Length, DataL, Check, Stop 
u8 const CMD_START_MEASUREMENT[8] = 	{0x7E, 0x00, 0x00, 0x02, 0x01, 0x03, 0xF9, 0x7E};
u8 const CMD_STOP_MEASUREMENT[6] = 		{0x7E, 0x00, 0x01, 0x00, 0xFE, 0x7E};
u8 const CMD_READ_MEASUREMENT[6] =	 	{0x7E, 0x00, 0x03, 0x00, 0xFC, 0x7E};
u8 const CMD_READ_AUTO_CLEAN[8] = 		{0x7E, 0x00, 0x80, 0x01, 0x00, 0x7D, 0x5E, 0x7E};
u8 const CMD_DISABLE_AUTO_CLEAN[11] = {0x7E, 0x00, 0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7A, 0x7E};
u8 const CMD_START_FAN_CLEAN[6] = 		{0x7E, 0x00, 0x56, 0x00, 0xA9, 0x7E};
u8 const PM25_CMD_RESET[6] = 							{0x7E, 0x00, 0xD3, 0x00, 0x2C, 0x7E};




u8 pm25_sensor_type = SENSIRION;
u8 pm25_current_cmd = SENSIRION_NULL;
u8 sensirion_rev_cnt = 0;
u8 sensirion_rev_end = 0;

uint16 pm25_number_05 = 0;
uint16 pm25_number_10 = 0;
uint16 pm25_number_25 = 0;
uint16 pm25_number_40 = 0;
uint16 pm25_number_100 = 0;
uint16 pm25_weight_10 = 0;
uint16 pm25_weight_25 = 0;
uint16 pm25_weight_40 = 0;
uint16 pm25_weight_100 = 0;

u8 shift_flag = 0;
u8 pm25_unit = NUM_CM3;
//u8 sensirion_rcv_buf[80];


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
uint16 pm25_org_value = 0;
uint16 pm10_org_value = 0;

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



void pm25_send_cmd(uint8 command)
{
	switch(command)
	{
		case SENSIRION_START_MEASUREMENT:
			sub_send_string((U8_T *)CMD_START_MEASUREMENT, 8);
		break;
		
		case SENSIRION_STOP_MEASUREMENT:
			sub_send_string((U8_T *)CMD_STOP_MEASUREMENT, 6);
		break;		
		
		case SENSIRION_READ_MEASUREMENT:
			sub_send_string((U8_T *)CMD_READ_MEASUREMENT, 6);
		break;
		
		case SENSIRION_READ_AUTO_CLEAN:
			sub_send_string((U8_T *)CMD_READ_AUTO_CLEAN, 8);
		break;
		
		case SENSIRION_DISABLE_AUTO_CLEAN:
			sub_send_string((U8_T *)CMD_DISABLE_AUTO_CLEAN, 11);
		break;
		
		case SENSIRION_START_FAN_CLEAN:
			sub_send_string((U8_T *)CMD_START_FAN_CLEAN, 6);
		break;
		
		case SENSIRION_RESET:
			sub_send_string((U8_T *)PM25_CMD_RESET, 6);
		break;

		default:
		break;
	
	}
}


  //----calculate float number-----------------------------------
float Datasum(uint8 FloatByte1, uint8 FloatByte2, uint8 FloatByte3, uint8 FloatByte4)
{   
	float aa;
	uint8 Sflag;
	uint16 Evalue;
	uint32 Mvalue;
	uint32 Mtemp;
	float mfloat = 0;
	uint32 Etemp;
	uint8 i;
	
	Sflag = 0x01& (FloatByte1 >> 7);//indicate it is positive or negative value
	
	Evalue = FloatByte1 & 0x7f;
	Evalue = Evalue<<1;
	if((FloatByte2 & 0x80) == 0x80)
		Evalue = Evalue | 0x01 ;
	else
		Evalue = Evalue & 0xfe ;
	
	Mvalue = FloatByte2 & 0x7f;
	Mvalue = (Mvalue << 16);
	Mvalue |= ((uint16)FloatByte3 << 8);
	Mvalue |= FloatByte4;
	

	for(i=0;i<23;i++)
	{
		Mtemp = (Mvalue >> i) & 0x01;
		if(Mtemp != 0)
			mfloat += (float)1/(Mtemp << (23-i));		
	}

	Etemp =  0x01 << (Evalue - 127);
	
	aa = (float)Etemp * (1 + mfloat);
	return aa;
}
   //end of calculate float value


u8 Process_Rece_Data(u8 *p)
{
	u8 check_sum;
	u8 i;
	uint16 pm25_org, pm100_org;
	int32 itemp; 	
	
	if(pm25_sensor_type == NOVA)
	{
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
			
			if((pm25_sensor.auto_manual&0x01)==0)
			{
				itemp = (p[CMD_DATA2] << 8) + p[CMD_DATA1]; 
				pm25_org_value = itemp;
				itemp *= (100 + pm25_sensor.pm25_offset);
				itemp /= 100;
				pm25_sensor.pre_pm25 = Sys_Filter(itemp,pm25_sensor.pre_pm25,pm25_sensor.PM25_filter);
				
				pm25_sensor.pm25 = pm25_sensor.pre_pm25;
				
				if(pm25_sensor.pm25 < 0) pm25_sensor.pm25 = 0;
			}
			if(((pm25_sensor.auto_manual>>1)&0x01)==0)
			{
				itemp = (p[CMD_DATA4] << 8) + p[CMD_DATA3]; 
				pm10_org_value = itemp;
				itemp *= (100 + pm25_sensor.pm10_offset);
				itemp /= 100;

				pm25_sensor.pre_pm10 = Sys_Filter(itemp,pm25_sensor.pre_pm10,pm25_sensor.PM10_filter);
				
				pm25_sensor.pm10 = pm25_sensor.pre_pm10;

				if(pm25_sensor.pm10 < 0) pm25_sensor.pm10 = 0;
			}
		}
		return 1;
	}
	else //sensirion sensor
	{
		
		if(pm25_current_cmd == SENSIRION_READ_MEASUREMENT)
		{
			if(p[0] != 0x7E || p[2] != 0x03 || p[4] != 0x28)
				return 0;
			
			if(pm25_unit == NUM_CM3)
			{
				pm25_org = (uint16)Datasum(p[DATA_OFFSET + 24],p[DATA_OFFSET + 25],p[DATA_OFFSET + 26],p[DATA_OFFSET + 27]);
				pm100_org = (uint16)Datasum(p[DATA_OFFSET + 32],p[DATA_OFFSET + 33],p[DATA_OFFSET + 34],p[DATA_OFFSET + 35]);
			}
			else
			{
				pm25_org= (int16)(Datasum(p[DATA_OFFSET + 4],p[DATA_OFFSET + 5],p[DATA_OFFSET + 6],p[DATA_OFFSET + 7])*10);
				pm100_org = (int16)(Datasum(p[DATA_OFFSET + 12],p[DATA_OFFSET + 13],p[DATA_OFFSET + 14],p[DATA_OFFSET + 15])*10);
			}
			
			check_sum = 0;
			for(i=1;i<sensirion_rev_cnt-1;i++)
			{
				check_sum += p[i];			
			}

			check_sum = ~check_sum;
			if(check_sum == p[sensirion_rev_cnt-1])
			{
				if(pm25_org != 0 && pm100_org != 0)
				{					
					if((pm25_sensor.auto_manual&0x01)==0)
					{
						itemp = pm25_org;//(p[CMD_DATA2] << 8) + p[CMD_DATA1]; 
						pm25_org_value = itemp;
						itemp *= (100 + pm25_sensor.pm25_offset);
						itemp /= 100;
						pm25_sensor.pre_pm25 = Sys_Filter(itemp,pm25_sensor.pre_pm25,pm25_sensor.PM25_filter);
						
						pm25_sensor.pm25 = pm25_sensor.pre_pm25;
						
						if(pm25_sensor.pm25 < 0) pm25_sensor.pm25 = 0;
					}					

				if(((pm25_sensor.auto_manual>>1)&0x01)==0)
				{
					itemp = pm100_org;//(p[CMD_DATA4] << 8) + p[CMD_DATA3]; 
					pm10_org_value = itemp;
					itemp *= (100 + pm25_sensor.pm10_offset);
					itemp /= 100;

					pm25_sensor.pre_pm10 = Sys_Filter(itemp,pm25_sensor.pre_pm10,pm25_sensor.PM10_filter);
					
					pm25_sensor.pm10 = pm25_sensor.pre_pm10;

					if(pm25_sensor.pm10 < 0) pm25_sensor.pm10 = 0;
				}					
					
					
					//pm25_sensor.pm25 = pm25_org;
					//pm25_sensor.pm10 = pm100_org;
					
					pm25_weight_10 = (uint16)Datasum(p[DATA_OFFSET + 0],p[DATA_OFFSET + 1],p[DATA_OFFSET + 2],p[DATA_OFFSET + 3]);
					pm25_weight_25 = (uint16)Datasum(p[DATA_OFFSET + 4],p[DATA_OFFSET + 5],p[DATA_OFFSET + 6],p[DATA_OFFSET + 7]);
					pm25_weight_40 = (uint16)Datasum(p[DATA_OFFSET + 8],p[DATA_OFFSET + 9],p[DATA_OFFSET + 10],p[DATA_OFFSET + 11]);
					pm25_weight_100 = (uint16)Datasum(p[DATA_OFFSET + 12],p[DATA_OFFSET + 13],p[DATA_OFFSET + 14],p[DATA_OFFSET + 15]);
					
					pm25_number_05 = (uint16)Datasum(p[DATA_OFFSET + 16],p[DATA_OFFSET + 17],p[DATA_OFFSET + 18],p[DATA_OFFSET + 19]);
					pm25_number_10 = (uint16)Datasum(p[DATA_OFFSET + 20],p[DATA_OFFSET + 21],p[DATA_OFFSET + 22],p[DATA_OFFSET + 23]);
					pm25_number_25 = (uint16)Datasum(p[DATA_OFFSET + 24],p[DATA_OFFSET + 25],p[DATA_OFFSET + 26],p[DATA_OFFSET + 27]);
					pm25_number_40 = (uint16)Datasum(p[DATA_OFFSET + 28],p[DATA_OFFSET + 29],p[DATA_OFFSET + 30],p[DATA_OFFSET + 31]);
					pm25_number_100 = (uint16)Datasum(p[DATA_OFFSET + 32],p[DATA_OFFSET + 33],p[DATA_OFFSET + 34],p[DATA_OFFSET + 35]);					
					
					
					// pm25_weight_25;
				}
			}

			return 1;
		
		}
	}
}

void pm25_initial(void)
{
	uint8 i;
	
	if(pm25_sensor_type == SENSIRION)
		set_sub_serial_baudrate(115200);
	else	
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
	pm25_sensor.pm25_range = PM25_0_100;
	
	if(pm25_sensor_type == SENSIRION)
		pm25_sensor.menu.menu_set = read_eeprom(EEP_MENU_SET) & 0xf3;
	else
		pm25_sensor.menu.menu_set = read_eeprom(EEP_MENU_SET);
	
	pm25_sensor.menu.scroll_set = read_eeprom(EEP_SCROLL_SET);
	pm25_sensor.menu.seconds = read_eeprom(EEP_MENU_SWITCH_SECONDS);
	pm25_sensor.pm25_offset = read_eeprom(EEP_PM25_OFFSET) |(int16)(read_eeprom(EEP_PM25_OFFSET+1)<<8);
	pm25_sensor.pm10_offset = read_eeprom(EEP_PM10_OFFSET) |(int16)(read_eeprom(EEP_PM10_OFFSET+1)<<8);
	pm25_sensor.PM25_filter = read_eeprom(EEP_PM25_FILTER);
	pm25_sensor.PM10_filter = read_eeprom(EEP_PM10_FILTER);
	pm25_sensor.pm25_range = read_eeprom(EEP_PM25_RANGE);
	pm25_sensor.AQI_area = read_eeprom(EEP_PM25_AREA);
	
	for(i = 0;i<5;i++)
	{
		aqi_table_customer[i] = (uint16)(read_eeprom(EEP_AQI_FIRST_LINE_LO+i*2+1)<<8)+read_eeprom(EEP_AQI_FIRST_LINE_LO+i*2);
		if((aqi_table_customer[4]>500)||(aqi_table_customer[i] == 0))
		{
			aqi_table_customer[0] = 12;
			aqi_table_customer[1] = 35;
			aqi_table_customer[2] = 55;
			aqi_table_customer[3] = 150;
			aqi_table_customer[4] = 250;
			write_eeprom(EEP_AQI_FIRST_LINE_LO, 12);
			write_eeprom(EEP_AQI_FIRST_LINE_HI, 0);
			write_eeprom(EEP_AQI_SECOND_LINE_LO, 35);
			write_eeprom(EEP_AQI_SECOND_LINE_HI, 0);
			write_eeprom(EEP_AQI_THIRD_LINE_LO, 55);
			write_eeprom(EEP_AQI_THIRD_LINE_HI, 0);
			write_eeprom(EEP_AQI_FOURTH_LINE_LO, 150);
			write_eeprom(EEP_AQI_FOURTH_LINE_HI, 0);
			write_eeprom(EEP_AQI_FIFTH_LINE_LO, 250);
			write_eeprom(EEP_AQI_FIFTH_LINE_HI, 0);
		}
	}
	
	if(pm25_sensor.pm10_offset > 200)
		pm25_sensor.pm10_offset = 0;
	if(pm25_sensor.pm10_offset < -200)
		pm25_sensor.pm10_offset = 0;
	if(pm25_sensor.pm25_offset > 200)
		pm25_sensor.pm25_offset = 0;
	if(pm25_sensor.pm25_offset < -200)
		pm25_sensor.pm25_offset = 0;
	
	if(pm25_sensor.PM25_filter == 0xff)
		pm25_sensor.PM25_filter = 5;
	if(pm25_sensor.PM10_filter == 0xff)
		pm25_sensor.PM10_filter = 5;
	
	if(pm25_sensor.pm25_range > PM25_0_1000)
		pm25_sensor.pm25_range = PM25_0_100;
	
	if(pm25_sensor.AQI_area >3)
		pm25_sensor.AQI_area = 0;
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
	write_eeprom(EEP_PM25_RANGE, PM25_0_100);
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
	if(pm25_sensor_type == NOVA)
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
	else
	{
		pm25_current_cmd = SENSIRION_READ_MEASUREMENT;
		pm25_send_cmd(SENSIRION_READ_MEASUREMENT);
		
		serial1_restart(); 
		
		delay_ms(100);
		
		if(sensirion_rev_end == PACKAGE_END)//one pacage received
		{
			Process_Rece_Data(subnet_response_buf);
			sensirion_rev_cnt = 0;
			sensirion_rev_end = 0;
		}
		return 1;
	}
}

void PM25_task(void *pvParameters)
{  
	portTickType xDelayPeriod = (portTickType)3000 / portTICK_RATE_MS;
	uint8 PM25_read_ctr,timer_counter; 
	pm25_initial();  
	PM25_read_ctr = 0; 
	timer_counter = 0;
	delay_ms(3000);
	
	if(pm25_sensor_type == SENSIRION)
	{
		pm25_current_cmd = SENSIRION_START_MEASUREMENT;
		pm25_send_cmd(SENSIRION_START_MEASUREMENT);
		serial1_restart();
	}
	delay_ms(1000);
	
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
				pm25_sensor.menu.display = pm25_sensor.menu.menu_set & (0x01<<(i++%5));
				
				if(pm25_sensor.menu.display != 0) break;
				
				j++;
				if(j == 5)
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





