
#include "config.h" 
#include "sht3x.h"
//uint16 light_org = 0;
uint16 hum_read_delay = 1;
extern uint8 update_flag;
extern void watchdog(void);
extern uint8 rx_icon;
extern uint8 tx_icon;


STR_LIGHT_SENSOR light;
uint16 internal_temp_ad = 0;


float tem_org = 0;
float hum_org = 0;
bit hum_exists = 0;
bit table_sel_enable = 1;

uint8 xdata display_state = 0;
uint8 xdata table_sel;
uint8 xdata hum_size_copy;
uint16 xdata hum_table[10][2];
int16 xdata humidity_back;

float  xdata k_line;
float  xdata b_line;
uint8   Run_Timer; 

uint8 temperature_sensor_select = EXTERNAL_TEMPERATURE_SENSOR;
int16 temperature;
//int16 temperature_f, temperature_c;
//uint8 temperature_filter_bypass = 5;
uint8 Temperature_Filter;
int16 internal_temperature_c = 0;
int16 internal_temperature_f = 0;
int16 internal_temperature_offset = 0;
uint8 deg_c_or_f = 0; // 0 = c, 1 = f
uint8 pre_deg_c_or_f = 2;// init state
int16 previous_temperature;
uint8 analog_output_sel;
uint8 auto_heat_enable;
//uint8 const code def_tab_pic[15] =
//{	// 10k termistor GREYSTONE -40 to 120 Deg.C or -40 to 248 Deg.F 
//	25, 39, 61, 83, 102, 113, 112, 101, 85, 67, 51, 38, 28, 21, 65
//};
uint16 const code def_tab_pic[33] =
{	// 10k termistor GREYSTONE -40 to 120 Deg.C or -40 to 248 Deg.F 
	110,142,182,227,278,333,389,444,492,531,558,570,567,551,524,488,446,401,
	357,315,275,239,206,178,153,130,112,96,83,71,61,53,383

};



uint16 look_up_table(uint16 count)
{
	uint16 val, work_var;
    uint8 index = 32;
	
	count *= 10;
	
	work_var = def_tab_pic[index];
	if(work_var >= count)
	{
		val = 1200;//the max T is 120 C
		return val;
	} 
	do 
	{
		index--;
//		co2_fb_i = index;  //test
		work_var += def_tab_pic[index];
//		temp_fb_i = work_var;  //test
		if(work_var > count)
		{
			val = (work_var - count) * 50;
			val /= def_tab_pic[index];
 
//			hum_fb_i = val;		//test
			if(index >= 8)
			{
				val += (index - 8) * 50;
				val &= 0x7fff;
			}
			else
			{
				val += index * 50;
				val = 400 - val;
				val |= 0x8000;
			}			 
			return val;
		}
	} while(index);
	
	val = 400 ;
	val |= 0x8000;
	return val;
}

 




static void reset_to_factory(void)
	{
		u8 i;
//reset humidity sensor		
		HumSensor.offset_h_default = 0;
		new_write_eeprom(EEP_CAL_DEFAULT_HUM, 0);
		new_write_eeprom(EEP_CAL_DEFAULT_HUM+1, 0);
		HumSensor.offset_t = 0;
		new_write_eeprom(EEP_TEMP_OFFSET,0);
		new_write_eeprom(EEP_TEMP_OFFSET+1,0); 
		HumSensor.offset_h = 0;
		new_write_eeprom(EEP_HUM_OFFSET+1,0);
		new_write_eeprom(EEP_HUM_OFFSET,0);  
		table_sel = FACTORY,
		new_write_eeprom(EEP_TABLE_SEL,table_sel);  
		new_write_eeprom(EEP_USER_POINTS,0);    //hum_size_copy
		new_write_eeprom(EEP_CAL_FAC_PTS,0);	//HumSensor.counter
		write_eeprom(EEP_DEG_C_OR_F, 0); 
		write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 0, 0);
		write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 1, 0);
		write_eeprom(EEP_SENSOR_SELECT, EXTERNAL_TEMPERATURE_SENSOR);
		  
//		output_range_table[CHANNEL_CO2].max = 1000;
//		write_eeprom(EEP_OUTPUT_CO2_RANGE_MAX, output_range_table[CHANNEL_CO2].max);
//		write_eeprom(EEP_OUTPUT_CO2_RANGE_MAX + 1, output_range_table[CHANNEL_CO2].max >>8); 
		
		 

//reset co2 sensor	 
		write_eeprom(EEP_CO2_FILTER,DEFAULT_FILTER);  
		
		int_co2_str.alarm_setpoint = 1000;
		write_eeprom(EEP_INT_ALARM_SETPOINT, int_co2_str.alarm_setpoint);
		write_eeprom(EEP_INT_ALARM_SETPOINT + 1, int_co2_str.alarm_setpoint >>8); 
		
		int_co2_str.pre_alarm_setpoint = 800;
		write_eeprom(EEP_INT_PRE_ALARM_SETPOINT, int_co2_str.pre_alarm_setpoint);
		write_eeprom(EEP_INT_PRE_ALARM_SETPOINT + 1, int_co2_str.pre_alarm_setpoint >>8); 
		
		int_co2_str.co2_offset = 0;
		write_eeprom(EEP_INT_CO2_OFFSET, int_co2_str.co2_offset);
		write_eeprom(EEP_INT_CO2_OFFSET + 1, int_co2_str.co2_offset >>8); 
		
		
	 

	    write_eeprom(EEP_PRE_ALARM_SETTING_ON_TIME,2);  
	    write_eeprom(EEP_PRE_ALARM_SETTING_OFF_TIME,2); 
		write_eeprom(EEP_ALARM_DELAY_TIME, 5);   
		write_eeprom(EEP_BACKLIGHT_KEEP_SECONDS,BACKLIGHT_KEEP_SECONDS_DEFAULT);
		

// co2 output range
		write_eeprom(EEP_OUTPUT_CO2_RANGE_MIN + 1,0);
		write_eeprom(EEP_OUTPUT_CO2_RANGE_MIN,0);

//		#ifdef CO2_SENSOR
		output_range_table[CHANNEL_CO2].max = 2000;
//		#else 
//		output_range_table[CHANNEL_CO2].max = 1000;
//		#endif
		write_eeprom(EEP_OUTPUT_CO2_RANGE_MAX + 1,output_range_table[CHANNEL_CO2].max>>8) ;
		write_eeprom(EEP_OUTPUT_CO2_RANGE_MAX,output_range_table[CHANNEL_CO2].max);
  
// temperature output range
		write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN + 1,0);
		write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN,0); 
		output_range_table[CHANNEL_TEMP].max = 1000;
		write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX + 1,output_range_table[CHANNEL_TEMP].max>>8);
		write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX,output_range_table[CHANNEL_TEMP].max);
	    
// humidity output range
		write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN + 1,0);
		write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN,0); 
		output_range_table[CHANNEL_HUM].max = 1000;
		write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX + 1,output_range_table[CHANNEL_HUM].max >>8);
		write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX,output_range_table[CHANNEL_HUM].max ); 	
//pass word
		use_password = 0;
		write_eeprom(EEP_USE_PASSWORD,use_password);
		user_password[0] = '1';
		user_password[1] = '2';
		user_password[2] = '3';
		user_password[3] = '4';
		write_eeprom(EEP_USER_PASSWORD0, user_password[0]);
		write_eeprom(EEP_USER_PASSWORD1, user_password[1]);
		write_eeprom(EEP_USER_PASSWORD2, user_password[2]);
		write_eeprom(EEP_USER_PASSWORD3, user_password[3]);	
		
		
		write_eeprom(EEP_BACKLIGHT_KEEP_SECONDS,BACKLIGHT_KEEP_SECONDS_DEFAULT);  
		write_eeprom(EEP_HUMIDITY_FILTER,DEFAULT_FILTER);   
		write_eeprom(EEP_EXT_TEMPERATURE_FILTER,DEFAULT_FILTER);   
		write_eeprom(EEP_INT_TEMPERATURE_FILTER,DEFAULT_FILTER);  
		 
		if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
			sprintf((char *)panelname,"%s", (char *)"CO2_NET");
		else if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
			sprintf((char *)panelname,"%s", (char *)"Pressure");
		else //((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
			sprintf((char *)panelname,"%s", (char *)"Humdity");
					 
		for(i=0;i<20;i++)			 
		{
			write_eeprom((EEP_TSTAT_NAME1 + i),panelname[i]); 
		}
		 
//		output_offset[0][CHANNEL_HUM]  
		write_eeprom(EEP_OUTPUT_HUM_VOL_OFFSET ,0);
		write_eeprom(EEP_OUTPUT_HUM_VOL_OFFSET + 1,0);	 
//		output_offset[0][CHANNEL_TEMP] 
		write_eeprom(EEP_OUTPUT_TEMP_VOL_OFFSET ,0);
		write_eeprom(EEP_OUTPUT_TEMP_VOL_OFFSET + 1,0);	 
//		output_offset[0][CHANNEL_CO2]  
		write_eeprom(EEP_OUTPUT_CO2_VOL_OFFSET ,0);
		write_eeprom(EEP_OUTPUT_CO2_VOL_OFFSET + 1,0);	  	
//		output_offset[1][CHANNEL_HUM] 
		write_eeprom(EEP_OUTPUT_HUM_CUR_OFFSET ,0);
		write_eeprom(EEP_OUTPUT_HUM_CUR_OFFSET + 1,0);	  
//		output_offset[1][CHANNEL_TEMP] 
		write_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET ,0);
		write_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET + 1,0);	 
//	    output_offset[1][CHANNEL_CO2] 
		write_eeprom(EEP_OUTPUT_CO2_CUR_OFFSET ,0);
		write_eeprom(EEP_OUTPUT_CO2_CUR_OFFSET + 1,0);
		
		write_eeprom(EEP_OUTPUT_SEL,0);
		
		write_eeprom(EEP_REPLY_DELAY,10);
		write_eeprom(EEP_RECEIVE_DELAY,10);
		 
		write_eeprom(EEP_UART1_PARITY,NONE_PARITY);
		
		auto_heat_enable = 0; 
		write_eeprom(EEP_AUTO_HEAT_CONTROL,0);
		
		light.filter = DEFAULT_FILTER;
		write_eeprom(EEP_LIGHT_FILTER,DEFAULT_FILTER);
		light.k = 100;
		write_eeprom(EEP_LIGHT_K,	 light.k); 
		write_eeprom(EEP_LIGHT_K + 1,light.k>>8); 
	}

void initial_hum_eep(void)
	{
		uint8 loop;
		if(read_eeprom(EEP_HUM_TEST) != 0x56)
		{
			write_eeprom(EEP_HUM_TEST,0x56);
			reset_to_factory();
		}
		output_manual_value_co2  = 500;
		display_state = PIC_INITIAL;
		humidity_version = 0;
		hum_exists = 0;
		hum_read_delay = 1;
		for(loop=0;loop<10;loop++) //detect if new hum sensor SHT35 is present,if so  
		{
			if(SHT3X_GetTempAndHumi(&tem_org, &hum_org, REPEATAB_HIGH, MODE_POLLING, 50) == 0)
			{
				hum_exists = 2;
				humidity_version = 30;
		    break;
			}
			else
			{
				delay_ms(10);
			}
	  }
		hum_read_delay = 1;
		table_sel_enable = 1;
		HumSensor.offset_t = ((unsigned int)new_read_eeprom(EEP_TEMP_OFFSET + 1)<<8)+new_read_eeprom(EEP_TEMP_OFFSET); 
		HumSensor.offset_h = ((unsigned int)new_read_eeprom(EEP_HUM_OFFSET + 1)<<8)+new_read_eeprom(EEP_HUM_OFFSET); 
		HumSensor.offset_h_default = ((unsigned int)new_read_eeprom(EEP_CAL_DEFAULT_HUM + 1)<<8)+new_read_eeprom(EEP_CAL_DEFAULT_HUM);   
		Run_Timer = 0;
		deg_c_or_f = new_read_eeprom(EEP_DEG_C_OR_F );
		internal_temperature_offset = (((int16)new_read_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 1) << 8) | new_read_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET )) ;
		temperature_sensor_select = new_read_eeprom(EEP_SENSOR_SELECT);	
		HumSensor.humidity = 0;
		HumSensor.temperature_c = 0;
		HumSensor.temperature_f = 320;
		analog_output_sel = new_read_eeprom(EEP_OUTPUT_SEL);
		
		auto_heat_enable =  new_read_eeprom(EEP_AUTO_HEAT_CONTROL);
		if(auto_heat_enable > 1) auto_heat_enable = 0;
		
		light.filter = new_read_eeprom(EEP_LIGHT_FILTER);
		light.k = ((int16)new_read_eeprom(EEP_LIGHT_K + 1) << 8) | new_read_eeprom(EEP_LIGHT_K); 
		
	}
	static void user_sector_initial(void)
	{
		table_sel =  read_eeprom(EEP_TABLE_SEL); 
		if(table_sel != USER) table_sel = FACTORY; 
		table_sel_enable = 1; 
		hum_size_copy =  new_read_eeprom(EEP_USER_POINTS);
		if(hum_size_copy > 10 ) hum_size_copy = 0;  	
	}
	bit refresh_sensor(void)
	{ 
		uint8 i;
		if (humidity_version >= 18)
		{ 
			while(!read_sensor())
			{ 
				watchdog();
				delay_us(2000);
			}
			if(humidity_version >= 20)
			{
				while(!pic_read_sn(&HumSensor.sn))
				{ 
					watchdog();
					delay_us(2000);
				}
			}
			else
				HumSensor.sn = 0xffff;
			if(HumSensor.sn != (new_read_eeprom(EEP_HUM_SN + 1) * 256 + new_read_eeprom(EEP_HUM_SN)))
			{
				new_write_eeprom(EEP_HUM_SN,HumSensor.sn);
				new_write_eeprom(EEP_HUM_SN + 1,HumSensor.sn>>8);     
				HumSensor.offset_h_default = 0;
				new_write_eeprom(EEP_CAL_DEFAULT_HUM, 0);
				new_write_eeprom(EEP_CAL_DEFAULT_HUM+1, 0);
				HumSensor.offset_t = 0;
				new_write_eeprom(EEP_TEMP_OFFSET,0);
				new_write_eeprom(EEP_TEMP_OFFSET+1,0); 
				HumSensor.offset_h = 0;
				new_write_eeprom(EEP_HUM_OFFSET+1,0);
				new_write_eeprom(EEP_HUM_OFFSET,0);  
			}
			if(HumSensor.counter != 0)
			{
				for (i=0;i<10;i++)  //number = 0~19 read the table from the pic on power up	,and write the data to eeprom
				{ 	
					uint16 hum,freq;
					while(! read_sensor_humtab(i, &hum, &freq) )
					{ 
						watchdog();
						delay_us(2000);
					}
					new_write_eeprom(HUMCOUNT1_H+4*i+1,   	(freq>>8) & 0x00ff);
					delay_us(20);
					new_write_eeprom(HUMCOUNT1_H+4*i,		freq & 0x00ff);
					delay_us(20);
					new_write_eeprom(HUMCOUNT1_H+4*i+3,    	(hum>>8) & 0x00ff);
					delay_us(20);
					new_write_eeprom(HUMCOUNT1_H+4*i+2,		hum & 0x00ff);
				}
				new_write_eeprom(EEP_CAL_FAC_PTS,HumSensor.counter);
			}
			user_sector_initial();
		} 
	 
		return 1;
	}



	static void min2method(float *K, float *B, unsigned char counter )
	{
		   float xdata sumx, sumx2, sumy, sumxy;
		   unsigned char  i;
		   
		   sumx = 0;
		   sumx2 = 0;
		   for(i = 0; i < counter; i++)
		   {
			  sumx += hum_table[i][HUM_FREQ];
			  sumx2 += (float)hum_table[i][HUM_FREQ] * hum_table[i][HUM_FREQ];
		   }

		   sumy = 0;
		   for(i = 0; i < counter; i++)
		   {
			  sumy += hum_table[i][HUM_PERCENT];
		   }

		   sumxy = 0;
		   for(i = 0; i < counter; i++)
		   {
			  sumxy += (float)hum_table[i][HUM_FREQ] * hum_table[i][HUM_PERCENT];
		   } 
		   *K = ((counter*sumxy - sumx*sumy) / (counter*sumx2 - sumx*sumx));
	// 	   *B = (sumy - (*k) * sumx)/counter;
		   *B = ((sumx2*sumy - sumx*sumxy) / (counter*sumx2 - sumx*sumx));
	}

	static void calibrate_point_read(void) //read percent and temparature from eeprom;
	{
		unsigned char  i; 
		unsigned char  itemp; 
		if((table_sel == USER)&&(hum_size_copy > 1)) 
		{
			for(i=0;i < hum_size_copy ;i++)
			{
				hum_table[i][HUM_PERCENT] =  new_read_eeprom(EEP_USER_RH1+i*4) +  ((unsigned int)new_read_eeprom(EEP_USER_RH1+i*4+1)<<8);   	
				hum_table[i][HUM_FREQ] =  new_read_eeprom(EEP_USER_RH1+i*4 + 2) +  ((unsigned int)new_read_eeprom(EEP_USER_RH1+i*4+3)<<8);				
			}
			min2method(&k_line, &b_line, hum_size_copy); 
		}
		else
		{
			if(HumSensor.counter != 0) itemp = HumSensor.counter;
			else  itemp = new_read_eeprom(EEP_CAL_FAC_PTS);
				
			for(i=0;i < itemp ;i++)
			{
				hum_table[i][HUM_PERCENT] =  new_read_eeprom(HUMRH1_H+i*4) +  ((unsigned int)new_read_eeprom(HUMRH1_H+i*4+1)<<8);   	
				hum_table[i][HUM_FREQ] =  new_read_eeprom(HUMCOUNT1_H+i*4) +  ((unsigned int)new_read_eeprom(HUMCOUNT1_H+i*4+1)<<8);				
			} 
			min2method(&k_line, &b_line, itemp);
		} 	 
	}
 
	static unsigned int get_humidity(signed int para_input )                                
	{
		float para_temp;
		para_temp = para_input;	
	
		para_temp = (uint32)(k_line * 1000) * para_temp;
		para_temp /= 1000;
		para_temp += b_line;
	
	   return  (unsigned int)para_temp; 
	}



	#define TABLE_N		5
	code signed short int Ref_T[] =  {210,250,300,350,400}; // Temperature
	code signed short int Error_H[5][5] = 
	{	
		{-842,-461, 15, 507,  998},		// Error in Humidiy where humidity is 40%
		{-828,-461, -3, 512, 1027},		// Error in Humidiy where humidity is 50%
		{-813,-461,-20, 518, 1055},		// Error in Humidiy where humidity is 60%
		{-760,-423, -1, 517, 1034},		// Error in Humidiy where humidity is 70%
		{-707,-385, 18, 516, 1013}, 	// Error in Humidiy where humidity is 80%
	};
	unsigned short int tempCompensation_HUM(signed short int TEMPVal, signed short int HUMVal){
	   
	   unsigned char xdata i,j;
	   long xdata temp1,temp2;
	   signed short int xdata xmean,ymean;
	   signed short int xdata error,compensatedHUM;
	   float xdata m,b;
	   if (HUMVal<490)
	   {
		 j=0; 
	   }      
	   else if (HUMVal<590)
	   {
		 j=1;  
	   }      
	   else if (HUMVal<690)
	   {
		  j=2;
	   }   
	   else if (HUMVal<790)
	   {
		  j=3;
	   }   
	   else
	   {
		  j=4;
	   }  
		temp1 = 0;
		temp2 = 0;   
		for (i=0;i<TABLE_N;i++)   
		{ 
			temp1   += Ref_T[i];  
			temp2   += Error_H[j][i];		
		}
		xmean=temp1 /TABLE_N; 
		ymean=temp2 /TABLE_N;
		
		temp1 = 0;
		temp2 = 0;   
		for (i=0;i<TABLE_N;i++)
		{
		   temp1 += (long)(Ref_T[i]-xmean)*(Error_H[j][i]-ymean);
		   temp2 += (long)(Ref_T[i]-xmean)*(Ref_T[i]-xmean); 
		}
		m=(float)temp1/temp2;
		b=ymean-m*xmean;
		error=(m*TEMPVal+b)/10;
		
		if(abs(error) > 200) error = 0;
		compensatedHUM=  HUMVal+error ;
		
		HumSensor.compensation_val = error;
//		if(compensatedHUM > 1000) compensatedHUM = 1000;
//		else if (compensatedHUM < 0) compensatedHUM = 0;
	   
		return (compensatedHUM);   
	}


//void arrange_hum_data(void)
//{

//}
//	

#define  ORG_RH  	0
#define  USER_RH  1	
//hum_table[i][HUM_FREQ]
//hum_table[i][HUM_PERCENT]	
int16 hum_cal_buf[9][2];
float hum_slope_k;
float hum_slope_b;	
void Get_hum_cal_slope(float hum_data)//according to hum_data, get relative slope
{
  uint16 compare_buf;
	uint8 i;// how many calibration points are persent
	float k ,b;
	uint8 num;
	int32 b_temp = 0;
	num = 0;
	if(update_flag == 9)
	{
		point_num = 0;
		for(i=0;i<9;i++)
		{
			hum_table[i][HUM_PERCENT] =  new_read_eeprom(EEP_USER_RH1+i*4) +  ((unsigned int)new_read_eeprom(EEP_USER_RH1+i*4+1)<<8);   	
			hum_table[i][HUM_FREQ] =  new_read_eeprom(EEP_USER_RH1+i*4 + 2) +  ((unsigned int)new_read_eeprom(EEP_USER_RH1+i*4+3)<<8);
		}
		
		for(i=0;i<9;i++)
		{
			if(hum_table[i][HUM_FREQ] != hum_table[i][HUM_PERCENT])
			{
				hum_cal_buf[num][HUM_FREQ] = hum_table[i][HUM_FREQ];
				hum_cal_buf[num][HUM_PERCENT] = hum_table[i][HUM_PERCENT];
				point_num++;
				num++;							
			}
		}				
		update_flag = 0;
		
	}
	
	if(point_num == 1)
	{
		HumSensor.offset_h = hum_cal_buf[num][HUM_PERCENT] - hum_cal_buf[num][HUM_FREQ];
	}
	
	//k = (y1-y2)/(x1-x2)
	//b = (x2y1-x1y2)/x2-x1
  else if(point_num == 2) //two points line
	{
	  k = (float)(hum_cal_buf[0][HUM_FREQ] - hum_cal_buf[1][HUM_FREQ])/(hum_cal_buf[0][HUM_PERCENT] - hum_cal_buf[1][HUM_PERCENT]);
		
		b_temp = hum_cal_buf[1][HUM_PERCENT] * hum_cal_buf[0][HUM_FREQ];
		b_temp = b_temp - (hum_cal_buf[0][HUM_PERCENT] * hum_cal_buf[1][HUM_FREQ]);
		b_temp = b_temp / (hum_cal_buf[1][HUM_PERCENT] - hum_cal_buf[0][HUM_PERCENT]);
		b = b_temp;
		hum_slope_k = k;
		hum_slope_b = b;
	}
	else if(point_num > 2)//multiple calibration lines
	{
		for(i=0;i<point_num;i++)
		{
			if(hum_data < hum_cal_buf[0][HUM_PERCENT])
			{
				k = (float)(hum_cal_buf[0][HUM_FREQ] - hum_cal_buf[1][HUM_FREQ])/(hum_cal_buf[0][HUM_PERCENT] - hum_cal_buf[1][HUM_PERCENT]);
				b_temp = hum_cal_buf[1][HUM_PERCENT] * hum_cal_buf[0][HUM_FREQ];
				b_temp = b_temp - (hum_cal_buf[0][HUM_PERCENT] * hum_cal_buf[1][HUM_FREQ]);
				b_temp = b_temp / (hum_cal_buf[1][HUM_PERCENT] - hum_cal_buf[0][HUM_PERCENT]);
				b = b_temp;
				hum_slope_k = k;
				hum_slope_b = b;
				break;
			}
			else if(hum_data >= hum_cal_buf[point_num-1][HUM_PERCENT])
			{
				k = (float)(hum_cal_buf[point_num-2][HUM_FREQ] - hum_cal_buf[point_num-1][HUM_FREQ])/(hum_cal_buf[point_num-2][HUM_PERCENT] - hum_cal_buf[point_num-1][HUM_PERCENT]);
				
				b_temp = hum_cal_buf[point_num-1][HUM_PERCENT] * hum_cal_buf[point_num-2][HUM_FREQ];
				b_temp = b_temp - (hum_cal_buf[point_num-2][HUM_PERCENT] * hum_cal_buf[point_num-1][HUM_FREQ]);
				b_temp = b_temp / (hum_cal_buf[point_num-1][HUM_PERCENT] - hum_cal_buf[point_num-2][HUM_PERCENT]);
				b = b_temp;
				hum_slope_k = k;
				hum_slope_b = b;
				break;				
			}
			else if((hum_data >= hum_cal_buf[i][HUM_PERCENT]) && (hum_data < hum_cal_buf[i+1][HUM_PERCENT]))
			{
				k = (float)(hum_cal_buf[i][HUM_FREQ] - hum_cal_buf[i+1][HUM_FREQ])/(hum_cal_buf[i][HUM_PERCENT] - hum_cal_buf[i+1][HUM_PERCENT]);				
				b_temp = hum_cal_buf[i+1][HUM_PERCENT] * hum_cal_buf[i][HUM_FREQ];
				b_temp = b_temp - (hum_cal_buf[i][HUM_PERCENT] * hum_cal_buf[i+1][HUM_FREQ]);
				b_temp = b_temp / (hum_cal_buf[i+1][HUM_PERCENT] - hum_cal_buf[i][HUM_PERCENT]);
				b = b_temp;
				hum_slope_k = k;
				hum_slope_b = b;
				break;				
			}
		}
	
	}

	
}	

uint16 const HUM_DEF_TABLE[9] = {100,200,300,400,500,600,700,800,900};
void calibrate_reset(void)
{
	uint8 i;
	for(i=0;i<9;i++)
	{
		new_write_eeprom(EEP_USER_RH1 + i*4,HUM_DEF_TABLE[i]);
		new_write_eeprom(EEP_USER_RH1 + i*4 + 1,HUM_DEF_TABLE[i]>>8);
		new_write_eeprom(EEP_USER_RH1 + i*4 + 2,HUM_DEF_TABLE[i]);
		new_write_eeprom(EEP_USER_RH1 + i*4 + 3,HUM_DEF_TABLE[i]>>8);
		hum_table[i][HUM_FREQ] = HUM_DEF_TABLE[i];
		hum_table[i][HUM_PERCENT] = HUM_DEF_TABLE[i];
	}
}

	
int16 Get_Average_Humdity(int16 para_h)
{
	uint8   i;
	int16   siMin,siMax;
    signed long   slSum;
	static int16   FilterH[21];
	FilterH[20] = para_h;
	siMin = FilterH[1];
	siMax = FilterH[1];
	slSum = 0;
	for(i=0;i<20;i++)
	{
		FilterH[i] = FilterH[i+1];
		if(siMin > FilterH[i])
			siMin = FilterH[i];
		if(siMax < FilterH[i])
			siMax = FilterH[i];
		slSum += FilterH[i];
	} 
	slSum = slSum - siMin - siMax;
	
	return (slSum /18);
} 
	
	void update_temperature(void)
	{ 
		int16 temp;
		float hum_temp = 0;
		
		static float pre_int_temperature = 0;
		if(Run_Timer <= FIRST_TIME) Run_Timer ++;
		
		if(hum_exists == 1)
		{
		if(Run_Timer > FIRST_TIME)
			HumSensor.pre_temperature_c = Sys_Filter(HumSensor.ad[1],HumSensor.pre_temperature_c,HumSensor.T_Filter);
		else
			HumSensor.pre_temperature_c = HumSensor.ad[1];
		}
		else if(hum_exists == 2)
		{
		if(Run_Timer > FIRST_TIME)
			HumSensor.pre_temperature_c = Sys_Filter(tem_org,HumSensor.pre_temperature_c,HumSensor.T_Filter);
		else
			HumSensor.pre_temperature_c = tem_org;
		}		
	
		HumSensor.temperature_c = HumSensor.pre_temperature_c ; 
		HumSensor.temperature_c += HumSensor.offset_t;
		if((output_auto_manual & 0x01) == 0x01)
		{
			HumSensor.temperature_c = output_manual_value_temp;
		}
		else
		   output_manual_value_temp = HumSensor.temperature_c ;
		HumSensor.temperature_f = HumSensor.temperature_c * 9 / 5 + 320;
		
		internal_temp_ad = get_ad_val(TEMP_AD);
		
		temp = (int16)(look_up_table(internal_temp_ad/4));
		if(temp & 0x8000) // minus temperature
			temp = -(signed int)(temp & 0x7fff);
		
 	//	temp = temp + internal_temperature_offset;
 		pre_int_temperature = Sys_Filter(temp, pre_int_temperature, Temperature_Filter);
		if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
			internal_temperature_c =   pre_int_temperature  + internal_temperature_offset; 
		else if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
			internal_temperature_c = 0;
		 
		
		internal_temperature_f = internal_temperature_c * 9 / 5 + 320;
		
		if(hum_exists == 1)
		{
			if(humidity_version>=18)		//we can read the value from the humidity sensor directly.
			{
					if(table_sel_enable)
					{
						table_sel_enable = 0;
						calibrate_point_read();
					}  
					humidity_back = get_humidity(HumSensor.frequency); 
					if(humidity_back > 2000) humidity_back = 2000; //200.0%
					else if(humidity_back < -2000) humidity_back = -2000; // -200.0%
					
	//				test[0] = humidity_back;
	//				test[1] = HumSensor.ad[0];
					if(table_sel == USER)
					{  
						if(hum_size_copy <= 1) 
						{ 	  
							if((humidity_version >= 24) ||(humidity_version == 19))
							{
								if(humidity_version >= 24)					
									humidity_back = tempCompensation_HUM(HumSensor.temperature_c ,humidity_back);//HumSensor.ad[0]);
								else
									humidity_back=HumSensor.ad[0]; 
							}
							humidity_back += HumSensor.offset_h_default;
							temp=humidity_back + HumSensor.offset_h; 	
						} 
						else
						{  
							temp=humidity_back+ HumSensor.offset_h; 
						}
					}
					else
					{ 		  
						if((humidity_version >= 24) ||(humidity_version == 19))
						{ 
							if(humidity_version >= 24)					
								;//humidity_back = tempCompensation_HUM(HumSensor.temperature_c ,humidity_back);//HumSensor.ad[0]);
							else
								humidity_back=HumSensor.ad[0]; 
						} 
						temp = humidity_back+HumSensor.offset_h_default; 
					} 		
			}
		}
		else if(hum_exists == 2)
		{
			if(update_flag == 8)//initialize user table to default value
			{
				calibrate_reset();
			}  

		 temp = hum_org;
		 
		}			
		
		if(temp < 0) temp =0;
		
		if(abs(HumSensor.pre_humidity - temp) > 100)  //10%
			HumSensor.pre_humidity = Sys_Filter(temp,HumSensor.pre_humidity,HumSensor.H_Filter);
		else
		{
			if(HumSensor.pre_humidity < temp) 
				HumSensor.pre_humidity++;
			else if(HumSensor.pre_humidity > temp)
				HumSensor.pre_humidity--;
		}	
 		HumSensor.pre_humidity = Get_Average_Humdity(HumSensor.pre_humidity);
		
		if(hum_exists == 1)
		{
			if(Run_Timer < FIRST_TIME)
				HumSensor.pre_humidity =  temp;
			else
				HumSensor.humidity = HumSensor.pre_humidity;		
		}			
		else if(hum_exists == 2)
		{
			if(Run_Timer < FIRST_TIME)
				HumSensor.humidity =  hum_org + HumSensor.offset_h;
			else
			{
				Get_hum_cal_slope(hum_org);
		
				if(point_num <= 1)//no calibration point or just one point, use hum offset
				{
					HumSensor.humidity = hum_org + HumSensor.offset_h;
				}
				else
				{	
					hum_temp = HumSensor.pre_humidity;					
				  hum_temp = hum_temp * hum_slope_k + hum_slope_b;
					HumSensor.humidity = hum_temp;
				}
			}
		}
		

		if(output_auto_manual & 0x02)
			HumSensor.humidity = output_manual_value_humidity;
		else
			output_manual_value_humidity = HumSensor.humidity;
		
		HumSensor.org_hum = HumSensor.humidity - HumSensor.compensation_val ;
		

		
		if(HumSensor.humidity > 1000)
			HumSensor.humidity = 1000; 
		
		
		
	    
//-----------------------------		
		//ADD BY CHARLY			
	    if(PRODUCT_ID == STM32_PM25)
		{
			//refresh_sensor();
			
			if((display_state >= PIC_WAITING1)&&(hum_exists))
				display_state++;
			if(display_state == PIC_WAITING_END)
			{ 
				hum_read_delay = 1;
	//			Lcd_Show_String(1, 0, DISP_NOR,(unsigned char *)"Done");
				display_state = PIC_NORMAL; 
				if(hum_exists == 1)				
					refresh_sensor(); 
				else if(hum_exists == 2)
					SHT3X_GetTempAndHumi(&tem_org, &hum_org, REPEATAB_HIGH, MODE_POLLING, 50);
				Lcd_Full_Screen(0);
				hum_read_delay = 1;
			}			
//=================================================================
			if(display_state == PIC_INITIAL)	
			{  
	//			Lcd_Show_String(0, 0, DISP_NOR, (unsigned char *)"Initialization...");
				display_state = PIC_WAITING1;
			} 
	//=================================================================    	
			if((display_state == PIC_OFF_TO_ON)&&(hum_exists == 1))
			{  
				if(isColorScreen==false)
				{
					Lcd_Full_Screen(0);
					Lcd_Show_String(2, 9, DISP_NOR, (unsigned char *)" NEW  "); 
					Lcd_Show_String(3, 9, DISP_NOR, (unsigned char *)"SENSOR"); 
					display_state = PIC_WAITING1;
				}
				//SoftReset();
			} 
				
	//======================================================================
			if(display_state == PIC_ON_TO_OFF)	
			{   
				display_state = PIC_WAIT_OFF_TO_ON;		 
			}
			
			
		}
	    else if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
		{
			if((display_state >= PIC_WAITING1)&&(hum_exists))
				display_state++;
			if(display_state == PIC_WAITING_END)
			{  
				display_state = PIC_NORMAL; 
				hum_read_delay = 1;				
				if(hum_exists == 1)				
					refresh_sensor(); 
				else if(hum_exists == 2)
					SHT3X_GetTempAndHumi(&tem_org, &hum_org, REPEATAB_HIGH, MODE_POLLING, 50); 
				hum_read_delay = 1;				
			}
	//=================================================================
			if(display_state == PIC_INITIAL)	
			{  
				display_state = PIC_WAITING1;
			} 
	//=================================================================    	
			if((display_state == PIC_OFF_TO_ON)&&(hum_exists == 1))
			{  
				if(isColorScreen == false)
				{
					Lcd_Full_Screen(0);
					Lcd_Show_String(2, 9, DISP_NOR, (unsigned char *)" NEW  "); 
					Lcd_Show_String(3, 9, DISP_NOR, (unsigned char *)"SENSOR"); 
					display_state = PIC_WAITING1;
				}
//				SoftReset();
			} 
				
	//======================================================================
			if(display_state == PIC_ON_TO_OFF)	
			{    
				display_state = PIC_WAIT_OFF_TO_ON;
			 
			}			
		}	
		
		
		
	    if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
		{
		//	refresh_sensor();
			
			if((display_state >= PIC_WAITING1)&&(hum_exists))
				display_state++;
			if(display_state == PIC_WAITING_END)
			{ 
	//			Lcd_Show_String(1, 0, DISP_NOR,(unsigned char *)"Done");
				display_state = PIC_NORMAL; 
				hum_read_delay = 1;				
				if(hum_exists == 1)				
					refresh_sensor(); 
				else if(hum_exists == 2)
					SHT3X_GetTempAndHumi(&tem_org, &hum_org, REPEATAB_HIGH, MODE_POLLING, 50); 
				hum_read_delay = 1;				
				if(isColorScreen == false)
					Lcd_Full_Screen(0);
				
			

			}
			
	//=================================================================
			if(display_state == PIC_INITIAL)	
			{  
	//			Lcd_Show_String(0, 0, DISP_NOR, (unsigned char *)"Initialization...");
				display_state = PIC_WAITING1;
			} 
	//=================================================================    	
			if((display_state == PIC_OFF_TO_ON)&&(hum_exists == 1))
			{  
				if(isColorScreen == false)
				{
					Lcd_Full_Screen(0);
					Lcd_Show_String(2, 9, DISP_NOR, (unsigned char *)" NEW  "); 
					Lcd_Show_String(3, 9, DISP_NOR, (unsigned char *)"SENSOR"); 
					display_state = PIC_WAITING1;
				}
//				SoftReset();
			} 
				
	//======================================================================
			if(display_state == PIC_ON_TO_OFF)	
			{   
				display_state = PIC_WAIT_OFF_TO_ON;		 
			}
			
			
		}
//	    else if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
//		{
//			if((display_state >= PIC_WAITING1)&&(hum_exists))
//				display_state++;
//			if(display_state == PIC_WAITING_END)
//			{  
//				display_state = PIC_NORMAL;  
//				if(hum_exists == 1)				
//					refresh_sensor(); 
//				else if(hum_exists == 2)
//					SHT3X_GetTempAndHumi(&tem_org, &hum_org, REPEATAB_HIGH, MODE_POLLING, 50);	 
//			}
//	//=================================================================
//			if(display_state == PIC_INITIAL)	
//			{  
//				display_state = PIC_WAITING1;
//			} 
//	//=================================================================    	
//			if((display_state == PIC_OFF_TO_ON)&&(hum_exists == 1))
//			{  
//				Lcd_Full_Screen(0);
//				Lcd_Show_String(2, 9, DISP_NOR, (unsigned char *)" NEW  "); 
//				Lcd_Show_String(3, 9, DISP_NOR, (unsigned char *)"SENSOR"); 
//				display_state = PIC_WAITING1;
////				SoftReset();
//			} 
//				
//	//======================================================================
//			if(display_state == PIC_ON_TO_OFF)	
//			{    
//				display_state = PIC_WAIT_OFF_TO_ON;
//			 
//			}
//		}
	 
		
		Get_Hum_Para( HumSensor.temperature_c, HumSensor.humidity,&HumSensor.dew_pt,\
					&HumSensor.Pws,&HumSensor.Mix_Ratio,&HumSensor.Enthalpy); 
//		#ifdef HUM_SENSOR
//		
//		if((output_auto_manual & 0x04) == 0x04)
//		{
//			HumSensor.dew_pt = output_manual_value_co2;	
//		}
//		
//		#endif
		HumSensor.dew_pt_F = HumSensor.dew_pt * 9 / 5 + 320;
		var[CHANNEL_HUM]. value = HumSensor.humidity;
		var[CHANNEL_TEMP]. value =HumSensor.temperature_c;

		

		
}

	void update_temperature_display(uint8 ForceUpdate)
  {
//	static uint8 pre_deg_c_or_f = 2;
	int16 temperature;

	if((output_auto_manual & 0x01) == 0x01)
	{
		temperature = output_manual_value_temp;
	}
	else
	{
		if(temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
		{
			if(deg_c_or_f == DEGREE_C)
				temperature = internal_temperature_c;
			else
				temperature = internal_temperature_f;
		}
		else
		{
			if(deg_c_or_f == DEGREE_C)
				temperature = HumSensor.temperature_c;
			else
				temperature = HumSensor.temperature_f;
		}
	}	

//	if((previous_temperature != temperature) || (pre_deg_c_or_f != deg_c_or_f) || ForceUpdate)
	{
		if(ForceUpdate || (previous_temperature != temperature))
		{
			 if(isColorScreen == false)
  			Lcd_Show_Data(0, 19, temperature, 1, ALIGN_RIGHT, DISP_NOR);
// 			sprintf((char *)text,"%5.1f",((float)temperature)/10);
// 			Lcd_Show_String(0, 15, DISP_NOR, (unsigned char *)text);  
		}

		if(ForceUpdate )//|| (pre_deg_c_or_f != deg_c_or_f))
		{
			if(isColorScreen == false)
			{
				if(deg_c_or_f == DEGREE_C)
					Lcd_Write_Char(0, 20, '~', DISP_NOR);
				else
					Lcd_Write_Char(0, 20, '|', DISP_NOR);
			}
		}
		
//		previous_temperature = temperature;
//		pre_deg_c_or_f = deg_c_or_f;
	}
	 
	if(rx_icon>0)
	{
		rx_icon--;
		if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
			Lcd_Show_String(0, 1, DISP_NOR,(uint8 *)"("); //show communication state
		else// if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
			Lcd_Show_String(1, 1, DISP_NOR,(uint8 *)"("); //show communication state
	}
	if(rx_icon == 0)//clear rx icon
	{
		if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
			Lcd_Show_String(0, 1, DISP_NOR,(uint8 *)" "); //show communication state
		else// if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
			Lcd_Show_String(1, 1, DISP_NOR,(uint8 *)" "); //show communication state
	}
	
	if(tx_icon>0)
	{
		tx_icon--;
		if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
			Lcd_Show_String(0, 0, DISP_NOR,(uint8 *)")"); //show communication state
		else// if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
			Lcd_Show_String(1, 0, DISP_NOR,(uint8 *)")");
	}
	if(tx_icon == 0)//clear tx icon
	{
		if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
			Lcd_Show_String(0, 0, DISP_NOR,(uint8 *)" "); //show communication state	
		else// if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
			Lcd_Show_String(1, 0, DISP_NOR,(uint8 *)" "); //show communication state
	}
 }
 

uint16 get_light_value(uint16 ad);
void vUpdate_Temperature_Task( void *pvParameters )
{ 
//	 initial_hum();
	 uint16 itemp;
	 etError   error; 
	 regStatus status;      // sensor status
	 uint32 serialNumber;// serial number
		
	float light_itime;
	float light_gain;
	float light_data0,light_data1;
	
	 print("UPDATE TEMPERATURE Task\r\n");
	 delay_ms(100);
	
//	 SHT3X_Init(0x45); 
  update_flag = 9;
  delay_ms(50);  
  
	 for(;;)
	 { 
			 
		static uint8 ctr = 0;	
		if(xQueueTakeMutexRecursive( IicMutex, portMAX_DELAY )==pdPASS)
		{
			hum_read_delay = 1;
			if(display_state != PIC_NORMAL)
			{
				if( SHT3X_GetTempAndHumi(&tem_org, &hum_org, REPEATAB_HIGH, MODE_POLLING, 50) == 0)
				{
					hum_exists = 2;
					ctr = 0;				
				}
				else
				{
					if(read_humidity_sensor_version())
					{ 
						hum_exists = 1;
						if(display_state == PIC_WAIT_OFF_TO_ON)
						{
							display_state = PIC_OFF_TO_ON; 
						}	
						ctr = 0;
					}
					else
					{
						if(ctr <= 20)  ctr++;
						
						if(ctr == 20)
						{	
							display_state = PIC_ON_TO_OFF;
							hum_exists = 0; 
						}
					}
				}
				
				
			}
			else
			{
//				if(!read_sensor())
//				{
				if(hum_exists == 2)
				{
         
					if(SHT3X_GetTempAndHumi(&tem_org, &hum_org, REPEATAB_HIGH, MODE_POLLING, 50) != 0 )
					{
						
						if(ctr < 20)  ctr++;
						
						if(ctr == 20)
						{	
							display_state = PIC_ON_TO_OFF;
							hum_exists = 0; 
						}				
					}
					else
					{
						ctr = 0;
					}
					
				}
				else if(hum_exists == 1)
				{
					if(!read_sensor())
					{
						if(ctr < 20)  ctr++;
						
						if(ctr == 20)
						{	
							display_state = PIC_ON_TO_OFF;
							hum_exists = 0; 
						}						
					}
					else
						ctr = 0;
				}
//					if(ctr <= 20)  ctr++;
//					
//					if(ctr == 20)
//					{	
//						display_state = PIC_ON_TO_OFF;
//						hum_exists = 0; 
//					}

				else 
					ctr = 0;
			} 
			hum_read_delay = 1;			
			  
			delay_ms(100);
			start_light_sensor_mearsure();
			if(read_light_sensor_version())
			{
				light_itime = (float)read_light_sensors_time();
				light_gain = (float)read_light_sensors_gain();
				light_data0 = (float)read_light_sensors_data0();
				light_data1 = (float)read_light_sensors_data1();
				if((light_data1/light_data0)<0.26)
					light.val = ((1.290*light_data0-2.733*light_data1)/(light_gain*102.6)/light_itime)*1000;
				if((light_data1/light_data0)<0.55)
					light.val = ((0.795*light_data0-0.859*light_data1)/(light_gain*102.6)/light_itime)*1000;
				if((light_data1/light_data0)<1.09)
					light.val = ((0.510*light_data0-0.345*light_data1)/(light_gain*102.6)/light_itime)*1000;
				if((light_data1/light_data0)<2.13)
					light.val = ((0.276*light_data0-0.130*light_data1)/(light_gain*102.6)/light_itime)*1000;
			}
			else
			{
				pic_read_light_val(&light.ad); 
				itemp = get_light_value(light.ad);
				light.pre_val = Sys_Filter(itemp,light.pre_val,light.filter);
				light.val = light.pre_val;
			}
			xQueueGiveMutexRecursive( IicMutex );
		}
		
		external_operation();
		update_temperature();
		if(auto_heat_enable)
			auto_heating();
//		stack_detect(&test[5]);
		if(display_state == PIC_NORMAL)
			vTaskDelay(5000 / portTICK_RATE_MS);
		else
			vTaskDelay(1000 / portTICK_RATE_MS);
	 }
	 
}	
 
 
uint16_t const lightsensor_table_tps851[27] =	
{
	  1,  2,  2,  4,  6,  7,   9, 12, 14, 16,
	 18, 20, 37, 63, 80, 94, 117,142,154,171,
	196,313,455,597,711,853, 1024
//	0x0001, 0x0002, 0x0002, 0x0004, 0x0006, 0x0007, 0x0009, 0x000c, 0x000e, 0x0010,
//	0x0012,	0x0014,	0x0025,	0x003f,	0x0050,	0x005e,	0x0075,	0x008e, 0x009a, 0x00ab, 
//	0x00c4, 0x0139, 0x01c7, 0x0255, 0x02c7, 0x0355, 0x0400
};

uint16 get_light_value(uint16 ad)
{
	uint32 temp;
	//temp = ad*10*light.k/1024;	//LUX = 2 * I(uA).//(VCC = 5V,RL = 10K, 10Bits AD)
	temp = ad*1000/1024;	//LUX = 2 * I(uA).//(VCC = 5V,RL = 10K, 10Bits AD)
	//temp = temp /100;
	
	light.org = temp;
	if(light.k == 0)
	{
		temp = light.org;
	}
	else
	{
//		if(temp >= 100)
			temp = temp * (150 + light.k * (light.org-87)/27)/100;
//		else
//			temp = temp * light.k + 1);
//		temp /= 100;
  }
	
	return (uint16)temp;
}





