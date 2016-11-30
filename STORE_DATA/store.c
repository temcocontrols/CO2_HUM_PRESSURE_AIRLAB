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
extern void watchdog(void);
void Flash_Write_Mass(void)
{ 
	uint16_t	len = 0 ;
//	uint16_t    loop1;	 
	uint8_t     tempbuf[PAGE_LENTH]; 
//	if(write_page_en[OUT_TYPE] == 1)
//	{
//		write_page_en[OUT_TYPE] = 0 ;	
//		STMFLASH_Unlock();  //解锁	
//		STMFLASH_ErasePage(OUT_PAGE_FLAG);				
//		for(loop1 = 0;loop1 < MAX_OUTS;loop1++)
//		{
//			memcpy(&tempbuf[sizeof(Str_out_point) * loop1],&outputs[loop1],sizeof(Str_out_point));					
//		}
//		len = sizeof(Str_out_point) *MAX_OUTS ;
//		iap_write_appbin(OUT_PAGE,(uint8_t*)tempbuf, len);
////		STMFLASH_Unlock();	
//		STMFLASH_WriteHalfWord(OUT_PAGE_FLAG, 10000) ;
//		STMFLASH_Lock();	
//	} 
	
//	if(write_page_en[IN_TYPE] == 1)
//	{
//		write_page_en[IN_TYPE] = 0 ;
//		STMFLASH_Unlock();	
//		STMFLASH_ErasePage(IN_PAGE_FLAG);
//		for(loop1 = 0;loop1 < MAX_INS;loop1++)
//		{
//			memcpy(&tempbuf[sizeof(Str_in_point) * loop1],&inputs[loop1],sizeof(Str_in_point));					
//		}
//		len = sizeof(Str_in_point)*MAX_INS ;
//		iap_write_appbin(IN_PAGE,(uint8_t*)tempbuf, len); 
//		STMFLASH_WriteHalfWord(IN_PAGE_FLAG, 10000) ;
//		STMFLASH_Lock();	
//	} 
	
	if(write_page_en[VAR_TYPE] == 1)
	{  
			
		STMFLASH_Unlock();
		STMFLASH_ErasePage(AV_PAGE_FLAG);
		STMFLASH_ErasePage(AV_PAGE_FLAG + 2048);
		 
		len = MAX_AVS * sizeof(Str_variable_point) ;
		test[0] = len; 
		memcpy(tempbuf,(void*)&var[0].description[0],len); 
		iap_write_appbin(AV_PAGE,(uint8_t*)tempbuf, len); 
		STMFLASH_WriteHalfWord(AV_PAGE_FLAG, 10000) ;	
		STMFLASH_Lock();
		write_page_en[VAR_TYPE] = 0 ; 
	}				
		 
}
const uint8 Var_label[MAX_AVS][9] = {
	
	"SN_L",   //0
	"SN_H",   //1
	"SW_Ver", //2
	"Address",//3
	"Model",  //4
	"HW_Ver", //5
	"Hum_Ver",//6
	"BaudRate",//7
	"Update",  //8
	"Protocol",//9
	"AM_Mode", //10
	"Humidity",//11
	"Tempera", //12
	"CO2",     //13
	"dew_pt",  //14
	"Pws",     //15
	"MixRatio",//16
	"Enthalpy",//17  
	"OffSet_H",//18
	"OffSet_T",//19 
	"OffSet_C",//20 
	"Filter_H",//21
	"Filter_T",//22
	"Filter_C",//23 
	"T_Unit",  //24 
	"OutMode", //25
	"Output0", //26
	"Output1", //27
	"Output2", //28
	"MIN_RNG0",//29 
	"MAX_RNG0",//30
	"MIN_RNG1",//31
	"MAX_RNG1",//32
	"MIN_RNG2",//33
	"MAX_RNG2",//34 
	"PID_SEL", //35
	"PID1_MODE",//36
	"PID1_SP",  //37 
	"PID1_P",   //38
	"PID1_I",   //39
	"PID1_V",   //40
	"PID2_MODE",//41 
	"PID2_SP",  //42 
	"PID2_P",   //43
	"PID2_I",   //44
	"PID2_V",   //45
	"PID3_MODE",//46 
	"PID3_SP",  //47 
	"PID3_P",   //48
	"PID3_I",   //49
	"PID3_V",   //50
	"AlarmAM",  //51
	"Alarm",    //52
	"PreAlarm", //53
	"Time_On",  //54
	"Time_Off", //55 
};
const uint8 Var_Description[MAX_AVS][21] = {
	
	"SerialNumberLowByte",   	//0
	"SerialNumberHighByte",   	//1
	"SoftWare Version", 		//2
	"ID Address",				//3
	"Product Model",			//4
	"HardWare Version",			//5
	"Humdity Version",			//6
	"Uart BaudRate",			//7
	"Update", 					//8
	"Protocol",					//9
	"Auto/Manual",				//10
	"Humidity",					//11
	"Tempera", 					//12
	"CO2",						//13
	"dew point",				//14
	"Pws",						//15
	"MixRatio",					//16
	"Enthalpy",  				//17
	"OffSet Humdity",			//18
	"OffSet Tempeature", 		//19
	"OffSet Co2",				//20 
	"Filter Humdity",			//21
	"Filter Temperature",		//22
	"Filter Co2",				//23 
	"Temperature Unit",			//24 
	"OutMode",					//25
	"Humdity",					//26
	"Temperature",				//27
	"Co2",						//28
	"Humdity Min Range",		//29
	"Humdity Max Range",		//30
	"Temperatur Min Range",		//31
	"Temperatur Max Range",		//32
	"Co2 Min Range",			//33
	"Co2 Max Range", 			//34
	"PID/Transmit select",		//35
	"PID1 MODE",				//36
	"PID1 SetPoint",			//37 
	"PID1 Pterm",				//38
	"PID1 Iterm",				//39
	"PID1 Value", 				//40
	"PID2 MODE",				//41
	"PID2 SetPoint",			//42 
	"PID2 Pterm",				//43
	"PID2 Iterm",				//44
	"PID2 Value",				//45
	"PID3 MODE",				//46
	"PID3 SetPoint",			//47 
	"PID3 Pterm",				//48
	"PID3 Iterm",				//49
	"PID3 Value",				//50
	"Alarm Auto/Manual",		//51	
	"Alarm Setpoint",			//52
	"PreAlarm Setpoint",		//53
	"Alarm Time On",			//54
	"Alarm Time Off", 			//55
};

const uint8 Outputs_label[MAX_AOS][9] = {
 	"Output0",
//	"Output1",
//	"Output2",
};
const uint8 Outputs_Description[MAX_AOS][21] = {
 	
 	"Tempreture",
//	"CO2/PRESSURE",
//	"Humidity"
};

//void mass_flash_init(void)
//{  
//	uint8 i;
//	uint16 pos;
//	write_page_en = 0; 
//    
//	if(AT24CXX_ReadOneByte(EEP_FACTORY_SET) == 0x55) 
//	{
//		pos = 0;
//		for(i = 0;i < MAX_VARS; i++) 
//		{	 
//			STMFLASH_Read_Page(PAGE127, tempbuf);
//			memcpy(var[i].description,&tempbuf[pos],21); 
//			pos += 21;
//			memcpy(var[i].label,&tempbuf[pos],9); 
//			pos += 9;
//		}
//		for(i = 0;i < MAX_AOS; i++) 
//		{
//			memcpy(outputs[i].description,&tempbuf[pos],21); 
//			pos += 21;
//			memcpy(outputs[i].label,&tempbuf[pos],9); 
//			pos += 9;			
//		}
//	}
//	else 
//	{
//		AT24CXX_WriteOneByte(EEP_FACTORY_SET,0x55);
//		for(i = 0;i < MAX_VARS; i++) 
//		{	 
//			memcpy(var[i].description,Var_Description[i],21);  
//			memcpy(var[i].label,Var_label[i],9); 
//			 
//		}
//		for(i = 0;i < MAX_AOS; i++) 
//		{
//			memcpy(outputs[i].description,Outputs_Description[i],21);  
//			memcpy(outputs[i].label,Outputs_label[i],9);   		
//		}
//		
//		Flash_Write_Mass();
//	}	 
//}

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
 
// 	temp = STMFLASH_ReadHalfWord(OUT_PAGE_FLAG);
////	printf("temp=%x, %x\n\r", temp2, temp);
//	if(temp == 0xffff)
//	{
//		STMFLASH_Unlock();
//		STMFLASH_ErasePage(OUT_PAGE_FLAG);
//		for(loop=0; loop<MAX_OUTS; loop++ )
//		{
//			memcpy(outputs[loop].description,Outputs_Description[loop],21);  
//			memcpy(outputs[loop].label,Outputs_label[loop],9);   	
////			memset(outputs[loop].description, 1, 21) ;
////			memset(outputs[loop].label, 0, 9) ;			
//			outputs[loop].value = 0; 
//			outputs[loop].auto_manual = 0 ;
//			outputs[loop].digital_analog = 0 ;
//			outputs[loop].switch_status = 0 ;
//			outputs[loop].control = 0 ;
//			outputs[loop].read_remote = 0 ;
//			outputs[loop].decom = 0 ;
//			outputs[loop].range = 0 ;
//			outputs[loop].sub_id = 0 ;
//			outputs[loop].sub_product = 0 ;
//			outputs[loop].pwm_period = 0 ;
//		}
//		len = MAX_OUTS * sizeof(Str_out_point) ;
//		if(len > PAGE_LENTH)
//		{
//			memcpy(tempbuf,(void *)&outputs[0].description[0],PAGE_LENTH); 
//			memcpy(tempbuf,(void *)(&outputs[0].description[0] + PAGE_LENTH),len - PAGE_LENTH);
//		}
//		else
//			memcpy(tempbuf,(void *)&outputs[0].description[0],len);  
//		iap_write_appbin(OUT_PAGE,(uint8_t*)tempbuf, len);	 
//		STMFLASH_WriteHalfWord(OUT_PAGE_FLAG, 10000) ;
//		STMFLASH_Lock();	
//	}
//	else
//	{
//		len = MAX_OUTS * sizeof(Str_out_point) ;
//		STMFLASH_MUL_Read(OUT_PAGE,(void *)&outputs[0].description[0], len );	
//	}
	
	temp = STMFLASH_ReadHalfWord(AV_PAGE_FLAG); 
 // 	temp =0;// 0xffff;
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
			var[loop].digital_analog = 0 ;
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
	
//	temp = STMFLASH_ReadHalfWord(IN_PAGE_FLAG);
//	if(temp == 0xffff)
//	{
//		STMFLASH_ErasePage(IN_PAGE_FLAG);
//		for(loop=0; loop<MAX_INS; loop++ )
//		{
//			sprintf((char*)label_buf, "input%u", loop);
//			memcpy(inputs[loop].description, label_buf, 21);
//			memcpy(inputs[loop].label, label_buf, 9);
//			
//			inputs[loop].value = 0; 
//			inputs[loop].filter = 5 ;
//			inputs[loop].decom = 0 ;
//			inputs[loop].sub_id = 0 ;
//			inputs[loop].sub_product = 0 ;
//			inputs[loop].control = 0 ;
//			inputs[loop].auto_manual = 0 ;
//			inputs[loop].digital_analog = 0 ;
//			inputs[loop].calibration_sign = 0 ;
//			inputs[loop].sub_number = 0 ;
//			inputs[loop].calibration_hi = (500>>8)&0xff ;
//			inputs[loop].calibration_lo = 500 &0xff ;
//			inputs[loop].range = 0 ; 
//		}
//		len = MAX_INS * sizeof(Str_in_point) ;
//		memcpy(tempbuf,(void*)&inputs[0], len);		
//		iap_write_appbin(IN_PAGE,(uint8_t*)tempbuf, len);	
//		STMFLASH_WriteHalfWord(IN_PAGE_FLAG, 10000) ;
//	}
//	else
//	{
//		len = MAX_INS * sizeof(Str_in_point) ;
//		STMFLASH_MUL_Read(IN_PAGE,(void *)&inputs[0].description[0], len ); 
//	}

}

