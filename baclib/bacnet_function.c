#include "stdint.h"
#include "types.h"
#include "define.h"
#include "usart.h"
#include "rs485.h"
#include "bacnet.h" 
#include "24cxx.h"
#include "modbus.h"
//#include "inputs.h"
#include "define.h"
#include "filter.h"
#include "registerlist.h"
#include "store.h"
#include "config.h"

#define AV_COMMON 0
#define AV_CO2 1
#define AV_HUM 2
#define AV_PM25 3
#define AV_PRESSURE 4

// type, av name
const uint8 AV_TYPE[MAX_AVS] = {
	AV_COMMON,   //0
	AV_COMMON,   //1
	AV_COMMON,   //2
	AV_COMMON,   //3
	AV_COMMON,  //4 
	AV_COMMON,  //5					 
	AV_COMMON,  //6
	AV_COMMON,	//7
	AV_COMMON,  //8
	AV_COMMON,	//9
	AV_COMMON, 	//10 
	AV_HUM,  		//11 HUM
	AV_HUM,     //12 HUM
	AV_HUM,			//13 HUM
	AV_HUM,//14 HUM
	AV_HUM,//15 HUM
	AV_HUM,//16 HUM
	AV_CO2,//17 CO2
	AV_PRESSURE,//18 PRESSURE
	AV_PM25,//19 PM25
	AV_PM25,//20 PM25
	AV_HUM,//21 HUM
	AV_HUM,//22 HUM
	AV_CO2,//23 CO2
	AV_PRESSURE,//24 PRESSURE
	AV_PM25,//25 PM25
	AV_PM25,//26 PM25
	AV_HUM,  //27 HUM
	AV_COMMON, //28
	 
	AV_HUM,//29 HUM
	AV_HUM,//30 HUM
	AV_HUM,//31 HUM
	AV_HUM,//32 HUM
	AV_CO2,//33 CO2
	AV_CO2,//34 CO2
	AV_PM25,  //35		PM25
	AV_COMMON, // 36 

};

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
	"AQI",	   //35
	"MaxMSTER",//36

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
	"dew point",				// HUM
	"Pws",							// HUM
	"MixRatio",					// HUM
	"Enthalpy",  				// HUM
	"OffSet Humdity",			// HUM
	"OffSet Tempeature", 		// HUM
	"OffSet Co2",				//  CO2
	"OffSet_Pressure",			// PRESSURE
	"OffSet_P25",				// PM25
	"OffSet_P10",				// PM25
	"Filter Humdity",			// HUM
	"Filter Temperature",		// HUM
	"Filter Co2",				// CO2
	"Filter_Pressure",			// PRESSURE
	"Filter_Pm25",				//PM25
	"Filter_Pm10",				//PM25
	"Temperature Unit",			//  HUM
	"OutMode",					//  
	"Humdity Min Range",		// HUM
	"Humdity Max Range",		// HUM 
	"Temperatur Min Range",		//  HUM
	"Temperatur Max Range",		//  HUM
	"Co2 Min Range",			// CO2
	"Co2 Max Range", 			// CO2
	"Air Quality Index",			// PM25
	"Max MSTP master"
};


	uint8_t panelname[21];

uint8_t AV_Index_To_Instance[MAX_AVS];
uint8_t AV_Instance_To_Index[MAX_AVS];
extern U8_T MAX_MASTER;

void Count_VAR_Object_Number(void)
{
	U8_T count,i;
	U8_T type;
	count = 0;
	
	for(i = 0;i < MAX_AVS;i++)
	{
		if(AV_TYPE[i] == AV_COMMON)
		{
			AV_Index_To_Instance[count] = i;
			AV_Instance_To_Index[i] = count;
			count++;
		}
		switch(PRODUCT_ID)
		{
			case STM32_PM25: 
				if(AV_TYPE[i] == AV_PM25)
				{
					AV_Index_To_Instance[count] = i;
					AV_Instance_To_Index[i] = count;
					count++;
				}
				break;
			case STM32_CO2_NET:
			case STM32_CO2_RS485:
			case STM32_CO2_NODE_NEW:
				if(AV_TYPE[i] == AV_CO2)
				{
					AV_Index_To_Instance[count] = i;
					AV_Instance_To_Index[i] = count;
					count++;
				}
				if(AV_TYPE[i] == AV_HUM)
				{
					AV_Index_To_Instance[count] = i;
					AV_Instance_To_Index[i] = count;
					count++;
				}
				break;
			case STM32_HUM_NET:
			case STM32_HUM_RS485:
				if(AV_TYPE[i] == AV_HUM)
				{
					AV_Index_To_Instance[count] = i;
					AV_Instance_To_Index[i] = count;
					count++;
				}
				break;
			case STM32_PRESSURE_NET:
			case STM32_PRESSURE_RS485:
				if(AV_TYPE[i] == AV_PRESSURE)
				{
					AV_Index_To_Instance[count] = i;
					AV_Instance_To_Index[i] = count;
					count++;
				}
				break;
			default:
				break;
		}
	}
	AVS = count;

}

int Get_Bacnet_Index_by_Number(U8_T number)
{
	U8_T count,i;
	count = 0;
	
	for(i = 0;i < MAX_AVS;i++)
	{
		if(AV_TYPE[i] == AV_COMMON)
		{
			if(i == number)
				return count;
			count++;
		}
		switch(PRODUCT_ID)
		{
			case STM32_PM25: 
				if(AV_TYPE[i] == AV_PM25)
				{
					if(i == number)
						return count;
					count++;
				}
				break;
			case STM32_CO2_NET:
			case STM32_CO2_RS485:
			case STM32_CO2_NODE_NEW:
				if(AV_TYPE[i] == AV_CO2 || AV_TYPE[i] == AV_HUM)
				{
					if(i == number)
						return count;
					count++;
				}
				break;
			case STM32_HUM_NET:
			case STM32_HUM_RS485:
				if(AV_TYPE[i] == AV_HUM)
				{
					if(i == number)
						return count;
					count++;
				}
				break;
			case STM32_PRESSURE_NET:
			case STM32_PRESSURE_RS485:
				if(AV_TYPE[i] == AV_PRESSURE)
				{
					if(i == number)
						return count;
					count++;
				}
				break;
			default:
				break;
		}
		
	}
	return -1;
}

int Get_Number_by_Bacnet_Index(U8_T index)
{
	U8_T count,i;
	count = 0;
	
	for(i = 0;i < MAX_AVS;i++)
	{
		if(AV_TYPE[i] == AV_COMMON)
		{
			if(count == index)
				return i;
			count++;
		}
		switch(PRODUCT_ID)
		{
			case STM32_PM25: 
				if(AV_TYPE[i] == AV_PM25)
				{
					if(count == index)
						return i;
					count++;
				}
				break;
			case STM32_CO2_NET:
			case STM32_CO2_RS485:
			case STM32_CO2_NODE_NEW:
				if(AV_TYPE[i] == AV_CO2 || AV_TYPE[i] == AV_HUM)
				{
					if(count == index)
						return i;
					count++;
				}
				break;
			case STM32_HUM_NET:
			case STM32_HUM_RS485:
				if(AV_TYPE[i] == AV_HUM)
				{
					if(count == index)
						return i;
					count++;
				}
				break;
			case STM32_PRESSURE_NET:
			case STM32_PRESSURE_RS485:
				if(AV_TYPE[i] == AV_PRESSURE)
				{
					if(count == index)
						return i;
					count++;
				}
				break;
			default:
				break;
		}		
	}
		
	return -1;
}


U8_T Get_index_by_AVx(uint8_t av_index,uint8_t *var_index)
{
	U8_T i;
		
	S8_T ret;
	ret = Get_Number_by_Bacnet_Index(av_index);
	if(ret != -1)
	{
		*var_index = ret;
		return 1;
	}
	else
	{
		return 0;
	}
}

//void bacnet_inital(void)
//{
//	uint8 i;
//	for(i = 0;i < MAX_VARS; i++) 
//	{	
//		memcpy(var[i].description,"test1",6); 
//		memcpy(var[i].label,"test2",6); 
//	}
//	for(i = 0;i < MAX_AOS; i++) 
//	{
//		memcpy(outputs[i].description,"test3",6); 
//		memcpy(outputs[i].label,"test4",6); 
//	}
//            
//}
 
void switch_to_modbus(void)
{
//	printf()
 
//	modbus.protocal = MODBUS;
// 	write_eeprom(EEP_MODBUS_COM_CONFIG, MODBUS);
//	
	if(modbus.baudrate  == BAUDRATE_19200)
		uart1_init(19200);
	else if(modbus.baudrate  == BAUDRATE_9600)
		uart1_init(9600);
	else if(modbus.baudrate  == BAUDRATE_38400)
		uart1_init(38400);
	else if(modbus.baudrate  == BAUDRATE_57600)
		uart1_init(57600);
	else if(modbus.baudrate  == BAUDRATE_76800)
		uart1_init(76800); 
	else if(modbus.baudrate  == BAUDRATE_115200)
		uart1_init(115200); 
}

uint16_t send_count;
//u16 far Test[50];

uint8_t RS485_Get_Baudrate(void)
{
 if(modbus.baudrate == BAUDRATE_9600)
  return 5;
 else if(modbus.baudrate == BAUDRATE_19200)
  return 6;
 else if(modbus.baudrate == BAUDRATE_38400)
  return 7;
 else if(modbus.baudrate == BAUDRATE_57600)
  return 8;
 else if(modbus.baudrate == BAUDRATE_115200)
  return 9;
 else if(modbus.baudrate == BAUDRATE_76800)
  return 10;  
 else 
  return 6;// default is 19200
}

//----------------------------------------------------------
void Get_AVS(void)
{
//	//bacnet_AV.reg.avs_num = 50;
//	bacnet_AV.reg.address = Modbus.address;
//	//	bacnet_AV.reg.product_model = Modbus.product_model;
//	bacnet_AV.reg.hardRev = Modbus.hardRev;
//	bacnet_AV.reg.firwareRev = SW_REV;
//	bacnet_AV.reg.tcp_type = Modbus.tcp_type;
//	memcpy(bacnet_AV.reg.ip_addr,Modbus.ip_addr,4);
//	memcpy(bacnet_AV.reg.mac_addr,Modbus.mac_addr,6);
//	memcpy(bacnet_AV.reg.subnet,Modbus.subnet,4);
//	memcpy(bacnet_AV.reg.getway,Modbus.getway,4);
//	bacnet_AV.reg.tcp_port = Modbus.tcp_port;
//	//	bacnet_AV.reg.mini_type = Modbus.mini_type;
//	memcpy(bacnet_AV.reg.com_config,Modbus.com_config,3);
//	bacnet_AV.reg.com_baudrate[0] = uart0_baudrate;
//	bacnet_AV.reg.com_baudrate[1] = uart1_baudrate;
//	bacnet_AV.reg.com_baudrate[2] = uart2_baudrate;
//	//	memcpy(bacnet_AV.reg.start_adc,Modbus.start_adc,11);
//	bacnet_AV.reg.network_number = Modbus.network_number;
//	bacnet_AV.reg.panel_number = Station_NUM;
//	
}
//modbus.input[0]
//----------------------------------------------
float Get_bacnet_value_from_buf(uint8_t type,uint8_t priority,uint8_t i)
{	
	float ftemp;
	uint8_t io_index;
//	if(i == 0) return 1;    //start from var1.
//	else i -= 1;
	switch(type)
	{  
		case AV: 			
			if(i < MAX_AVS)
			{ 
				Get_index_by_AVx(i,&io_index);
				switch (io_index)
				{
					case 0://serial number low byte
						var[io_index].value =((uint16)modbus.serial_Num[1]<<8)|modbus.serial_Num[0];
						break;
					case 1://serial number high byte
						var[io_index].value=((uint16)modbus.serial_Num[3]<<8)|modbus.serial_Num[2];
						break;
					case 2://software version
						var[io_index].value=SOFTREV;
						break;
					case 3: //id address 
						var[io_index].value = modbus.address;
						break; 
					case 4: //product_model  
						var[io_index].value = PRODUCT_ID;
						break;
					case 5: //Instance
						var[io_index].value = Instance;
						break;
					case 6: //Station
						var[io_index].value = Station_NUM;
						break;
					case 7: //baud rate
						var[io_index].value = modbus.baud;
						break;
					case 8: //update
						var[io_index].value =  modbus.update;
						break; 
					case 9: //protocol
						var[io_index].value =  modbus.protocal;
						break; 
					case 10: //auto manual
						var[io_index].value = output_auto_manual;
						break;
					 
					case 11: //dew point
						if(deg_c_or_f == DEGREE_C)
							var[io_index].value = (float)HumSensor.dew_pt/10;
						else
							var[io_index].value = (float)HumSensor.dew_pt_F/10;
						break; 
					case 12://pws
						var[io_index].value = (float)HumSensor.Pws/10;
						break;
					case 13://MixRatio
						var[io_index].value = (float)HumSensor.Mix_Ratio/10;
						break;
					case 14://Enthalpy
						var[io_index].value = (float)HumSensor.Enthalpy/10;
						break;
					case 15: //OffSet_H 
						if(table_sel == USER)
							var[io_index].value = (float)HumSensor.offset_h/10;
						else
							var[io_index].value = (float)HumSensor.offset_h_default/10;
						break;
					case 16: //OffSet_T 
						var[io_index].value = (float)HumSensor.offset_t/10 ;
						break; 
					case 17: //OffSet_C 
						var[io_index].value = (float)HumSensor.offset_t;
						break;
					case 18: //OffSet_P 
						if((Pressure.SNR_Model == PRS_26PCGFA)||(Pressure.SNR_Model == PRS_26PCDFA))
							var[io_index].value = (float)Pressure.org_val_offset/10;
						else
							var[io_index].value = (float)Pressure.org_val_offset/100;
						break;
					case 19: //OfSe_P25 
						var[io_index].value = (float)pm25_sensor.pm25_offset/10;
						break;
					case 20: //OfSe_P10 
						var[io_index].value = (float)pm25_sensor.pm10_offset/10;
						break; 
					case 21: //Filter_H 
						var[io_index].value = HumSensor.H_Filter;
						break;  
					case 22: //Filter_T 
						var[io_index].value = HumSensor.T_Filter;
						break; 
					case 23: //Filter_C 
						var[io_index].value = int_co2_filter;
						break; 
					case 24: //Filter_P 
						var[io_index].value = Pressure.filter;
						break; 
					case 25: //Filter_PM25 
						var[io_index].value = pm25_sensor.PM25_filter;
						break;
					case 26: //Filter_PM10 
						var[io_index].value = pm25_sensor.PM10_filter;
						break;  
					case 27: //T_Unit 
						var[io_index].value = deg_c_or_f;
						break;  
					case 28: //OutMode 
						var[io_index].value = output_mode;
						break; 
					 
					case 29: //MIN_RNG0 
						var[io_index].value = (float)output_range_table[CHANNEL_HUM].min/10;
						break; 
					case 30: //MAX_RNG0 
						var[io_index].value = (float)output_range_table[CHANNEL_HUM].max/10;
						break; 
					case 31: //MIN_RNG1 
						var[io_index].value = (float)output_range_table[CHANNEL_TEMP].min/10;
						break; 
					case 32: //MAX_RNG1 
						var[io_index].value = (float)output_range_table[CHANNEL_TEMP].max/10;
						break; 
					case 33: //MIN_RNG2 
						var[io_index].value = output_range_table[CHANNEL_CO2].min;
						break; 
					case 34: //MAX_RNG2 
						var[io_index].value = output_range_table[CHANNEL_CO2].max;
						break;
					case 35: //AQI 
						var[io_index].value = pm25_sensor.AQI;
						break;
					case 36: // max master
						var[io_index].value = MAX_MASTER;
						break;
					default: 
						var[io_index].value = 0;
					break;
				}
				
				return var[io_index].value;
			}
			break;
		case AI:
			if(i < MAX_INS)
			{ 
				switch(PRODUCT_ID)
				{
					case STM32_PM25:
						if(i == 0)		ftemp = (float)pm25_weight_25;//pm25_sensor.pm25/10;
						else if(i == 1)	ftemp =(float)pm25_weight_100;//pm25_sensor.pm10/10;
						break;
					case STM32_CO2_NET:		
					case STM32_CO2_RS485:
					case STM32_CO2_NODE_NEW:
						if(i == 0)	
						{
							if(deg_c_or_f == DEGREE_C)
								ftemp = (float)HumSensor.temperature_c/10;
							else
								ftemp = (float)HumSensor.temperature_f/10;
						}	
						else if(i == 1)	
							ftemp = (float)HumSensor.humidity/10;
						else if(i == 2)	
							ftemp = int_co2_str.co2_int;
						break;
					case STM32_HUM_NET:	//			212
					case STM32_HUM_RS485://				213
						if(i == 0)	
						{
							if(deg_c_or_f == DEGREE_C)
								ftemp = (float)HumSensor.temperature_c/10;
							else
								ftemp = (float)HumSensor.temperature_f/10;
						}	
						else if(i == 1)	
							ftemp = (float)HumSensor.humidity/10;
					case STM32_PRESSURE_NET: //			214
					case STM32_PRESSURE_RS485://		215
						if(i == 0)	
						{
						if((Pressure.SNR_Model == PRS_26PCGFA)||(Pressure.SNR_Model == PRS_26PCGFA))
								ftemp = (float)Pressure.org_val/10;
							else
								ftemp =(float)Pressure.org_val/100;
						}
						break;
					default:
						break;
				}				
			}
			return ftemp;		
		break;
		case AO: 
			if(i < MAX_AOS)
			{
				switch (i)
				{
					case 0:
						ftemp = (float)analog_output[CHANNEL_HUM]/100;	
					break;
					
					case 1:
						ftemp = (float)analog_output[CHANNEL_TEMP]/100;	
					break;
					
					case 2:
						ftemp = (float)analog_output[CHANNEL_CO2]/100;	
					break;
					
					default:break;
				}
				return ftemp ; 
			}
		break;
			
		case BO:			  
		break;			
		default:
			break;
				
	}	
	return 1;
}
//------------------------------------------------------------
void wirte_bacnet_value_to_buf(uint8_t type,uint8_t priority,uint8_t i,float value)
{
		uint8_t io_index;
		uint16 StartAdd;
//		if(i == 0) return;    //start from var1.
//		else i -= 1;
		switch(type)
		{
			case AV:  
				{	
					Get_index_by_AVx(i,&io_index);					
					switch(io_index)
					{
						case 3: //id address
							StartAdd = MODBUS_ADDRESS;
							var[io_index].value = value;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break;  
						case 5:  //instance
							StartAdd = MODBUS_PROTOCOL_TYPE;
							var[io_index].value = value;
							Instance = value;
							write_eeprom( EEP_INSTANCE_LOWORD,(uint8)Instance); 
							write_eeprom((EEP_INSTANCE_LOWORD + 1),(uint8)(Instance >> 8)); 
							write_eeprom((EEP_INSTANCE_LOWORD + 2),(uint8)(Instance >> 16)); 
							write_eeprom((EEP_INSTANCE_LOWORD + 3),(uint8)(Instance >> 24)); 
							break;
						case 6: //StaNum
							StartAdd = MODBUS_STATION_NUMBER;
							var[io_index].value = value;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break;
						case 7: //baud rate
							StartAdd = MODBUS_BAUDRATE;
							var[io_index].value = value;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break;
						case 8: //update
							StartAdd = MODBUS_UPDATE_STATUS;
							var[io_index].value = value;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break; 
						case 9: //protocol
							StartAdd = MODBUS_PROTOCOL_TYPE;
							var[io_index].value = value;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break; 
						case 10: //auto manual 
//								StartAdd = MODBUS_OUTPUT_AUTO_MANUAL;
							var[io_index].value = value;
							output_auto_manual = (uint8)value;
							break; 
						case 15: //OffSet_H 
							var[io_index].value = value*10;
							if(table_sel == USER)
							{
								HumSensor.offset_h = (int16)var[io_index].value;  
								write_eeprom(EEP_HUM_OFFSET,HumSensor.offset_h);
								write_eeprom(EEP_HUM_OFFSET + 1,HumSensor.offset_h >> 8);
							}
							else
							{
								HumSensor.offset_h_default = (int16)var[io_index].value;   
								write_eeprom(EEP_CAL_DEFAULT_HUM ,HumSensor.offset_h_default);
								write_eeprom(EEP_CAL_DEFAULT_HUM + 1,HumSensor.offset_h_default >> 8);
							}
							break;
						case 16: //OffSet_T 
							var[io_index].value = value*10;
							HumSensor.offset_t = (int16)var[io_index].value;    
							write_eeprom(EEP_TEMP_OFFSET,HumSensor.offset_t);
							write_eeprom(EEP_TEMP_OFFSET + 1,HumSensor.offset_t>>8);
							break; 
						case 17: //OffSet_C   
							var[io_index].value = value;
							int_co2_str.co2_offset =  (int16)value;
							write_eeprom(EEP_INT_CO2_OFFSET, (uint8)(int_co2_str.co2_offset));
							write_eeprom(EEP_INT_CO2_OFFSET + 1, (uint8)(int_co2_str.co2_offset >> 8));
							break; 
						case 18: //OffSet_Pressure 
							if((Pressure.SNR_Model == PRS_26PCGFA)||(Pressure.SNR_Model == PRS_26PCGFA))
								var[io_index].value = value*10;
							else
								var[io_index].value = value*100;
							Pressure.org_val_offset = (int16)var[io_index].value ;  
							write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,Pressure.org_val_offset);
							write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,Pressure.org_val_offset >> 8);
							break;
						case 19://offset pm2.5
							var[io_index].value = value*10;
							pm25_sensor.pm25_offset = (int16)var[io_index].value;
							write_eeprom(EEP_PM25_OFFSET, pm25_sensor.pm25_offset);
							write_eeprom(EEP_PM25_OFFSET + 1, pm25_sensor.pm25_offset>>8);
							break;
						case 20://offset pm10
							var[io_index].value = value*10;
							pm25_sensor.pm10_offset = (int16)var[io_index].value;
							write_eeprom(EEP_PM10_OFFSET, pm25_sensor.pm10_offset);
							write_eeprom(EEP_PM10_OFFSET + 1, pm25_sensor.pm10_offset>>8);
							break;
						case 21: //Filter_H 
							var[io_index].value = value;
							HumSensor.H_Filter = (uint8)var[io_index].value;
							write_eeprom(EEP_HUMIDITY_FILTER,HumSensor.H_Filter); 
							break; 
						case 22: //Filter_T 
							var[io_index].value = value;
							HumSensor.T_Filter = (uint8)var[io_index].value;
							write_eeprom(EEP_EXT_TEMPERATURE_FILTER,HumSensor.T_Filter);
							break; 
						case 23: //Filter_C 
							var[io_index].value = value;
							int_co2_filter = (uint8)var[io_index].value;
							write_eeprom(EEP_CO2_FILTER, int_co2_filter);
//								StartAdd = MODBUS_CO2_FILTER;
//								var[i].value = value;
//								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
							break; 
						case 24://filter pressure
							var[io_index].value = value;
							Pressure.filter = (uint8)var[io_index].value;
							write_eeprom(EEP_PRESSURE_FILTER,Pressure.filter);
						case 25://filter pm2.5
							var[io_index].value = value;
							pm25_sensor.PM25_filter = (uint8)var[io_index].value;
							write_eeprom(EEP_PM25_FILTER,pm25_sensor.PM25_filter);
						case 26://filter pm10
							var[io_index].value = value;
							pm25_sensor.PM10_filter = (uint8)var[io_index].value;
							write_eeprom(EEP_PM10_FILTER,pm25_sensor.PM10_filter);
						case 27: //T_Unit 
							var[io_index].value = value;
							if(var[io_index].value)
								deg_c_or_f = 1;
							else
								deg_c_or_f = 0;
							write_eeprom(EEP_DEG_C_OR_F, (uint8)deg_c_or_f);
							break;  
						case 29: //MIN_RNG0 
							StartAdd = MODBUS_OUTPUT_RANGE_MIN_HUM;
							var[io_index].value = value*10;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break; 
						case 30: //MAX_RNG0 
							StartAdd = MODBUS_OUTPUT_RANGE_MAX_HUM;
							var[io_index].value = value*10;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break; 
						case 31: //MIN_RNG1 
							StartAdd = MODBUS_OUTPUT_RANGE_MIN_TEM;
							var[io_index].value = value*10;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break; 
						case 32: //MAX_RNG1 
							StartAdd = MODBUS_OUTPUT_RANGE_MAX_TEM;
							var[io_index].value = value*10;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break; 
						case 33: //MIN_RNG2 
							StartAdd = MODBUS_OUTPUT_RANGE_MIN_CO2;
							var[io_index].value = value;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break; 
						case 34: //MAX_RNG2 
							StartAdd = MODBUS_OUTPUT_RANGE_MAX_CO2;
							var[io_index].value = value;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break;	
						case 36:
							StartAdd = MODBUS_MSTP_MAX_MASTER;
							var[io_index].value = value;
							Data_Deal(StartAdd,(int16)var[io_index].value>>8,var[io_index]. value); 
							break;	
						default: 
							break;
					}
				}
			 
			break;
			case AI:
				if(i < MAX_INS)
				{ 	
					s16 itemp;
					switch(PRODUCT_ID)
					{
						case STM32_PM25:
							if(i == 0)
						{
							inputs[i].value = value*10;
							itemp = (unsigned int)inputs[i].value ;
							if(itemp > 0)
							{	
				//				itemp -= pm25_sensor.pm25;
								pm25_sensor.pm25_offset += (itemp - pm25_sensor.pm25);
								pm25_sensor.pm25 = itemp;
								write_eeprom(EEP_PM25_OFFSET, pm25_sensor.pm25_offset);
								write_eeprom(EEP_PM25_OFFSET + 1, pm25_sensor.pm25_offset>>8);
							}
						}
						break;
						case STM32_CO2_NET:
						case STM32_CO2_RS485:	
						case STM32_CO2_NODE_NEW:
						if(i == 0) //temperature
						{
							inputs[i].value = value*10;
							if(deg_c_or_f == DEGREE_C)
							{
								external_operation_value = (int16)inputs[i].value;
								if((output_auto_manual & 0x01) == 0x01)
								{
									output_manual_value_temp = external_operation_value;
								}
								else
									external_operation_flag = TEMP_CALIBRATION;
							} 
							else
							{
								external_operation_value = ((int16)inputs[i].value - 320) * 5 / 9;
								if((output_auto_manual & 0x01) == 0x01)
								{
									output_manual_value_temp = external_operation_value;
								}
								else
									external_operation_flag = TEMP_CALIBRATION;
							}
						}
						if(i == 1)//humidity
						{
							inputs[i].value = value*10;
							external_operation_value =  (int16)inputs[i].value;
			
							if(output_auto_manual & 0x02)
								output_manual_value_humidity = external_operation_value;
							else if(external_operation_value < 950)  //< 95%
							{
								external_operation_flag = HUM_CALIBRATION;
								Run_Timer = 0;
							}
						}
						if(i == 2)//CO2
						{
							inputs[i].value = value;
							if((output_auto_manual & 0x04) == 0x04)
							{
								output_manual_value_co2 =  (int16)inputs[i].value;	
							}
							else
							{
								int_co2_str.co2_offset += (int16)inputs[i].value - int_co2_str.co2_int;
								write_eeprom(EEP_INT_CO2_OFFSET, (uint8)(int_co2_str.co2_offset));
								write_eeprom(EEP_INT_CO2_OFFSET + 1, (uint8)(int_co2_str.co2_offset >> 8));
							}							
						}
						case STM32_HUM_NET	:		
						case STM32_HUM_RS485:		
						if(i == 0) //temperature
						{
							inputs[i].value = value*10;
							if(deg_c_or_f == DEGREE_C)
							{
								external_operation_value = (int16)inputs[i].value;
								if((output_auto_manual & 0x01) == 0x01)
								{
									output_manual_value_temp = external_operation_value;
								}
								else
									external_operation_flag = TEMP_CALIBRATION;
							} 
							else
							{
								external_operation_value = ((int16)inputs[i].value - 320) * 5 / 9;
								if((output_auto_manual & 0x01) == 0x01)
								{
									output_manual_value_temp = external_operation_value;
								}
								else
									external_operation_flag = TEMP_CALIBRATION;
							}
						}
						if(i == 1)//humidity
						{
							inputs[i].value = value*10;
							external_operation_value =  (int16)inputs[i].value;
			
							if(output_auto_manual & 0x02)
								output_manual_value_humidity = external_operation_value;
							else if(external_operation_value < 950)  //< 95%
							{
								external_operation_flag = HUM_CALIBRATION;
								Run_Timer = 0;
							}
						}							
							break;
						case STM32_PRESSURE_NET	:		
						case STM32_PRESSURE_RS485	:	
							if(i == 0)
							{
								if((Pressure.SNR_Model == PRS_26PCGFA)||(Pressure.SNR_Model == PRS_26PCGFA))
								{
									inputs[i].value = value*10;
								}
								else
									inputs[i].value = value*100;
								 
								if(output_auto_manual & 0x04)	//manu mode
								{
									output_manual_value_co2 = (uint16)inputs[i].value;
								}
								else
								{
									itemp = (uint16)inputs[i].value;
									Pressure.org_val_offset += (itemp - Pressure.org_val );
									Pressure.org_val =  itemp;
									write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,Pressure.org_val_offset);
									write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,Pressure.org_val_offset >> 8);
								}
							}
							break;
						default:
							break;
					}					
				}
			break;
		
			default:
			break;
		}			

}
//-------------------------------------------------
void write_bacnet_name_to_buf(uint8_t type,uint8_t priority,uint8_t i,char* str)
{ 
}
//---------------------------------------------------
void write_bacnet_unit_to_buf(uint8_t type,uint8_t priority,uint8_t i,uint8_t unit)
{
}
//------------------------------------------------------------
char Get_Out_Of_Service(uint8_t type,uint8_t num)
{	
	if (type == AV) 
	{
		if(num < MAX_AVS) return 1;
//		output_auto_manual &= ~(1 << i); 
//		output_auto_manual |= (am << i); 
//		eeprom_write_byte(EEP_AUTO_MANUAL, output_auto_manual);
	}
	return 0;
	
}
//------------------------------------------------------------
void write_Out_Of_Service(uint8_t type,uint8_t i,uint8_t am)
{
	if (type == AV) 
	{
		if(i < MAX_AVS)  ;
//		output_auto_manual &= ~(1 << i); 
//		output_auto_manual |= (am << i); 
//		eeprom_write_byte(EEP_AUTO_MANUAL, output_auto_manual);
	}
}
//------------------------------------------------------------
void add_remote_panel_db(uint32_t device_id,uint8_t panel)
{				
}
//------------------------------------------------------------

char* get_label(uint8_t type,uint8_t num)
{
	U8_T io_index;
	switch(type)
   {
        case AV: 
				Get_index_by_AVx(num,&io_index);	
				if(io_index < MAX_AVS)
				{
					return (char *)Var_label[io_index];//var[num].label; 
				}  
        break;
       case AI:
			 if(num < MAX_INS)
			 {
				 if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485)||(PRODUCT_ID == STM32_CO2_NODE_NEW))
				 {
						if(num == 0)
						 return "Temperature";
					 else if(num == 1)
						 return "Humidity";
					 else if(num == 2)
						 return "CO2";
				 }
				 else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				 {
					if(num == 0)
						return (char *)"PRESSURE";
				 }
				 else if(PRODUCT_ID == STM32_PM25) 
				 {
					 if(num == 0)
						 return "PM2.5";
					 else if(num == 1)
						 return "PM10";
				 }
				 else if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
				 {
						if(num == 0)
						 return "Temperature";
					 else if(num == 1)
						 return "Humidity";
				 }
			 }
            break;
         case AO: 
			 {
				 uint8 AOS_TEMP;
				 if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485)||(PRODUCT_ID == STM32_CO2_NODE_NEW))
				 {
					 	if(num == 0)
							return "Temp_AO";
						else if(num == 1)
							return "HUM_AO";
						else if(num == 2)
							return "CO2_AO";
				 }
				 else if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
				 {
					 	if(num == 0)
							return "Temp_AO";
						else if(num == 1)
							return "HUM_AO";
				 }
				 else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				 {
					 if(num == 0)
							return "PESUR_AO";
				 }
				 else if(PRODUCT_ID == STM32_PM25) 
				 { //AOS_TEMP = 2;
						if(num == 0)
							return "PM2.5_AO";
						else if(num == 1)
							return "PM10_AO";
				 }
					
			 }
        break;
         default:
         break;
      }
	  return "null";
}
char* get_description(uint8_t type,uint8_t num)
{
//	if(num == 0) return "null";    //start from var1.
//	else num -= 1;
	uint8_t io_index;
	switch(type)
	{
		 case AV: 
			Get_index_by_AVx(num,&io_index);		
			if(io_index < MAX_AVS) 
				return (char *)var[io_index].description;   
				break;
		 case AI: 
			if(num < MAX_INS)
				return (char *)inputs[num].description;
						break;
		 case AO: 
			 if(num < MAX_AOS)
				return (char *)outputs[num].description;  
		 
				break;
			case BO: 
				break;
		 default:
		 break;
	}
	return "null";
}

char get_range(uint8_t type,uint8_t num)
{ 
	uint8_t io_index;
	switch(type)
	{
		case AV: 
//			Get_index_by_AVx(num,&io_index);
//			if(io_index == 14)	
//				return UNITS_KILOJOULES_PER_KILOGRAM_DRY_AIR;
//			else if(io_index == 14)	
			return 	UNITS_NO_UNITS; 
				
			break;
		
		case AI:
			if(PRODUCT_ID == STM32_PM25)
			{
				return 	UNITS_NO_UNITS; 
			}
			else if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485)||(PRODUCT_ID == STM32_CO2_NODE_NEW) \
				|| (PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
			{
			if(num == 0)
			{
				if(deg_c_or_f == DEGREE_C)
					return 	UNITS_DEGREES_CELSIUS ;
				else	
					return 	UNITS_DEGREES_FAHRENHEIT ;					
			}
			else if(num == 1)
				return 	UNITS_PERCENT_RELATIVE_HUMIDITY ; 
			}
			else if(PRODUCT_ID == STM32_PRESSURE_NET || PRODUCT_ID == STM32_PRESSURE_RS485)
				{// TBD:
					if(Pressure.default_unit == inWC)					
						return UNITS_INCHES_OF_WATER;					
					else if(Pressure.default_unit == Psi)
						return 	UNITS_POUNDS_FORCE_PER_SQUARE_INCH;
					else 
						return UNITS_NO_UNITS;
			}
				
		break;
		
		case AO:
			return 	UNITS_NO_UNITS ; 
		break;		
	}
	
}

void Set_Object_Name(char * name)	
{
	u8 temp = strlen(name);
	if(temp > 20) temp = 20;
	memcpy(panelname,name,temp + 1);   
	//AT24CXX_Write(EEP_TSTAT_NAME1, panelname,temp+ 1); 
}
void write_bacnet_description_to_buf(uint8_t type, uint8_t priority, uint8_t i, char* str)
{
}	

char* Get_Object_Name(void)
{
	return (char*)panelname;
} 
#if BAC_SCHEDULE 
BACNET_TIME_VALUE* Get_Time_Value(uint8_t object_index,uint8_t day,uint8_t i) {return 0;}
uint8_t Get_TV_count(uint8_t object_index,uint8_t day) {return 0;}
BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * Get_Object_Property_References(uint8_t i) {return 0;} 
void write_Time_Value(uint8_t index,uint8_t day,uint8_t i,BACNET_TIME_VALUE time_value) {;}
#endif

#if BAC_CALENDAR
 
uint8_t Get_CALENDAR_count(uint8_t object_index) {return 0;}
BACNET_DATE* Get_Calendar_Date(uint8_t object_index,uint8_t i) {return 0;} 
void write_annual_date(uint8_t index,BACNET_DATE date){;}
	
#endif
 

	
void uart_send_string(U8_T *p, U16_T length,U8_T port) 
{
	 memcpy(uart_send, p, length);
	USART_SendDataString(length);
}

u8 	UART_Get_SendCount(void)
{
	return 1;
}

void Set_TXEN(u8 dir)
{
	if(dir)
		TXEN = 1;
	else
		TXEN = 0;
}
	
float Get_Output_Relinguish(uint8_t type,uint8_t i)
{
	return 0;
}	

void write_Output_Relinguish(uint8_t type,uint8_t i,float value)
{
	
}

U16_T Get_Vendor_ID(void)
{
//	switch(Bacnet_Vendor_ID)
//	{
//		case 1: //netixcontrols
//			bacnet_vendor_name = BACNET_VENDOR_NETIX;
//		bacnet_vendor_product = BACNET_PRODUCT_NETIX;
//			return BACNET_VENDOR_ID_NETIX;
//		case 2: // jet controls
//			bacnet_vendor_name = BACNET_VENDOR_JET;
//		bacnet_vendor_product = BACNET_PRODUCT_JET;
//			return BACNET_VENDOR_ID_JET;
//		default: // temco controls
//			bacnet_vendor_name = BACNET_VENDOR_TEMCO;
//			bacnet_vendor_product = BACNET_PRODUCT_TEMCO;
//			return BACNET_VENDOR_ID_TEMCO;  
//	}
	return 148;
}



const char*  Get_Vendor_Name(void)
{
//	switch(Bacnet_Vendor_ID)
//	{
//		case 1: //netixcontrols
//			return BACNET_VENDOR_NETIX;
//		case 2: // jet controls
//			return BACNET_VENDOR_JET;
//		default: // temco controls
//			return BACNET_VENDOR_TEMCO;  
//	}
	//return "temco controls";
	return "TemcoControls";
}

const char*  Get_Vendor_Product(void)
{
//	switch(Bacnet_Vendor_ID)
//	{
//		case 1: //netixcontrols
//			return BACNET_VENDOR_NETIX;
//		case 2: // jet controls
//			return BACNET_VENDOR_JET;
//		default: // temco controls
//			return BACNET_VENDOR_TEMCO;  
//	}
	if(PRODUCT_ID == STM32_CO2_NET)
		return "CO2_NET";
	else if(PRODUCT_ID == STM32_CO2_RS485)
		return "CO2_RS485";
	else if(PRODUCT_ID == STM32_CO2_NODE_NEW)
		return "CO2_NODE";
	else if(PRODUCT_ID == STM32_HUM_NET)
		return "HUM_NET";
	else if(PRODUCT_ID == STM32_HUM_RS485)
		return "HUM_RS485";
	else if(PRODUCT_ID == STM32_PRESSURE_NET)
		return "PRESSURE_NET";
	else if(PRODUCT_ID == STM32_PRESSURE_RS485)
		return "PRESSURE_RS485";
	else if(PRODUCT_ID == STM32_PM25)
		return "PM2.5_NET";
	else
		return "null";
}

uint8_t Get_modbus_address(void)
{
	return modbus.address;
}

bool Analog_Input_Change_Of_Value(unsigned int object_instance)
{
		return false;
}

bool Analog_Value_Change_Of_Value(unsigned int object_instance)
{	
	return false;
}	

void Store_Instance_To_Eeprom(uint32_t Instance)
{
	AT24CXX_WriteOneByte((u16)EEP_INSTANCE_1, (Instance>>24)&0xff);
	AT24CXX_WriteOneByte((u16)EEP_INSTANCE_2, (Instance>>16)&0xff);
	AT24CXX_WriteOneByte((u16)EEP_INSTANCE_3, (Instance>>8)&0xff);
	AT24CXX_WriteOneByte((u16)EEP_INSTANCE_4, Instance&0xff);
}

void Store_MASTER_To_Eeprom(uint8_t master)
{
	AT24CXX_WriteOneByte(EEP_MAX_MASTER,master);
}

uint8_t AI_Index_To_Instance[MAX_INS];
//uint8_t BI_Index_To_Instance[MAX_INS];

uint8_t AI_Instance_To_Index[MAX_INS];
//uint8_t BI_Instance_To_Index[MAX_INS];

BACNET_POLARITY Binary_Output_Polarity(
    uint32_t object_instance)
{
	
	return POLARITY_NORMAL;
}

bool Binary_Output_Polarity_Set(
    uint32_t object_instance,
    BACNET_POLARITY polarity)
{
	return false;
}

void Set_Vendor_Name(char* name)
{
//	write_page_en[EN_OTHER] = 1;
//	memcpy(bacnet_vendor_name,name,20);
}


void Set_Vendor_Product(char* product)
{
//	write_page_en[EN_OTHER] = 1;
//	memcpy(bacnet_vendor_product,product,20);
}

void Set_Vendor_ID(uint16_t vendor_id)
{

//	AT24CXX_WriteOneByte(EEP_BAC_VENDOR_ID_LO ,vendor_id);
//	AT24CXX_WriteOneByte(EEP_BAC_VENDOR_ID_HI ,vendor_id >> 8);
//	if((vendor_id == 0) || (vendor_id == 255) || (vendor_id == 65535) // temco
//		|| (vendor_id == 1)  // netIX
//		|| (vendor_id == 2))  // JET
//	{
//		//Bacnet_Vendor_ID = 
//		
//		return;
//	}
//	else
//		Bacnet_Vendor_ID = vendor_id;
	
}

char get_current_mstp_port(void)
{// no matter which port , same result. T3 only has one UART
	return 0;

}
//BACNET_POLARITY Binary_Input_Polarity(
//    uint32_t object_instance)
//{
////?????????????????????	
//	return POLARITY_NORMAL;
//}

//bool Binary_Input_Polarity_Set(
//    uint32_t object_instance,
//    BACNET_POLARITY polarity)
//{
//		bool status = false;

////???????????????????
//		return status;
//}

