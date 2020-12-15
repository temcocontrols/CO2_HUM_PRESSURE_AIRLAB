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
//#ifdef CO2_SENSOR
//	uint8_t panelname[21] = "CO2_NET";
//#elif defined PRESSURE_SENSOR	
//	uint8_t panelname[21] = "Pressure";
//#elif defined HUM_SENSOR	
//	uint8_t panelname[21] = "Humdity";
//#else
//	uint8_t panelname[21] = "CO2_NET";
//#endif 

	uint8_t panelname[21];
	
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
//	if(i == 0) return 1;    //start from var1.
//	else i -= 1;
	switch(type)
	{  
		case AV: 
			
			if(i < MAX_AVS)
			{ 
				switch (i)
				{
					case 0://serial number low byte
						var[i].value =((uint16)modbus.serial_Num[1]<<8)|modbus.serial_Num[0];
						break;
					case 1://serial number high byte
						var[i].value=((uint16)modbus.serial_Num[3]<<8)|modbus.serial_Num[2];
						break;
					case 2://software version
						var[i].value=SOFTREV;
						break;
					case 3: //id address 
						var[i].value = modbus.address;
						break; 
					case 4: //product_model  
						var[i].value = PRODUCT_ID;
						break;
					case 5: //Instance
						var[i].value = Instance;
						break;
					case 6: //Station
						var[i].value = Station_NUM;
						break;
					case 7: //baud rate
						var[i].value = modbus.baud;
						break;
					case 8: //update
						var[i].value =  modbus.update;
						break; 
					case 9: //protocol
						var[i].value =  modbus.protocal;
						break; 
					case 10: //auto manual
						var[i].value = output_auto_manual;
						break;
					 
					case 11: //dew point
						if(deg_c_or_f == DEGREE_C)
							var[i].value = (float)HumSensor.dew_pt/10;
						else
							var[i].value = (float)HumSensor.dew_pt_F/10;
						break; 
					case 12://pws
						var[i].value = (float)HumSensor.Pws/10;
						break;
					case 13://MixRatio
						var[i].value = (float)HumSensor.Mix_Ratio/10;
						break;
					case 14://Enthalpy
						var[i].value = (float)HumSensor.Enthalpy/10;
						break;
					case 15: //OffSet_H 
						if(table_sel == USER)
							var[i].value = (float)HumSensor.offset_h/10;
						else
							var[i].value = (float)HumSensor.offset_h_default/10;
						break;
					case 16: //OffSet_T 
						var[i].value = (float)HumSensor.offset_t/10 ;
						break; 
					case 17: //OffSet_C 
						var[i].value = (float)HumSensor.offset_t;
						break;
					case 18: //OffSet_P 
						if((Pressure.SNR_Model == PRS_26PCGFA)||(Pressure.SNR_Model == PRS_26PCDFA))
							var[i].value = (float)Pressure.org_val_offset/10;
						else
							var[i].value = (float)Pressure.org_val_offset/100;
						break;
					case 19: //OfSe_P25 
						var[i].value = (float)pm25_sensor.pm25_offset/10;
						break;
					case 20: //OfSe_P10 
						var[i].value = (float)pm25_sensor.pm10_offset/10;
						break; 
					case 21: //Filter_H 
						var[i].value = HumSensor.H_Filter;
						break;  
					case 22: //Filter_T 
						var[i].value = HumSensor.T_Filter;
						break; 
					case 23: //Filter_C 
						var[i].value = int_co2_filter;
						break; 
					case 24: //Filter_P 
						var[i].value = Pressure.filter;
						break; 
					case 25: //Filter_PM25 
						var[i].value = pm25_sensor.PM25_filter;
						break;
					case 26: //Filter_PM10 
						var[i].value = pm25_sensor.PM10_filter;
						break;  
					case 27: //T_Unit 
						var[i].value = deg_c_or_f;
						break;  
					case 28: //OutMode 
						var[i].value = output_mode;
						break; 
					 
					case 29: //MIN_RNG0 
						var[i].value = (float)output_range_table[CHANNEL_HUM].min/10;
						break; 
					case 30: //MAX_RNG0 
						var[i].value = (float)output_range_table[CHANNEL_HUM].max/10;
						break; 
					case 31: //MIN_RNG1 
						var[i].value = (float)output_range_table[CHANNEL_TEMP].min/10;
						break; 
					case 32: //MAX_RNG1 
						var[i].value = (float)output_range_table[CHANNEL_TEMP].max/10;
						break; 
					case 33: //MIN_RNG2 
						var[i].value = output_range_table[CHANNEL_CO2].min;
						break; 
					case 34: //MAX_RNG2 
						var[i].value = output_range_table[CHANNEL_CO2].max;
						break;
					case 35: //AQI 
						var[i].value = pm25_sensor.AQI;
						break;
//					case 35: //PID_SEL 
//						var[i].value = mode_select;
//						break; 
//					case 36: //PID1_MODE 
//						var[i].value = controllers[0].action;
//						break; 
//					case 37: //PID1_SP 
//						var[i].value = (float)PID[0].EEP_SetPoint/10;
//						break; 
//					case 38: //PID1_P 
//						var[i].value = controllers[0].proportional;
//						break;
//					case 39: //PID1_I 
//						var[i].value = controllers[0].reset;
//						break; 
//					case 40: //PID1_V 
//						var[i].value = PID[0].EEP_Pid;
//						break;
//					case 41: //PID2_MODE 
//						var[i].value = controllers[1].action;
//						break;
//					case 42: //PID2_SP 
//						var[i].value = (float)PID[1].EEP_SetPoint/10;
//						break;  
//					case 43: //PID2_P 
//						var[i].value = controllers[1].proportional;
//						break; 
//					case 44: //PID2_I 
//						var[i].value = controllers[1].reset;
//						break; 
//					case 45: //PID2_V 
//						var[i].value = PID[1].EEP_Pid;
//						break;
//					case 46: //PID3_MODE 
//						var[i].value = controllers[2].action;
//						break;
//					case 47: //PID3_SP 
//						var[i].value = PID[2].EEP_SetPoint;
//						break; 
//					case 48: //PID3_P 
//						var[i].value = controllers[2].proportional;
//						break;
//					case 49: //PID3_I 
//						var[i].value = controllers[2].reset;
//						break;
//					case 50: //PID3_V 
//						var[i].value = PID[2].EEP_Pid;
//						break; 
//					case 51: //AlarmAM 
//						var[i].value = alarm_state;
//						break;
//					case 52: //Alarm sp 
//						var[i].value = int_co2_str.alarm_setpoint;
//						break;
//					case 53: //PreAlarm sp 
//						var[i].value = int_co2_str.pre_alarm_setpoint;
//						break;
//					case 54: //Time_On 
//						var[i].value = pre_alarm_on_time;
//						break;
//					case 55: //Time_Off 
//						var[i].value = pre_alarm_off_time;
//						break;  
					default: var[i].value =0;break;
				}
				
				return var[i].value;
			}
			break;
		case AI:
			if(i < MAX_INS)
			{ 
				switch (i)
				{
					case 0: //temperature
						if(deg_c_or_f == DEGREE_C)
							ftemp = (float)HumSensor.temperature_c/10;
						else
							ftemp = (float)HumSensor.temperature_f/10;
						break;
					case 1: //humidity
						ftemp = (float)HumSensor.humidity/10;
						break;
					case 2://co2
						ftemp = int_co2_str.co2_int;
						break;
					case 3://pressure
						if((Pressure.SNR_Model == PRS_26PCGFA)||(Pressure.SNR_Model == PRS_26PCGFA))
							ftemp = (float)Pressure.org_val/10;
						else
							ftemp =(float)Pressure.org_val/100;
						break;
					case 4://PM2.5
						ftemp = (float)pm25_sensor.pm25/10;
						break;
					case 5://PM10
						ftemp =(float)pm25_sensor.pm10/10;
						break; 
					case 6://light sensor
						ftemp =(float)light.val;
						break; 
					default:break;
						
				}
				return ftemp;
			}
						
			 
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

		uint16 StartAdd;
//		if(i == 0) return;    //start from var1.
//		else i -= 1;
		switch(type)
		{
			case AV:  
				{
					
					switch (i)
					{
						case 3: //id address
							StartAdd = MODBUS_ADDRESS;
							var[i].value = value;
							Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
							break;  
						case 5:  //instance
							StartAdd = MODBUS_PROTOCOL_TYPE;
							var[i].value = value;
							Instance = value;
							write_eeprom( EEP_INSTANCE_LOWORD     ,(uint8)Instance); 
							write_eeprom((EEP_INSTANCE_LOWORD + 1),(uint8)(Instance >> 8)); 
							write_eeprom((EEP_INSTANCE_LOWORD + 2),(uint8)(Instance >> 16)); 
							write_eeprom((EEP_INSTANCE_LOWORD + 3),(uint8)(Instance >> 24)); 
							break;
						case 6: //StaNum
							StartAdd = MODBUS_STATION_NUMBER;
							var[i].value = value;
							Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
							break;
						case 7: //baud rate
							StartAdd = MODBUS_BAUDRATE;
							var[i].value = value;
							Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
							break;
						case 8: //update
							StartAdd = MODBUS_UPDATE_STATUS;
							var[i].value = value;
							Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
							break; 
						case 9: //protocol
							StartAdd = MODBUS_PROTOCOL_TYPE;
							var[i].value = value;
							Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
							break; 
						default:break;
					}
//					if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
					{
						switch (i)
						{ 
							case 10: //auto manual 
//								StartAdd = MODBUS_OUTPUT_AUTO_MANUAL;
								var[i].value = value;
								output_auto_manual = (uint8)value;
								break; 
							case 15: //OffSet_H 
								var[i].value = value*10;
								if(table_sel == USER)
								{
									HumSensor.offset_h = (int16)var[i].value;  
									write_eeprom(EEP_HUM_OFFSET,HumSensor.offset_h);
									write_eeprom(EEP_HUM_OFFSET + 1,HumSensor.offset_h >> 8);
								}
								else
								{
									HumSensor.offset_h_default = (int16)var[i].value;   
									write_eeprom(EEP_CAL_DEFAULT_HUM ,HumSensor.offset_h_default);
									write_eeprom(EEP_CAL_DEFAULT_HUM + 1,HumSensor.offset_h_default >> 8);
								}
//								var[i].value = value*10;
//								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break;
							case 16: //OffSet_T 
//								StartAdd = MODBUS_TEMP_OFFSET;
//								var[i].value = value*10;
//								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								var[i].value = value*10;
								HumSensor.offset_t = (int16)var[i].value;    
								write_eeprom(EEP_TEMP_OFFSET,HumSensor.offset_t);
								write_eeprom(EEP_TEMP_OFFSET + 1,HumSensor.offset_t>>8);
								break; 
							case 17: //OffSet_C   
								var[i].value = value;
 								int_co2_str.co2_offset =  (int16)value;
								write_eeprom(EEP_INT_CO2_OFFSET, (uint8)(int_co2_str.co2_offset));
								write_eeprom(EEP_INT_CO2_OFFSET + 1, (uint8)(int_co2_str.co2_offset >> 8));
								break; 
							case 18: //OffSet_Pressure 
								if((Pressure.SNR_Model == PRS_26PCGFA)||(Pressure.SNR_Model == PRS_26PCGFA))
									var[i].value = value*10;
								else
									var[i].value = value*100;
 								Pressure.org_val_offset = (int16)var[i].value ;  
								write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,Pressure.org_val_offset);
								write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,Pressure.org_val_offset >> 8);
								break;
							case 19://offset pm2.5
								var[i].value = value*10;
								pm25_sensor.pm25_offset = (int16)var[i].value;
								write_eeprom(EEP_PM25_OFFSET, pm25_sensor.pm25_offset);
								write_eeprom(EEP_PM25_OFFSET + 1, pm25_sensor.pm25_offset>>8);
								break;
							case 20://offset pm10
								var[i].value = value*10;
								pm25_sensor.pm10_offset = (int16)var[i].value;
								write_eeprom(EEP_PM10_OFFSET, pm25_sensor.pm10_offset);
								write_eeprom(EEP_PM10_OFFSET + 1, pm25_sensor.pm10_offset>>8);
								break;
							case 21: //Filter_H 
								var[i].value = value;
								HumSensor.H_Filter = (uint8)var[i].value;
								write_eeprom(EEP_HUMIDITY_FILTER,HumSensor.H_Filter); 
//								StartAdd = MODBUS_HUIDITY_FILTER;
//								var[i].value = value;
//								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break; 
							case 22: //Filter_T 
								var[i].value = value;
								HumSensor.T_Filter = (uint8)var[i].value;
								write_eeprom(EEP_EXT_TEMPERATURE_FILTER,HumSensor.T_Filter);
//								StartAdd = MODBUS_EXT_TEMPRATURE_FILTER;
//								var[i].value = value;
//								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break; 
							case 23: //Filter_C 
								var[i].value = value;
								int_co2_filter = (uint8)var[i].value;
								write_eeprom(EEP_CO2_FILTER, int_co2_filter);
//								StartAdd = MODBUS_CO2_FILTER;
//								var[i].value = value;
//								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break; 
							case 24://filter pressure
								var[i].value = value;
								Pressure.filter = (uint8)var[i].value;
								write_eeprom(EEP_PRESSURE_FILTER,Pressure.filter);
							case 25://filter pm2.5
								var[i].value = value;
								pm25_sensor.PM25_filter = (uint8)var[i].value;
								write_eeprom(EEP_PM25_FILTER,pm25_sensor.PM25_filter);
							case 26://filter pm10
								var[i].value = value;
								pm25_sensor.PM10_filter = (uint8)var[i].value;
								write_eeprom(EEP_PM10_FILTER,pm25_sensor.PM10_filter);
							case 27: //T_Unit 
								var[i].value = value;
								if(var[i].value)
									deg_c_or_f = 1;
								else
									deg_c_or_f = 0;
								write_eeprom(EEP_DEG_C_OR_F, (uint8)deg_c_or_f);
//								StartAdd = MODBUS_TEMPERATURE_DEGREE_C_OR_F;
//								var[i].value = value;
//								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break;  
							case 29: //MIN_RNG0 
								StartAdd = MODBUS_OUTPUT_RANGE_MIN_HUM;
								var[i].value = value*10;
								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break; 
							case 30: //MAX_RNG0 
								StartAdd = MODBUS_OUTPUT_RANGE_MAX_HUM;
								var[i].value = value*10;
								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break; 
							case 31: //MIN_RNG1 
								StartAdd = MODBUS_OUTPUT_RANGE_MIN_TEM;
								var[i].value = value*10;
								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break; 
							case 32: //MAX_RNG1 
								StartAdd = MODBUS_OUTPUT_RANGE_MAX_TEM;
								var[i].value = value*10;
								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break; 
							case 33: //MIN_RNG2 
								StartAdd = MODBUS_OUTPUT_RANGE_MIN_CO2;
								var[i].value = value;
								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break; 
							case 34: //MAX_RNG2 
								StartAdd = MODBUS_OUTPUT_RANGE_MAX_CO2;
								var[i].value = value;
								Data_Deal(StartAdd,(int16)var[i].value>>8,var[i]. value); 
								break;
							 
							default: break;
						} 
					}
//					 
				}
 
			 
			break;
			case AI:
				if(i < MAX_INS)
				{ 	
					s16 itemp;
					switch (i)
					{
						case 0: //temperature
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
							break;
						case 1://humidity
							inputs[i].value = value*10;
							external_operation_value =  (int16)inputs[i].value;
			
							if(output_auto_manual & 0x02)
								output_manual_value_humidity = external_operation_value;
							else if(external_operation_value < 950)  //< 95%
							{
								external_operation_flag = HUM_CALIBRATION;
								Run_Timer = 0;
							}
							break;
						case 2://CO2
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
						case 3://pressure
							
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
						break;
						
						case 4://pm2.5
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
							break;
						case 5://PM10
							inputs[i].value = value*10;
							itemp = (unsigned int)inputs[i].value;
							if(itemp > 0)
							{	
				//				itemp -= pm25_sensor.pm10;
								pm25_sensor.pm10_offset += (itemp - pm25_sensor.pm10);
								pm25_sensor.pm10 = itemp;
								write_eeprom(EEP_PM10_OFFSET, pm25_sensor.pm10_offset);
								write_eeprom(EEP_PM10_OFFSET + 1, pm25_sensor.pm10_offset>>8);
							}
						case 6://Light sensor 
							if(((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485)))
							{
								inputs[i].value = value;  
								StartAdd = MODBUS_HUM_LIGHT_VALUE; 
								Data_Deal(StartAdd,(int16)inputs[i].value>>8,inputs[i]. value); 
							}
						default:break;
					}
				}

			break;
			case BO:
			 
			break;
			case AO:
//					#ifdef T38AI8AO6DO
//					 outputs[i+MAX_DO].value= value;
//					#endif
			break;
	
			default:
			break;
		}			

}
//-------------------------------------------------
void write_bacnet_name_to_buf(uint8_t type,uint8_t priority,uint8_t i,char* str)
{
 
//		if(i == 0) return;    //start from var1.
//		else i -= 1;
		switch(type)
		{
			case AI: 
				if(i < MAX_INS)
				{
					memcpy(inputs[i].label,str,9);
					inputs[i].label[8] = 0;
					write_page_en[IN_TYPE] = 1;
				}
				break;
//			case BO:
//				memcpy(outputs[i].label,str,8);
//				break;
			case AO:
				if(i < MAX_AOS)
				{
					memcpy(outputs[i].label,str,9); 
					outputs[i].label[8] = 0;
					write_page_en[OUT_TYPE] = 1;
				} 
				break;
			case AV:
				if(i < MAX_AVS) 
				{
					memcpy(var[i].label,str,9); 
					var[i].label[8] = 0;
					 write_page_en[VAR_TYPE] = 1;
				}
				break;
	
			default:
			break;
		} 
}
//---------------------------------------------------
void write_bacnet_unit_to_buf(uint8_t type,uint8_t priority,uint8_t i,uint8_t unit)
{
//	U8_T temp;
		switch(type)
		{
			case AV:
			break ;
			case AI:
			break ;
			case AO:
			break ;
			default:
			break;
		}	
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
//	if(num == 0) return "null";    //start from var1.
//	else num -= 1;
	
	switch(type)
      {
         case AV: 
			if(num < MAX_AVS)
			{
				return (char *)var[num].label; 
			}  
            break;
         case AI:
			 if(num < MAX_INS)
			 {
				 if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
				 {
					if((num==0)||(num==1)||(num==2))
						return (char *)inputs[num].label;
					else
						return (char *)"null";
				 }
				 else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
				 {
					if(num==3)
						return (char *)inputs[num].label;
					else
						return (char *)"null";
				 }
				 else if(PRODUCT_ID == STM32_PM25) 
				 {
					if((num==4)||(num==5))
						return (char *)inputs[num].label;
					else
						return (char *)"null";
				 }
				 else if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
				 {
					if(humidity_version == LIGHT_SENSOR)
					{
						 if((num==0)||(num==1)||(num==6))
							return (char *)inputs[num].label;
					}
					else
					{
						 if((num==0)||(num==1))
							return (char *)inputs[num].label;
					}
					return (char *)"null";
				 }
			 }
            break;
         case AO: 
			 {
				 uint8 AOS_TEMP;
				 if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
						AOS_TEMP = 3;
				 else if((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485))
						AOS_TEMP = 1;
				 else if(PRODUCT_ID == STM32_PM25) 
						AOS_TEMP = 2;
				 else //if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
						AOS_TEMP = 2;
				 if(num < AOS_TEMP)//(num < MAX_AOS)
				 { 
					return (char *)outputs[num].label;  
				 }
					
			 }
            break;
          case BO:
			  
//			 if(num < BOS)
//				return (char *)DO_name[num];
			 
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
	
	switch(type)
      {
         case AV: 
			if(num < MAX_AVS) 
				return (char *)var[num].description;   
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
	switch(type)
	{
		case AV: 
			
			if(num == 14)	
				return UNITS_KILOJOULES_PER_KILOGRAM_DRY_AIR;
		break;
		
		case AI:
			if(num == 0)
			{
				if(deg_c_or_f == DEGREE_C)
					return 	UNITS_DEGREES_CELSIUS ;
				else	
					return 	UNITS_DEGREES_FAHRENHEIT ;					
			}
			else if(num == 1)
				return 	UNITS_PERCENT_RELATIVE_HUMIDITY ; 
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
	
//		if(i == 0) return ;    //start from var1.
//		else i -= 1;
	
		switch(type)
		{ 
			case AO:
				if(i < MAX_AOS)
				{
					memcpy(outputs[i].description,str,20); 
					write_page_en[OUT_TYPE] = 1;
				} 
				break;
			case AI:
				if(i < MAX_INS)
				{
					memcpy(inputs[i].description,str,20); 
					write_page_en[IN_TYPE] = 1;
				} 
				break;
			case AV:
				if(i < MAX_AVS) 
				{
					memcpy(var[i].description,str,20); 
					var[i].description[20] = 0;
					write_page_en[VAR_TYPE] = 1; 
				}
				break;
	
			default:
			break;
		} 
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
	return "temco controls";
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

