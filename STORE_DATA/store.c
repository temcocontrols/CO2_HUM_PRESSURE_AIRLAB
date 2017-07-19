//#include "store.h"
//#include "modbus.h"
////#include "ud_str.h"
//#include "controls.h"
//#include "bacnet.h"
//#include "stmflash.h"
//#include "delay.h"
//#include "registerlist.h"
//#include "24cxx.h"
#include "config.h"
#define PAGE_LENTH		MAX_AVS * sizeof(Str_variable_point)

uint8_t write_page_en[MAX_TYPE];
Str_variable_point var[MAX_AVS] ;

 
  

//static uint8_t  tempbuf[1024] = {0};

 
 
//#define PAGE127     0x0803f800 
// 

//void Flash_Write_Mass(void)
//{
//	uint8 i;
//	uint16 pos;
//	if(write_page_en == 1)
//	{
//		write_page_en = 0; 
//		pos = 0;
//		for(i = 0;i < MAX_VARS; i++) 
//		{	 
//			memcpy(&tempbuf[pos],var[i].description,21); 
//			pos += 21;
//			memcpy(&tempbuf[pos],var[i].label,9); 
//			pos += 9;
//		}
//		for(i = 0;i < MAX_AOS; i++) 
//		{
//			memcpy(&tempbuf[pos],outputs[i].description,21); 
//			pos += 21;
//			memcpy(&tempbuf[pos],outputs[i].label,9); 
//			pos += 9;			
//		} 
//		
//		STMFLASH_Unlock();  //解锁	 
//		STMFLASH_ErasePage(PAGE127);	//擦除这个扇区
//		iap_write_appbin(PAGE127,tempbuf,pos); 
//		STMFLASH_Lock();	//上锁
//	}
//	 
//}

void Flash_Write_Mass(void)
{ 
	uint16_t	len = 0 ;
//	uint16_t    loop1;	 
	uint8_t     tempbuf[PAGE_LENTH]; 
	if(write_page_en[OUT_TYPE] == 1)
	{
		write_page_en[OUT_TYPE] = 0 ;	
		STMFLASH_Unlock();  //解锁	
		STMFLASH_ErasePage(OUT_PAGE_FLAG);				
//		for(loop1 = 0;loop1 < MAX_OUTS;loop1++)
//		{
//			memcpy(&tempbuf[sizeof(Str_out_point) * loop1],&outputs[loop1],sizeof(Str_out_point));					
//		}
//		len = sizeof(Str_out_point) *MAX_OUTS ;
//		iap_write_appbin(OUT_PAGE,(uint8_t*)tempbuf, len); 
		
		len = MAX_OUTS * sizeof(Str_out_point) ; 
		memcpy(tempbuf,(void*)&outputs[0].description[0],len); 
		iap_write_appbin(OUT_PAGE,(uint8_t*)tempbuf, len); 
		
		STMFLASH_WriteHalfWord(OUT_PAGE_FLAG, 10000) ;
		STMFLASH_Lock();	
 	} 
	
	if(write_page_en[IN_TYPE] == 1)
	{
		write_page_en[IN_TYPE] = 0 ;
		STMFLASH_Unlock();	
		STMFLASH_ErasePage(IN_PAGE_FLAG);
//		for(loop1 = 0;loop1 < MAX_INS;loop1++)
//		{
//			memcpy(&tempbuf[sizeof(Str_in_point) * loop1],&inputs[loop1],sizeof(Str_in_point));					
//		}
//		len = sizeof(Str_in_point)*MAX_INS ;
		len = MAX_INS * sizeof(Str_in_point) ; 
		memcpy(tempbuf,(void*)&inputs[0].description[0],len);  
		iap_write_appbin(IN_PAGE,(uint8_t*)tempbuf, len); 
		
		STMFLASH_WriteHalfWord(IN_PAGE_FLAG, 10000) ;
		STMFLASH_Lock();	
	} 
	
	if(write_page_en[VAR_TYPE] == 1)
	{  
			
		STMFLASH_Unlock();
		STMFLASH_ErasePage(AV_PAGE_FLAG);
		STMFLASH_ErasePage(AV_PAGE_FLAG + 2048);
		 
		len = MAX_AVS * sizeof(Str_variable_point) ; 
		memcpy(tempbuf,(void*)&var[0].description[0],len); 
		iap_write_appbin(AV_PAGE,(uint8_t*)tempbuf, len); 
		
		STMFLASH_WriteHalfWord(AV_PAGE_FLAG, 10000) ;	
		STMFLASH_Lock();
		write_page_en[VAR_TYPE] = 0 ; 
	}				
	io_control(); 
}
const uint8 Var_label[MAX_AVS][9] = {
	
	"SN_L",   //0
	"SN_H",   //1
	"SW_Ver", //2
	"Address",//3
	"Model",  //4 
	"Instance",//5					 
	"Station",//6
	"BaudRate",//7
	"Update",  //8
	"Protocol",//9
	"AM_Mode", //10 
	"dew_pt",  //11
	"Pws",     //12
	"MixRatio",//13
	"Enthalpy",//14  
	"OffSet_H",//15
	"OffSet_T",//16 
	"OffSet_C",//17 
	"OffSet_P",//18
	"OfSe_P25",//19
	"OfSt_P10",//20
	"Filter_H",//21
	"Filter_T",//22
	"Filter_C",//23 
	"Filter_P",//24
	"Filt_P25",//25
	"Filt_P10",//26
	"T_Unit",  //27 
	"OutMode", //28
	 
	"MIN_RNG0",//29 
	"MAX_RNG0",//30
	"MIN_RNG1",//31
	"MAX_RNG1",//32
	"MIN_RNG2",//33
	"MAX_RNG2",//34 
	"AQI"	   //35
//	"PID_SEL", //35
//	"PID1_MODE",//36
//	"PID1_SP",  //37 
//	"PID1_P",   //38
//	"PID1_I",   //39
//	"PID1_V",   //40
//	"PID2_MODE",//41 
//	"PID2_SP",  //42 
//	"PID2_P",   //43
//	"PID2_I",   //44
//	"PID2_V",   //45
//	"PID3_MODE",//46 
//	"PID3_SP",  //47 
//	"PID3_P",   //48
//	"PID3_I",   //49
//	"PID3_V",   //50
//	"AlarmAM",  //51
//	"Alarm",    //52
//	"PreAlarm", //53
//	"Time_On",  //54
//	"Time_Off", //55 
};
const uint8 Var_Description[MAX_AVS][21] = {
	
	"SerialNumberLowByte",   	//0
	"SerialNumberHighByte",   	//1
	"SoftWare Version", 		//2
	"ID Address",				//3
	"Product Model",			//4
	"Instance",					//5					 
	"Station number",			//6
	"Uart BaudRate",			//7
	"Update", 					//8
	"Protocol",					//9
	"Auto/Manual",				//10 
	"dew point",				// 
	"Pws",						// 
	"MixRatio",					// 
	"Enthalpy",  				// 
	"OffSet Humdity",			//15 
	"OffSet Tempeature", 		// 
	"OffSet Co2",				//  
	"OffSet_Pressure",			//
	"OffSet_P25",				//
	"OffSet_P10",				//20
	"Filter Humdity",			// 
	"Filter Temperature",		// 
	"Filter Co2",				// 
	"Filter_Pressure",			//
	"Filter_Pm25",				//25
	"Filter_Pm10",				//
	"Temperature Unit",			//  
	"OutMode",					//  
	"Humdity Min Range",		// 
	"Humdity Max Range",		//30 
	"Temperatur Min Range",		// 
	"Temperatur Max Range",		//  
	"Co2 Min Range",			// 
	"Co2 Max Range", 			//
	"Air Quality Index"			//35
	
//	"PID/Transmit select",		//35
//	"PID1 MODE",				//36
//	"PID1 SetPoint",			//37 
//	"PID1 Pterm",				//38
//	"PID1 Iterm",				//39
//	"PID1 Value", 				//40
//	"PID2 MODE",				//41
//	"PID2 SetPoint",			//42 
//	"PID2 Pterm",				//43
//	"PID2 Iterm",				//44
//	"PID2 Value",				//45
//	"PID3 MODE",				//46
//	"PID3 SetPoint",			//47 
//	"PID3 Pterm",				//48
//	"PID3 Iterm",				//49
//	"PID3 Value",				//50
//	"Alarm Auto/Manual",		//51	
//	"Alarm Setpoint",			//52
//	"PreAlarm Setpoint",		//53
//	"Alarm Time On",			//54
//	"Alarm Time Off", 			//55
};

const uint8 Outputs_label[MAX_AOS][9] = {
 	"Output1",
	"Output2",
	"Output3" 
};
const uint8 Outputs_Description[MAX_AOS][21] = {
 	
 	"Analog output1",
	"Analog output2",
	"Analog output3"
};

const uint8 Inputs_label[MAX_INS][9] = {
 	"Temperat",
	"Hum",
	"CO2",
	"Pressure",
	"PM2.5",
	"PM10",
	"Light"
};
const uint8 Inputs_Description[MAX_INS][21] = {
 	
 	"Temperature",
	"Humidity",
	"CO2",
	"Pressure",
	"PM2.5",
	"PM10",
	"Light Sensor"
};

 

void mass_flash_init(void)
{
	u16 temp = 0 ;
	u16 loop , j ;
	u16 len = 0;
	uint8_t  tempbuf[PAGE_LENTH];
// 	u16 temp2 = 0 ;
//	u8 label_buf[21] ;
	for(j = 0; j < MAX_TYPE; j++)
		write_page_en[j] = 0;
 
 	temp = STMFLASH_ReadHalfWord(OUT_PAGE_FLAG);
//	printf("temp=%x, %x\n\r", temp2, temp);
	if(temp == 0xffff)
	{
		STMFLASH_Unlock();
		STMFLASH_ErasePage(OUT_PAGE_FLAG);
		for(loop=0; loop<MAX_OUTS; loop++ )
		{
			memcpy(outputs[loop].description,Outputs_Description[loop],21);  
			memcpy(outputs[loop].label,Outputs_label[loop],9);  		
			outputs[loop].value = 0; 
			outputs[loop].auto_manual = 0 ;
			outputs[loop].digital_analog = 1 ;
			outputs[loop].switch_status = 1 ;
			outputs[loop].control = 0 ;
			outputs[loop].read_remote = 0 ;
			outputs[loop].decom = 0 ;
			outputs[loop].range = 0 ;
			outputs[loop].sub_id = 0 ;
			outputs[loop].sub_product = 0 ;
			outputs[loop].pwm_period = 0 ;
		}
		len = MAX_OUTS * sizeof(Str_out_point) ;
//		if(len > PAGE_LENTH)
//		{
//			memcpy(tempbuf,(void *)&outputs[0].description[0],PAGE_LENTH); 
//			memcpy(tempbuf,(void *)(&outputs[0].description[0] + PAGE_LENTH),len - PAGE_LENTH);
//		}
//		else
		memcpy(tempbuf,(void *)&outputs[0].description[0],len);  
		iap_write_appbin(OUT_PAGE,(uint8_t*)tempbuf, len);	 
		STMFLASH_WriteHalfWord(OUT_PAGE_FLAG, 10000) ;
		STMFLASH_Lock();	
	}
	else
	{
		len = MAX_OUTS * sizeof(Str_out_point) ;
		STMFLASH_MUL_Read(OUT_PAGE,(void *)&outputs[0].description[0], len );	
	}
	
	temp = STMFLASH_ReadHalfWord(AV_PAGE_FLAG);  
	if(temp == 0xffff)
	{
		STMFLASH_Unlock();
		STMFLASH_ErasePage(AV_PAGE_FLAG);
		STMFLASH_ErasePage(AV_PAGE_FLAG + 2048);
		for(loop=0; loop<MAX_AVS; loop++ )
		{
			memcpy(var[loop].description,Var_Description[loop],21);  
			memcpy(var[loop].label,Var_label[loop],9); 
			var[loop].value = 0; 
			var[loop].auto_manual = 0 ;
			var[loop].digital_analog = 1 ;
			var[loop].control = 0 ;
			var[loop].unused = 0 ;
			var[loop].range = 0 ;
			var[loop].range = 0 ;
		}
		len = MAX_AVS * sizeof(Str_variable_point) ;
		memcpy(tempbuf,(void*)&var[0].description[0],len); 
		iap_write_appbin(AV_PAGE,(uint8_t*)tempbuf, len); 
		STMFLASH_WriteHalfWord(AV_PAGE_FLAG, 10000) ;	
		STMFLASH_Lock();
	}
	else
	{
		len = MAX_AVS * sizeof(Str_variable_point) ;
		STMFLASH_MUL_Read(AV_PAGE,(void *)&var[0].description[0], len );
	}
	
	temp = STMFLASH_ReadHalfWord(IN_PAGE_FLAG);
	if(temp == 0xffff)
	{
		STMFLASH_ErasePage(IN_PAGE_FLAG);
		for(loop=0; loop<MAX_INS; loop++ )
		{
			 
			memcpy(inputs[loop].description, Inputs_Description[loop], 21);
			memcpy(inputs[loop].label, Inputs_label[loop], 9); 
			inputs[loop].value = 0; 
			inputs[loop].filter = 5 ;
			inputs[loop].decom = 0 ;
			inputs[loop].sub_id = 0 ;
			inputs[loop].sub_product = 0 ;
			inputs[loop].control = 0 ;
			inputs[loop].auto_manual = 0 ;
			inputs[loop].digital_analog = 1 ;
			inputs[loop].calibration_sign = 0 ;
			inputs[loop].sub_number = 0 ;
			inputs[loop].calibration_hi = 0 ;
			inputs[loop].calibration_lo = 0 ;
			inputs[loop].range = 0 ; 
		}
		len = MAX_INS * sizeof(Str_in_point) ;
		memcpy(tempbuf,(void*)&inputs[0], len);		
		iap_write_appbin(IN_PAGE,(uint8_t*)tempbuf, len);	
		STMFLASH_WriteHalfWord(IN_PAGE_FLAG, 10000) ;
	}
	else
	{
		len = MAX_INS * sizeof(Str_in_point) ;
		STMFLASH_MUL_Read(IN_PAGE,(void *)&inputs[0].description[0], len ); 
	}

}

uint8 IO_Change_Flag[3];

void io_control(void)
{
	uint8 loop;
	int16 external_operation_value;
	if (IO_Change_Flag[IN_TYPE]) //write
	{
		IO_Change_Flag[IN_TYPE] = 0;
		
	//input0  temperature  
		if(inputs[0].range == R10K_40_120DegC)
		{
			deg_c_or_f = DEGREE_C;
			write_eeprom(EEP_DEG_C_OR_F, (uint8)deg_c_or_f);
		}
		else
		{
			deg_c_or_f = DEGREE_F;
			write_eeprom(EEP_DEG_C_OR_F, (uint8)deg_c_or_f);
		}
		 
		external_operation_value = inputs[0].value/100;
		if((output_auto_manual & 0x01) == 0x01)
		{
			output_manual_value_temp = external_operation_value;
			
		}
		else
		{ 
			external_operation_value = inputs[0].calibration_lo + ((int16)inputs[0].calibration_hi<<8); 
			if(inputs[0].calibration_sign) external_operation_value = 0 -external_operation_value;
			HumSensor.offset_t  = external_operation_value ;
//			external_operation_flag = 0;
			new_write_eeprom(EEP_TEMP_OFFSET,HumSensor.offset_t); 
			new_write_eeprom(EEP_TEMP_OFFSET+1,HumSensor.offset_t>>8); 
		}
			
		if(inputs[0].auto_manual)
			output_auto_manual |= 0x01; 
		else
			output_auto_manual &= 0xfe;
		
	//input1 humidity		  
		external_operation_value =  inputs[1].value / 100;
			
		if(output_auto_manual & 0x02)
			output_manual_value_humidity = external_operation_value;
		else  
		{
			external_operation_value = inputs[1].calibration_lo + ((int16)inputs[1].calibration_hi<<8); 
			if(inputs[1].calibration_sign) external_operation_value = 0 -external_operation_value;
			 
			if(table_sel== USER)
			{
				HumSensor.offset_h =external_operation_value;
				new_write_eeprom(EEP_HUM_OFFSET,HumSensor.offset_h); 
				new_write_eeprom(EEP_HUM_OFFSET+1,HumSensor.offset_h>>8);
			}
			else
			{
				HumSensor.offset_h_default = external_operation_value;
				new_write_eeprom(EEP_CAL_DEFAULT_HUM,HumSensor.offset_h_default); 
				new_write_eeprom(EEP_CAL_DEFAULT_HUM+1,HumSensor.offset_h_default>>8); 
			}
//			external_operation_flag = 0; 
			Run_Timer = 0;
		}
			
		HumSensor.H_Filter = inputs[1].filter;  
		write_eeprom(EEP_HUMIDITY_FILTER,HumSensor.H_Filter ); 
		
		
		if(inputs[1].auto_manual)
			output_auto_manual |= 0x02;
		else
			output_auto_manual &= 0xfd;
		
	//input2 co2		
		external_operation_value = inputs[2].value / 1000;

		if((output_auto_manual & 0x04) == 0x04)
		{
			output_manual_value_co2 =  external_operation_value;	
		}
		else
		{
			external_operation_value = inputs[2].calibration_lo + ((int16)inputs[2].calibration_hi<<8); 
			if(inputs[2].calibration_sign) external_operation_value = 0 -external_operation_value;
			 
			int_co2_str.co2_offset  = external_operation_value ;
			write_eeprom(EEP_INT_CO2_OFFSET, (uint8)(int_co2_str.co2_offset));
			write_eeprom(EEP_INT_CO2_OFFSET + 1, (uint8)(int_co2_str.co2_offset >> 8));
		}
		 
		int_co2_filter = inputs[2].filter;
		write_eeprom(EEP_CO2_FILTER, int_co2_filter);
		
		if(inputs[2].auto_manual)
			 output_auto_manual |= 0x04;
		else
			 output_auto_manual &= 0xFB;

	//input3 Pressure	
		if(Pressure.default_unit == inWC)
			external_operation_value = inputs[3].value/10;
		else if(Pressure.default_unit == Psi) 
			external_operation_value = inputs[3].value/100;
		
		if(Pressure.auto_manu & 0x04)	//manu mode
		{
			output_manual_value_co2 = external_operation_value;
		}
		else
		{ 
			external_operation_value = inputs[3].calibration_lo + ((int16)inputs[3].calibration_hi<<8); 
			if(inputs[3].calibration_sign) external_operation_value = 0 -external_operation_value;
			
			if(Pressure.default_unit == Psi) 
				Pressure.org_val_offset  = external_operation_value; 
			else if(Pressure.default_unit == inWC)
				Pressure.org_val_offset  = external_operation_value *10; 
			
			write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,Pressure.org_val_offset);
			write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,Pressure.org_val_offset >> 8);
		}
		Pressure.filter = inputs[3].filter;
		write_eeprom(EEP_PRESSURE_FILTER,Pressure.filter);
		Pressure.auto_manu = inputs[3].auto_manual;  

	//input4 PM2.5
		external_operation_value = inputs[4].value / 100;
		
//		if(external_operation_value > 0)
		{	 
			
			pm25_sensor.pm25 = external_operation_value;
			
			external_operation_value = inputs[4].calibration_lo + ((int16)inputs[4].calibration_hi<<8); 
			if(inputs[4].calibration_sign) external_operation_value = 0 -external_operation_value;
			pm25_sensor.pm25_offset  = external_operation_value; 
			write_eeprom(EEP_PM25_OFFSET, pm25_sensor.pm25_offset);
			write_eeprom(EEP_PM25_OFFSET + 1, pm25_sensor.pm25_offset>>8);
		} 
		pm25_sensor.PM25_filter = inputs[4].filter;
		write_eeprom(EEP_PM25_FILTER, pm25_sensor.PM25_filter); 
		
	//input5 PM10		
		external_operation_value = inputs[5].value / 100;
//		if(external_operation_value > 0)
		{	
//				itemp -= pm25_sensor.pm10;
			pm25_sensor.pm10 = external_operation_value;
			
			external_operation_value = inputs[5].calibration_lo + ((int16)inputs[5].calibration_hi<<8); 
			if(inputs[5].calibration_sign) external_operation_value = 0 -external_operation_value;
			pm25_sensor.pm10_offset  = external_operation_value; 
			 
			
			write_eeprom(EEP_PM10_OFFSET, pm25_sensor.pm10_offset);
			write_eeprom(EEP_PM10_OFFSET + 1, pm25_sensor.pm10_offset>>8);
		} 
		
		pm25_sensor.PM10_filter = inputs[5].filter;
		write_eeprom(EEP_PM10_FILTER, pm25_sensor.PM10_filter); 
		
	//input6 Light Sensor		
		
//		{ 
//			uint32 itemp;
//			itemp = inputs[6].value/1000;
//			light.k = itemp*light.k/light.val;
//			light.pre_val = (uint16)itemp;
//			light.val = light.pre_val;
//			write_eeprom(EEP_LIGHT_K,light.k); 
//			write_eeprom(EEP_LIGHT_K + 1,light.k>>8); 
//		} 
		 
		inputs[6].filter = light.filter;  
		light.filter = inputs[6].filter;
		write_eeprom(EEP_LIGHT_FILTER,inputs[6].filter);
	}
	else	//read
	{
	//input0  temperature	
		if(deg_c_or_f == DEGREE_C)
		{
			inputs[0].value = (int32)HumSensor.temperature_c * 100;
			inputs[0].range = R10K_40_120DegC ;
		}
		else
		{
			inputs[0].value = (int32)HumSensor.temperature_f * 100;
			inputs[0].range = R10K_40_250DegF;
		}
		 
		inputs[0].filter = HumSensor.T_Filter ; 
		inputs[0].auto_manual = output_auto_manual & 0x01; 
		
		external_operation_value = abs(HumSensor.offset_t);
		inputs[0].calibration_lo = external_operation_value;
		inputs[0].calibration_hi = external_operation_value>>8;
		if(HumSensor.offset_t < 0)
		{
			inputs[0].calibration_sign = 1;
		}
		else
		{
			inputs[0].calibration_sign = 0;
		}
		 
	//input1 humidity		
		inputs[1].value = (int32)HumSensor.humidity * 100; 
		inputs[1].filter = HumSensor.H_Filter ; 
		inputs[1].auto_manual = output_auto_manual & 0x02; 
		inputs[1].range = HUMIDITY;  
		 
		if(table_sel== USER)
		{
			external_operation_value = HumSensor.offset_h; 
		}
		else
		{
			external_operation_value = HumSensor.offset_h_default; 
		}
		if(external_operation_value < 0) inputs[1].calibration_sign = 1;
		else inputs[1].calibration_sign = 0;
		external_operation_value = abs(external_operation_value);
		
		inputs[1].calibration_lo = external_operation_value;
		inputs[1].calibration_hi = external_operation_value>>8;
			
		
	//input2 co2		
		inputs[2].value = (int32)int_co2_str.co2_int * 1000; 
		inputs[2].filter = int_co2_filter; 
		inputs[2].auto_manual = output_auto_manual & 0x04; 
		inputs[2].range = PPM;  
		 
		
		external_operation_value = int_co2_str.co2_offset; 
		if(external_operation_value < 0) inputs[2].calibration_sign = 1;
		else inputs[2].calibration_sign = 0;
		external_operation_value = abs(external_operation_value);
		
		inputs[2].calibration_lo = external_operation_value;
		inputs[2].calibration_hi = external_operation_value>>8;

	//input3 Pressure	
		if(Pressure.default_unit == inWC)
		{
			inputs[3].value = (int32)Pressure.org_val*10;
			inputs[3].range = INWC;
		}
		else if(Pressure.default_unit == Psi) 
		{
			inputs[3].value = (int32)Pressure.org_val*100;
			inputs[3].range = PSI ;
		}
		inputs[3].filter = Pressure.filter; 
		inputs[3].auto_manual = Pressure.auto_manu; 
		
	 
		if(Pressure.default_unit == Psi) 
			external_operation_value = Pressure.org_val_offset; 
		else if(Pressure.default_unit == inWC)
			external_operation_value = Pressure.org_val_offset / 10; 
		
		if(external_operation_value < 0) inputs[3].calibration_sign = 1;
		else inputs[3].calibration_sign = 0;
		external_operation_value = abs(external_operation_value);
		
		inputs[3].calibration_lo = external_operation_value;
		inputs[3].calibration_hi = external_operation_value>>8;

	//input4 PM2.5		
		inputs[4].value = (int32)pm25_sensor.pm25 * 100; 
		inputs[4].filter = pm25_sensor.PM25_filter; 
		inputs[4].auto_manual = 0; 
		inputs[4].range = 0 ; 
		
		external_operation_value = pm25_sensor.pm25_offset; 
		
		if(external_operation_value < 0) inputs[4].calibration_sign = 1;
		else inputs[4].calibration_sign = 0;
		external_operation_value = abs(external_operation_value);
		
		inputs[4].calibration_lo = external_operation_value;
		inputs[4].calibration_hi = external_operation_value>>8;
		
	//input5 PM10		
		inputs[5].value = (int32)pm25_sensor.pm10 * 100; 
		inputs[5].filter = pm25_sensor.PM10_filter; 
		inputs[5].auto_manual = 0; 
		inputs[5].range = 0 ; 
		
		external_operation_value = pm25_sensor.pm10_offset; 
		
		if(external_operation_value < 0) inputs[5].calibration_sign = 1;
		else inputs[5].calibration_sign = 0;
		external_operation_value = abs(external_operation_value);
		
		inputs[5].calibration_lo = external_operation_value;
		inputs[5].calibration_hi = external_operation_value>>8;
		
	//input6 Light Sensor		
		inputs[6].value =(int32)light.val * 1000; 
		inputs[6].filter = light.filter; 
		inputs[6].auto_manual = 0; 
		inputs[6].range = 0 ; 
	}
	
//analog output	
	for(loop=0; loop<MAX_OUTS; loop++ )
	{
		outputs[loop].value = (int32)analog_output[loop] * 10;  
		if(output_mode ==_4_20MA)
		{
			outputs[loop].range = I_0_20ma ; 
		}
		else if(output_mode ==_0_10V)
		{
			outputs[loop].range = V0_10;  
		}
		else 
		{
			outputs[loop].range = V0_10;  
		}
	}
 
}

