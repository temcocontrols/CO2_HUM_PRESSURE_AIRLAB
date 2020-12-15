//#include "stdlib.h"
//#include "crc.h"
//#include "usart.h"
//#include "24cxx.h" 
//#include "delay.h"
//#include "define.h"
//#include "modbus.h"
//#include "registerlist.h"
//#include "inputs.h"
//#include "../filter/filter.h"
//#include "led.h"
//#include "fifo.h"
////#include "bacnettimer.h"
//#include "../KEY/key.h" 
//#include "tcp_modbus.h"
//#include "tapdev.h"
//#include "bacnet.h"
//#include "rs485.h"
//#include "store.h"
////#include "ud_str.h"

#include "config.h"
#include "stmflash.h"
#include "sht3x.h"

extern uint8 update_flag;
uint8 mhz19_cal_h = 0;
uint8 mhz19_cal_l = 0;
//extern U8_T cal_co2_MHZ19B_span[9];

uint8 rx_icon = 0;
uint8 tx_icon = 0;

void co2_reset(void);

void Timer_Silence_Reset(void);
static u8 randval = 0 ;
//u8 i2c_test[10] ;
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 uart_send[USART_SEND_LEN] ;
vu8 transmit_finished = 0 ; 
vu8 revce_count = 0 ;
vu8 rece_size = 0 ;
vu8 serial_receive_timeout_count ;
u8 SERIAL_RECEIVE_TIMEOUT ;
u8 dealwithTag ;
STR_MODBUS modbus ;
u8 DealwithTag ;
u16 sendbyte_num = 0 ;
u16 uart_num = 0 ;
 u8  	Station_NUM= 12;
//extern uint8_t Receive_Buffer_Data0[512];
extern FIFO_BUFFER Receive_Buffer0;
u8 dis_hum_info = 0; 

u8 reply_delay_time;
u8 receive_delay_time;

u8 reply_done;
u8 uart1_parity;
STR_UART uart;

//extern uint16 lastCO2;
//extern int16 lastTemp, lastHumi;
bool isFirstLineChange = false; 
bool isSecondLineChange = false; 
bool isThirdLineChange = false;
uint16_t co2_frc = 0;

extern uint8 sensirion_co2_cmd_ForcedCalibration[8];

extern void watchdog(void);
void USART1_IRQHandler(void)                	//串口1中断服务程序
{		
	u8 receive_buf ;
	u8 i;
	unsigned portBASE_TYPE uxSavedInterruptStatus;
	
	static u16 send_count = 0;
	
	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)	//接收中断
	{
		  rx_icon = 2;
			if(modbus.protocal == MODBUS)	
			{
					if(revce_count < USART_REC_LEN)
						USART_RX_BUF[revce_count++] = USART_ReceiveData(USART1);//(USART1->DR);		//读取接收到的数据
						else
							 serial_restart();
						if(revce_count == 1)
						{
							// This starts a timer that will reset communication.  If you do not
							// receive the full packet, it insures that the next receive will be fresh.
							// The timeout is roughly 7.5ms.  (3 ticks of the hearbeat)
							rece_size = 250;
							serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;
						}
						else if(revce_count == 3 )
						{
							if(USART_RX_BUF[1] == CHECKONLINE)
							rece_size = 6;
						}
						else if(revce_count == 4)
						{
							//check if it is a scan command
							if((((vu16)(USART_RX_BUF[2] << 8) + USART_RX_BUF[3]) == 0x0a) && (USART_RX_BUF[1] == WRITE_VARIABLES))
							{
								rece_size = DATABUFLEN_SCAN;
								serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;	
							}
						}
						else if(revce_count == 7)
						{
							if((USART_RX_BUF[1] == READ_VARIABLES) || (USART_RX_BUF[1] == WRITE_VARIABLES))
							{
								rece_size = 8;
								//dealwithTag = 1;
							}
							else if(USART_RX_BUF[1] == MULTIPLE_WRITE)
							{
								rece_size = USART_RX_BUF[6] + 9;
								serial_receive_timeout_count = USART_RX_BUF[6] + 8;
							}
							else
							{
								rece_size = 250;
							}
						}
//						else if(USART_RX_BUF[0] == 0x55 && USART_RX_BUF[1] == 0xff && USART_RX_BUF[2] == 0x01 && USART_RX_BUF[5] == 0x00 && USART_RX_BUF[6] == 0x00)
//						{//bacnet protocal detected
//								
//							
////								if(modbus.protocal != BAC_MSTP)
////									modbus.protocal_timer_enable  = 1;
//								
////								iap_load_app(FLASH_APP1_ADDR);
////								iap_load_app(STM32_FLASH_BASE);
////								Recievebuf_Initialize(0);							
//						}
						else if(revce_count == rece_size)		
						{
							// full packet received - turn off serial timeout
							if((USART_RX_BUF[0] == 0xff) || (USART_RX_BUF[0] == modbus.address))
							{
							serial_receive_timeout_count = 0;
							dealwithTag = 2;		// making this number big to increase delay
							rx_count = 2 ;
							uart.rx_count++;
							
//							for(i=0;i<rece_size;i++)
//							{
//								ctest[10+i] = USART_RX_BUF[i];
//							}								
								
							if(uart.rx_count > 99) uart.rx_count = 0;
							}
							else
								serial_restart();
						}
		
			}
			else if(modbus.protocal == BAC_MSTP )
			{
					if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
					{
							receive_buf =  USART_ReceiveData(USART1); 
							FIFO_Put(&Receive_Buffer0, receive_buf);
						  //rx_icon = 2;
					}
			}
	}
//	else if( USART_GetITStatus(USART1, USART_IT_TC) == SET  )
//     {
//         if( uart_num >=sendbyte_num)
//		 {
//             USART_ClearFlag(USART1, USART_FLAG_TC);
//			 uart_num = 0 ;
//		 }
//         else
//             USART_SendData(USART1, pDataByte[uart_num++]);
//	}
	else  if( USART_GetITStatus(USART1, USART_IT_TXE) == SET  )
     {
        if((modbus.protocal == MODBUS )||(modbus.protocal == BAC_MSTP))
				{
					 USART_SendData(USART1, uart_send[send_count++] );  
					 Timer_Silence_Reset();
					 if( send_count >= sendbyte_num)
					 {
								tx_icon = 2;
								while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
								USART_ClearFlag(USART1, USART_FLAG_TC); 
								USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
								send_count = 0 ; 
								if(modbus.protocal == MODBUS)
								{
									reply_done = receive_delay_time;
									if(reply_done == 0) serial_restart();
								}
								else
									serial_restart(); 	
					 }  				
		   }
     }
	 
	 portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}

void serial_restart(void)
{
	TXEN = RECEIVE;
	revce_count = 0;
	dealwithTag = 0;
} 

//it is ready to send data by serial port . 
static void initSend_COM(void)
{
	TXEN = SEND;
}

void send_byte(u8 ch, u8 crc)
{	
	USART_ClearFlag(USART1, USART_FLAG_TC); 
	USART_SendData(USART1,  ch);
	tx_count = 2 ;
	if(crc)
	{
		crc16_byte(ch);
	}
}
//void USART_SendDataString(u8 *pbuf, u8 num)
// {
//     sendbyte_num = num ;
//	 uart_num = 0 ;
//	 pDataByte = pbuf ;
//	 USART_ClearFlag(USART1, USART_FLAG_TC);   
//     USART_SendData(USART1, pDataByte[uart_num++] ); 
//	 tx_count = 20 ;
// }
 void USART_SendDataString( u16 num )
 {
	 tx_count = 2 ;
	 sendbyte_num = num;
	 uart_num = 0 ;
	 if(modbus.protocal == MODBUS)
		delay_ms(reply_delay_time);
	 USART_ITConfig(USART1, USART_IT_TXE, ENABLE);//
 }
void modbus_init(void)
{
	//uart1_init(19200);
	serial_restart();
	SERIAL_RECEIVE_TIMEOUT = 3;
	serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;
	reply_delay_time = read_eeprom(EEP_REPLY_DELAY);
	if(reply_delay_time > 100) reply_delay_time = 10;
	receive_delay_time = read_eeprom(EEP_RECEIVE_DELAY);
	if(receive_delay_time > 100) receive_delay_time = 10; 
	
	uart.rx_count = 0;
	uart.tx_count = 0;
}
void write_user_data_by_block(U16_T StartAdd,U8_T HeadLen,U8_T *pData) 
{
	U8_T far i;

	if(StartAdd  >= MODBUS_OUTPUT_BLOCK_FIRST && StartAdd  <= MODBUS_OUTPUT_BLOCK_LAST)
	{
		if((StartAdd - MODBUS_OUTPUT_BLOCK_FIRST) % ((sizeof(Str_out_point) + 1) / 2) == 0)
		{
			i = (StartAdd - MODBUS_OUTPUT_BLOCK_FIRST) / ((sizeof(Str_out_point) + 1) / 2);
			memcpy(&outputs[i],&pData[HeadLen + 7],sizeof(Str_out_point));
		}
	}
	else if(StartAdd  >= MODBUS_INPUT_BLOCK_FIRST && StartAdd  <= MODBUS_INPUT_BLOCK_LAST)
	{
		if((StartAdd - MODBUS_INPUT_BLOCK_FIRST) % ((sizeof(Str_in_point) + 1	) / 2) == 0)
		{
			i = (StartAdd - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
			memcpy(&inputs[i],&pData[HeadLen + 7],sizeof(Str_in_point)); 
			IO_Change_Flag[IN_TYPE] = 1;
		}
	}
	else if(StartAdd  >= MODBUS_VAR_BLOCK_FIRST && StartAdd  <= MODBUS_VAR_BLOCK_LAST)
	{
		if((StartAdd - MODBUS_VAR_BLOCK_FIRST) % ((sizeof(Str_variable_point) + 1	) / 2) == 0)
		{
			i = (StartAdd - MODBUS_VAR_BLOCK_FIRST) / ((sizeof(Str_variable_point) + 1) / 2);
			memcpy(&var[i],&pData[HeadLen + 7],sizeof(Str_variable_point)); 	
		}
	}
//	else if(StartAdd  >= MODBUS_PRG_BLOCK_FIRST && StartAdd  <= MODBUS_PRG_BLOCK_LAST)
//	{
//		if((StartAdd - MODBUS_PRG_BLOCK_FIRST) % ((sizeof(Str_program_point) + 1	) / 2) == 0)
//		{
//			i = (StartAdd - MODBUS_PRG_BLOCK_FIRST) / ((sizeof(Str_program_point) + 1) / 2);
//			memcpy(&programs[i],&pData[HeadLen + 7],sizeof(Str_program_point)); 	
//		}
//	}
//	else if(StartAdd  >= MODBUS_WR_BLOCK_FIRST && StartAdd  <= MODBUS_WR_BLOCK_LAST)
//	{
//		if((StartAdd - MODBUS_WR_BLOCK_FIRST) % ((sizeof(Str_weekly_routine_point) + 1	) / 2) == 0)
//		{
//			i = (StartAdd - MODBUS_VAR_BLOCK_FIRST) / ((sizeof(Str_weekly_routine_point) + 1) / 2);
//			memcpy(&weekly_routines[i],&pData[HeadLen + 7],sizeof(Str_weekly_routine_point)); 	
//		}
//	}
//	else if(StartAdd  >= MODBUS_AR_BLOCK_FIRST && StartAdd  <= MODBUS_AR_BLOCK_LAST)
//	{
//		if((StartAdd - MODBUS_AR_BLOCK_FIRST) % ((sizeof(Str_annual_routine_point) + 1	) / 2) == 0)
//		{
//			i = (StartAdd - MODBUS_AR_BLOCK_FIRST) / ((sizeof(Str_annual_routine_point) + 1) / 2);
//			memcpy(&annual_routines[i],&pData[HeadLen + 7],sizeof(Str_annual_routine_point)); 	
//		}
//	}
//	else if(StartAdd  >= MODBUS_WR_TIME_FIRST && StartAdd  <= MODBUS_WR_TIME_LAST)
//	{
//		if((StartAdd - MODBUS_WR_TIME_FIRST) % (sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK / 2) == 0)
//		{		
//			i = (StartAdd - MODBUS_WR_TIME_FIRST) / (sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK / 2);
//			memcpy(&wr_times[i],&pData[HeadLen + 7],(sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK)); 	
//		}
//	}
//	else if(StartAdd  >= MODBUS_AR_TIME_FIRST && StartAdd  <= MODBUS_AR_TIME_LAST)
//	{
//		if((StartAdd - MODBUS_AR_TIME_FIRST) % AR_DATES_SIZE == 0)
//		{
//			i = ((StartAdd - MODBUS_AR_TIME_FIRST) / AR_DATES_SIZE);
//			memcpy(&ar_dates[i],&pData[HeadLen + 7],AR_DATES_SIZE); 	
//		}
//	}	
	else  if(StartAdd  >= MODBUS_CONTROLLER_BLOCK_FIRST && StartAdd  <= MODBUS_CONTROLLER_BLOCK_LAST)
	{ 
		if((StartAdd - MODBUS_CONTROLLER_BLOCK_FIRST) % ((sizeof(Str_controller_point) + 1	) / 2) == 0)
		{
			i = (StartAdd - MODBUS_CONTROLLER_BLOCK_FIRST) / ((sizeof(Str_controller_point) + 1) / 2);
			memcpy(&controllers[i],&pData[HeadLen + 7],sizeof(Str_controller_point));
			PID[i].Set_Flag = 1;			
		}
	}
}



U16_T read_user_data_by_block(U16_T addr) 
{
	U8_T far index,item;	
	U16_T far *block;			
	if( addr >= MODBUS_OUTPUT_BLOCK_FIRST && addr <= MODBUS_OUTPUT_BLOCK_LAST )
	{
		index = (addr - MODBUS_OUTPUT_BLOCK_FIRST) / ( (sizeof(Str_out_point) + 1) / 2);
		block = (U16_T *)&outputs[index];
		item = (addr - MODBUS_OUTPUT_BLOCK_FIRST) % ((sizeof(Str_out_point) + 1) / 2);
	}
	else if( addr >= MODBUS_INPUT_BLOCK_FIRST && addr <= MODBUS_INPUT_BLOCK_LAST )
	{
		index = (addr - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
		block = (U16_T *) &inputs[index];
		item = (addr - MODBUS_INPUT_BLOCK_FIRST) % ((sizeof(Str_in_point) + 1) / 2);
	}
	else if( addr >= MODBUS_VAR_BLOCK_FIRST && addr <= MODBUS_VAR_BLOCK_LAST )
	{
		index = (addr - MODBUS_VAR_BLOCK_FIRST) / ((sizeof(Str_variable_point) + 1) / 2);
		block = (U16_T *)&inputs[index];
		item = (addr - MODBUS_VAR_BLOCK_FIRST) % ((sizeof(Str_variable_point) + 1) / 2);
	}
//	else if( addr >= MODBUS_PRG_BLOCK_FIRST && addr <= MODBUS_PRG_BLOCK_LAST )
//	{

//		index = (addr - MODBUS_PRG_BLOCK_FIRST) / ((sizeof(Str_program_point) + 1) / 2);
//		block = &programs[index];
//		item = (addr - MODBUS_PRG_BLOCK_FIRST) % ((sizeof(Str_program_point) + 1) / 2);
//	}
//	else if( addr >= MODBUS_CODE_BLOCK_FIRST && addr <= MODBUS_CODE_BLOCK_LAST )
//	{	
//		index = (addr - MODBUS_CODE_BLOCK_FIRST) / 100;
//		block = &prg_code[index / (CODE_ELEMENT * MAX_CODE / 200)][CODE_ELEMENT * MAX_CODE % 200];
//		item = (addr - MODBUS_CODE_BLOCK_FIRST) % 100;
//	}
//	else if( addr >= MODBUS_WR_BLOCK_FIRST && addr <= MODBUS_WR_BLOCK_LAST )
//	{
//		index = (addr - MODBUS_WR_BLOCK_FIRST) / ((sizeof(Str_weekly_routine_point) + 1) / 2);
//		block = &weekly_routines[index];
//		item = (addr - MODBUS_WR_BLOCK_FIRST) % ((sizeof(Str_weekly_routine_point) + 1) / 2);
//	}
//	else if( addr >= MODBUS_WR_TIME_FIRST && addr <= MODBUS_WR_TIME_LAST )
//	{
//		index = (addr - MODBUS_WR_TIME_FIRST) / ((sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK + 1) / 2);
//		block = &wr_times[index];
//		item = (addr - MODBUS_WR_TIME_FIRST) % ((sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK + 1) / 2);
//	}
//	else if( addr >= MODBUS_AR_BLOCK_FIRST && addr <= MODBUS_AR_BLOCK_LAST )
//	{
//		index = (addr - MODBUS_AR_BLOCK_FIRST) / ((sizeof(Str_annual_routine_point) + 1) / 2);
//		block = &annual_routines[index];
//		item = (addr - MODBUS_AR_BLOCK_FIRST) % ((sizeof(Str_annual_routine_point) + 1) / 2);
//		
//	}
//	else if( addr >= MODBUS_AR_TIME_FIRST && addr <= MODBUS_AR_TIME_LAST )
//	{
//	
//		index = (addr - MODBUS_AR_TIME_FIRST) / (AR_DATES_SIZE / 2);
//		block = &ar_dates[index];
//		item = (addr - MODBUS_AR_TIME_FIRST) % (AR_DATES_SIZE / 2);
//	}
	else if( addr >= MODBUS_CONTROLLER_BLOCK_FIRST && addr <= MODBUS_CONTROLLER_BLOCK_LAST )
	{
		index = (addr - MODBUS_CONTROLLER_BLOCK_FIRST) / ((sizeof(Str_controller_point) + 1) / 2);
		block = (U16_T *)&controllers[index];
		item = (addr - MODBUS_CONTROLLER_BLOCK_FIRST) % ((sizeof(Str_controller_point) + 1) / 2);
	 
	}
	return block[item];
	
} 

void internalDeal(u8 type,  u8 *pData)
{

//	u8 address_temp ;
//	u8 div_buf ;
//	u8 address_buf ;
	u8 i ;
	u8 HeadLen ;
	u16 StartAdd ;
	
	if(type == 0 || type == 4)
	{
		HeadLen = 0 ;	
	}
	else
	{
		HeadLen = 6 ;

	}
	StartAdd = (u16)(pData[HeadLen + 2] <<8 ) + pData[HeadLen + 3];
	 if (pData[HeadLen + 1] == MULTIPLE_WRITE) //multi_write
	{
		if(StartAdd == MODBUS_MAC_ADDRESS_1)
		{
			if((modbus.mac_enable == 1) && (pData[HeadLen + 6] == 12))
			{
				modbus.mac_addr[0] = pData[HeadLen + 8];
				modbus.mac_addr[1] = pData[HeadLen + 10];
				modbus.mac_addr[2] = pData[HeadLen + 12];
				modbus.mac_addr[3] = pData[HeadLen + 14];
				modbus.mac_addr[4] = pData[HeadLen + 16];
				modbus.mac_addr[5] = pData[HeadLen + 18];
				for(i=0; i<6; i++)
						{
							AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_1+i, modbus.mac_addr[i]);
						}
//				while(tapdev_init())	//初始化ENC28J60错误
//				{								   
//				//	printf("ENC28J60 Init Error!\r\n");
//				delay_ms(50);
//				};
				IP_Change = 1 ;
				modbus.mac_enable = 0 ;
			}
		} 	
		else if(StartAdd  >= MODBUS_USER_BLOCK_FIRST && StartAdd  <= MODBUS_USER_BLOCK_LAST)
		{  
			write_user_data_by_block(StartAdd,HeadLen,pData);
		}
		if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485) || (PRODUCT_ID == STM32_PM25) || (PRODUCT_ID == STM32_CO2_NET) || (PRODUCT_ID == STM32_CO2_RS485))
		{
			if(StartAdd  >= TSTAT_NAME1 && StartAdd <= TSTAT_NAME8)
			{
				if(pData[HeadLen +6] <= 20)
				{
					for(i=0;i<pData[HeadLen + 6];i++)			//	(data_buffer[6]*2)
					{
						write_eeprom((EEP_TSTAT_NAME1 + i),pData[HeadLen + 7+i]);
						panelname[i] = pData[HeadLen + 7+i];
					}
				}
			}
		}
		if(PRODUCT_ID == STM32_PRESSURE_NET || PRODUCT_ID == STM32_PRESSURE_RS485)
		{
			if(StartAdd  >= PRESSURE_TSTAT_NAME1 && StartAdd <= PRESSURE_TSTAT_NAME8)
			{
				if(pData[HeadLen +6] <= 20)
				{
					for(i=0;i<pData[HeadLen + 6];i++)			//	(data_buffer[6]*2)
					{
						write_eeprom((EEP_TSTAT_NAME1 + i),pData[HeadLen + 7+i]);
						panelname[i] = pData[HeadLen + 7+i];
					}
				}
			}
		}		
		
	}
 	else if(pData[HeadLen + 1] == WRITE_VARIABLES)
	{
		 
		 Data_Deal(StartAdd,pData[HeadLen+4],pData[HeadLen+5]);
	}
 		
		
}

void Data_Deal(u16 StartAdd,u8 Data_H,u8 Data_L)	
{
	u8 address_temp ;
	uint8 i;
	uint8 check_sum = 0;
	if(StartAdd  < ORIGINALADDRESSVALUE )
	{								
		// If writing to Serial number Low word, set the Serial number Low flag
		if(StartAdd <= MODBUS_SERIALNUMBER_LOWORD+1)
		{
			AT24CXX_WriteOneByte((u16)EEP_SERIALNUMBER_LOWORD, Data_L);
			AT24CXX_WriteOneByte((u16)EEP_SERIALNUMBER_LOWORD+1, Data_H);
			modbus.serial_Num[0] =Data_L ;
			modbus.serial_Num[1] = Data_H ;
			modbus.SNWriteflag |= 0x01;
			AT24CXX_WriteOneByte((u16)EEP_SERIALNUMBER_WRITE_FLAG, modbus.SNWriteflag);
			
			if(modbus.SNWriteflag)
			{
				modbus.update = 0;
				AT24CXX_WriteOneByte((u16)EEP_UPDATE_STATUS, 0);
			}
		}
		// If writing to Serial number High word, set the Serial number High flag
		else if(StartAdd <= MODBUS_SERIALNUMBER_HIWORD+1)
		{
			
			AT24CXX_WriteOneByte((u16)EEP_SERIALNUMBER_HIWORD, Data_L);
			AT24CXX_WriteOneByte((u16)EEP_SERIALNUMBER_HIWORD+1, Data_H);
			modbus.serial_Num[2] = Data_L ;
			modbus.serial_Num[3] = Data_H ;
			modbus.SNWriteflag |= 0x02;
			AT24CXX_WriteOneByte((u16)EEP_SERIALNUMBER_WRITE_FLAG, modbus.SNWriteflag);
			
			if(modbus.SNWriteflag)
			{
				modbus.update = 0;
				AT24CXX_WriteOneByte((u16)EEP_UPDATE_STATUS, 0);
			}
		}
//			else if(USART_RX_BUF[3] <= MODBUS_VERSION_NUMBER_LO+1)
//			{	
//				AT24CXX_WriteOneByte((u16)EEP_VERSION_NUMBER_LO, USART_RX_BUF[5]);
//				AT24CXX_WriteOneByte((u16)EEP_VERSION_NUMBER_LO+1, USART_RX_BUF[4]);
//				modbus.software = (USART_RX_BUF[5]<<8) ;
//				modbus.software |= USART_RX_BUF[4] ;				
//			}
		else if(StartAdd == MODBUS_ADDRESS )
		{
			 if(Data_L != check_master_id_in_database(Data_L-1, 0))
			{				 
				modbus.address = Data_L;
				AT24CXX_WriteOneByte(EEP_ADDRESS, modbus.address);
				scan_db[0].id = modbus.address; // update the scan talbe
				Station_NUM = modbus.address;
				//Inital_Bacnet_Server();
				dlmstp_init(NULL);
			}
		}
		else if(StartAdd == MODBUS_PRODUCT_MODEL )
		{
			AT24CXX_WriteOneByte((u16)EEP_PRODUCT_MODEL, Data_L);
			modbus.product	= Data_L ;
			PRODUCT_ID = Data_L;
			modbus.SNWriteflag |= 0x08;
			AT24CXX_WriteOneByte((u16)EEP_SERIALNUMBER_WRITE_FLAG, modbus.SNWriteflag);
		}
		else if(StartAdd == MODBUS_HARDWARE_REV )
		{
			AT24CXX_WriteOneByte((u16)EEP_HARDWARE_REV, Data_L);
			modbus.hardware_Rev	= Data_L ;
			modbus.SNWriteflag |= 0x04;
			AT24CXX_WriteOneByte((u16)EEP_SERIALNUMBER_WRITE_FLAG, modbus.SNWriteflag);
		} 
		else if(StartAdd == MODBUS_BAUDRATE )
		{			
			modbus.baud = Data_L ;
			switch(modbus.baud)
			{
				case 0:
					modbus.baudrate = BAUDRATE_9600 ;
					uart1_init(BAUDRATE_9600);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);					
					SERIAL_RECEIVE_TIMEOUT = 6;
				break ;
				case 1:
					modbus.baudrate = BAUDRATE_19200 ;
					uart1_init(BAUDRATE_19200);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 3;
				break;
				case 2:
					modbus.baudrate = BAUDRATE_38400 ;
					uart1_init(BAUDRATE_38400);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 2;
				break;
				case 3:
					modbus.baudrate = BAUDRATE_57600 ;
					uart1_init(BAUDRATE_57600);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 1;
				break;
				case 5:
					modbus.baudrate = BAUDRATE_76800 ;
					uart1_init(BAUDRATE_76800);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 1;
				break;
				case 4:
					modbus.baudrate = BAUDRATE_115200 ;
					uart1_init(BAUDRATE_115200);
					AT24CXX_WriteOneByte(EEP_BAUDRATE,Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 1;		
				break ;
				default:
				break ;				
			}
			modbus_init();
		}
		else if(StartAdd == MODBUS_UPDATE_STATUS )			// july 21 Ron
		{
			//AT24CXX_WriteOneByte(EEP_UPDATE_STATUS, pData[HeadLen+5]);
			modbus.update = Data_L ;
			if (modbus.update == 0x7F)
			{
				Lcd_Full_Screen(0);
				Lcd_Show_String(1, 6, 0, (uint8 *)"Updating...");
				Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
				SoftReset();		
			}
			else if(Data_L == 99)
			{
				write_eeprom(EEP_CLEAR_EEP, 99);
				Lcd_Full_Screen(0);
				Lcd_Show_String(1, 6, 0, (uint8 *)"eeprom ini...");
				Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
				SoftReset();
				
			}
			else if((modbus.update == 0x8e)||(modbus.update == 0x8f))
			{ 
				
				if(modbus.update == 0x8e)
				{
					modbus.SNWriteflag = 0x00;
					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_WRITE_FLAG, 0);
				} 
				{
					u8 i;
					write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,0);
					write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,0); 
					write_eeprom(EEP_PRESSURE_FILTER,DEFAULT_FILTER);   
					write_eeprom(EEP_CO2_FILTER,DEFAULT_FILTER);   
					int_co2_str.alarm_setpoint = 1000;
					write_eeprom(EEP_INT_ALARM_SETPOINT, int_co2_str.alarm_setpoint);
					write_eeprom(EEP_INT_ALARM_SETPOINT + 1, int_co2_str.alarm_setpoint >>8); 
					
					int_co2_str.pre_alarm_setpoint = 800;
					write_eeprom(EEP_INT_PRE_ALARM_SETPOINT, int_co2_str.pre_alarm_setpoint);
					write_eeprom(EEP_INT_PRE_ALARM_SETPOINT + 1, int_co2_str.pre_alarm_setpoint >>8); 
					 
				 

					write_eeprom(EEP_PRE_ALARM_SETTING_ON_TIME,2);  
					write_eeprom(EEP_PRE_ALARM_SETTING_OFF_TIME,2); 
					write_eeprom(EEP_ALARM_DELAY_TIME, 5);   
					write_eeprom(EEP_BACKLIGHT_KEEP_SECONDS,BACKLIGHT_KEEP_SECONDS_DEFAULT);
					

			// co2 output range
					write_eeprom(EEP_OUTPUT_CO2_RANGE_MIN + 1,0);
					write_eeprom(EEP_OUTPUT_CO2_RANGE_MIN,0);

		//			#ifdef CO2_SENSOR
					output_range_table[CHANNEL_CO2].max = 2000;
		//			#else 
		//			output_range_table[CHANNEL_CO2].max = 1000;
		//			#endif
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
					
					
//					write_eeprom(EEP_BACKLIGHT_KEEP_SECONDS,BACKLIGHT_KEEP_SECONDS_DEFAULT);  
					write_eeprom(EEP_HUMIDITY_FILTER,DEFAULT_FILTER);   
					write_eeprom(EEP_EXT_TEMPERATURE_FILTER,DEFAULT_FILTER);   
					write_eeprom(EEP_INT_TEMPERATURE_FILTER,DEFAULT_FILTER);

					if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
						sprintf((char *)panelname,"%s", (char *)"CO2_NET");
					else if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) )
						sprintf((char *)panelname,"%s", (char *)"Pressure");
					else if(PRODUCT_ID == STM32_PM25)
						sprintf((char *)panelname,"%s", (char *)"PM25");
					else //if((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485))
						sprintf((char *)panelname,"%s", (char *)"Humdity");
					
					 
					for(i=0;i<20;i++)			 
					{
						write_eeprom((EEP_TSTAT_NAME1 + i),panelname[i]); 
					}
					
					write_eeprom(EEP_REPLY_DELAY,10);
					write_eeprom(EEP_RECEIVE_DELAY,10);
				
					//intnital pid
					write_eeprom(EEP_PID_TEST,0x00);
	
					
					//intial variable
					STMFLASH_Unlock();	
					STMFLASH_ErasePage(AV_PAGE_FLAG);
					STMFLASH_WriteHalfWord(AV_PAGE_FLAG, 0xffff) ;
					
					STMFLASH_ErasePage(IN_PAGE_FLAG);
					STMFLASH_WriteHalfWord(IN_PAGE_FLAG, 0xffff) ;
					
					STMFLASH_ErasePage(OUT_PAGE_FLAG);
					STMFLASH_WriteHalfWord(OUT_PAGE_FLAG, 0xffff) ;
					STMFLASH_Lock();
					
//-----------------IP Mode-----------------------					
					AT24CXX_WriteOneByte(EEP_IP_MODE,0);
//-----------------IP Address-----------------------					
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_1, 192);
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_2, 168);
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_3, 0);
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_4, 34);
					
//-----------------MASK Address-----------------------		 
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_1, 255);
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_2, 255);
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_3, 255);
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_4, 0); 
//-----------------Gate Address-----------------------		  
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_1, 192);
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_2, 168);
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_3, 0);
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_4, 4); 			
//-----------------TCP Serve-----------------------	 
					AT24CXX_WriteOneByte(EEP_TCP_SERVER, 0); 
//-----------------Listen Port-----------------------	    
					AT24CXX_WriteOneByte(EEP_LISTEN_PORT_HI, (u8)(502>>8));
					AT24CXX_WriteOneByte(EEP_LISTEN_PORT_LO, (u8)502);
  
					write_eeprom(EEP_AUTO_HEAT_CONTROL,0);
					write_eeprom(EEP_OUTPUT_SEL,0);
					if(PRODUCT_ID == STM32_PM25)
					{
						pm25_reset_factory();
					}
					for(i=0;i<4;i++)			 
					{
						write_eeprom((EEP_INSTANCE_LOWORD + i),0xff); 
					}
					 
					write_eeprom(EEP_STATION_NUMBER,DEFAULT_STATION_NUMBER);
					
					light.filter = DEFAULT_FILTER;
					write_eeprom(EEP_LIGHT_FILTER,DEFAULT_FILTER);
					light.k = 15;
					write_eeprom(EEP_LIGHT_K,	 light.k); 
					write_eeprom(EEP_LIGHT_K + 1,light.k>>8); 
					
					Lcd_Full_Screen(0);
					Lcd_Show_String(1, 6, 0, (uint8 *)"Restarting...");
					Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
					SoftReset();
				}
			}
		}
		
		
//		else if(StartAdd == MODBUS_HUM_READ_DELAY)
//		{
//			hum_read_delay =   (uint16)Data_L * 1000;
//		  AT24CXX_WriteOneByte(EEP_HUM_READ_DELAY, Data_L);
//		}			
		
		else if(StartAdd == MODBUS_PROTOCOL_TYPE )			// july 21 Ron
		{
			if((Data_L == MODBUS)||(Data_L== BAC_MSTP))
			{
				AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, Data_L);
					modbus.protocal = Data_L ;
				if(Data_L == MODBUS)
					update_flag = 1;
				else
					update_flag = 2;
			}
		}
		else if(StartAdd == MODBUS_INSTANCE_LOWORD)
		{
			Instance &= 0xffff0000;
			Instance |= ((uint16)Data_H << 8 | Data_L); 
			write_eeprom(EEP_INSTANCE_LOWORD,Data_L);
			write_eeprom(EEP_INSTANCE_LOWORD + 1,Data_H); 
		}
		else if(StartAdd == MODBUS_INSTANCE_HIWORD)
		{
			Instance &= 0x0000ffff;
			Instance |= (uint32)((uint16)Data_H << 8 | Data_L) << 16; 
			write_eeprom(EEP_INSTANCE_HIWORD,Data_L);
			write_eeprom(EEP_INSTANCE_HIWORD + 1,Data_H); 
		} 
		else if(StartAdd == MODBUS_STATION_NUMBER)
		{ 
			modbus.address = Data_L;
			//write_eeprom(EEP_STATION_NUMBER,Data_L);  
			AT24CXX_WriteOneByte(EEP_ADDRESS, modbus.address);
			scan_db[0].id = modbus.address; // update the scan talbe
			Station_NUM = modbus.address;
			//Inital_Bacnet_Server();
			dlmstp_init(NULL);
		}
		else if(( StartAdd >= MODBUS_MAC_ADDRESS_1 )&&( StartAdd <= MODBUS_MAC_ADDRESS_6 ))
		{
				address_temp	= StartAdd - MODBUS_MAC_ADDRESS_1 ;
				modbus.mac_addr[address_temp] = Data_L ;
				AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_1+address_temp, Data_L);
		}
		else if(StartAdd == MODBUS_GHOST_IP_MODE )
		{
			modbus.ghost_ip_mode = Data_L ;
		}
		else if(( StartAdd >= MODBUS_GHOST_IP_ADDRESS_1 )&&( StartAdd <= MODBUS_GHOST_IP_ADDRESS_4 ))
		{
				address_temp	= StartAdd - MODBUS_GHOST_IP_ADDRESS_1 ;
				modbus.ghost_ip_addr[address_temp] =Data_L ;
		}
		else if((StartAdd >= MODBUS_GHOST_SUB_MASK_ADDRESS_1 )&&( StartAdd <= MODBUS_GHOST_SUB_MASK_ADDRESS_4 ))
		{
				address_temp	= StartAdd - MODBUS_GHOST_SUB_MASK_ADDRESS_1 ;
				modbus.ghost_mask_addr[address_temp] = Data_L ;
		}
		else if(( StartAdd >= MODBUS_GHOST_GATEWAY_ADDRESS_1 )&&( StartAdd <= MODBUS_GHOST_GATEWAY_ADDRESS_4 ))
		{
				address_temp	= StartAdd - MODBUS_GHOST_GATEWAY_ADDRESS_1 ;
				modbus.ghost_gate_addr[address_temp] = Data_L ;
		}
		else if(StartAdd == MODBUS_GHOST_TCP_SERVER )
		{
			modbus.ghost_tcp_server = Data_L ;
		}
		else if(StartAdd == MODBUS_GHOST_LISTEN_PORT )
		{
			modbus.ghost_listen_port =  ((uint16)Data_H<<8) +Data_L ;
		}
		else if(StartAdd == MODBUS_WRITE_GHOST_SYSTEM )
		{
			modbus.write_ghost_system = Data_L ;
//				if(modbus.write_ghost_system == 1)
//				{
//						modbus.ip_mode = modbus.ghost_ip_mode ;
//						modbus.tcp_server = modbus.ghost_tcp_server ;
//						modbus.listen_port = modbus.ghost_listen_port ;
//						AT24CXX_WriteOneByte(EEP_IP_MODE, modbus.ip_mode);
//						AT24CXX_WriteOneByte(EEP_TCP_SERVER, modbus.tcp_server);				
//						AT24CXX_WriteOneByte(EEP_LISTEN_PORT_HI, modbus.listen_port>>8);
//						AT24CXX_WriteOneByte(EEP_LISTEN_PORT_LO, modbus.listen_port &0xff);
//						for(i=0; i<4; i++)
//						{
//							modbus.ip_addr[i] = modbus.ghost_ip_addr[i] ;
//							modbus.mask_addr[i] = modbus.ghost_mask_addr[i] ;
//							modbus.gate_addr[i] = modbus.ghost_gate_addr[i] ;
//							
//							AT24CXX_WriteOneByte(EEP_IP_ADDRESS_1+i, modbus.ip_addr[i]);
//							AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_1+i, modbus.mask_addr[i]);
//							AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_1+i, modbus.gate_addr[i]);						
//						}
////						for(i=0; i<5; i++)
////						{
////							AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_1+i, modbus.mac_addr[i]);
////						}
////						if(!tapdev_init()) 
////						{
////							printf("Init fail\n\r");
////						}
////							while(tapdev_init())	//初始化ENC28J60错误
////							{								   
////							//	printf("ENC28J60 Init Error!\r\n");
////							delay_ms(50);
////							};	
//							IP_Change = 1; 
//						modbus.write_ghost_system = 0 ;
//				}
		}
		else if(StartAdd == MODBUS_IS_COLOR_SCREEN)
		{
			if(Data_L==1)
				isColorScreen = true;
			else if(Data_L == 0)
				isColorScreen = false;
				
			write_eeprom(EEP_IS_COLOR_SCREEN ,isColorScreen);	
		}
		else if(StartAdd == MODBUS_MAC_ENABLE )
		{
			modbus.mac_enable = Data_L ;	
		} 
	}
	else if((StartAdd < MODBUS_HUM_END)&&((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485)))
	{
		 
		if(StartAdd == MODBUS_HUM_BAUDRATE) 
		{			
			modbus.baud = Data_L ;
			switch(modbus.baud)
			{
				case 0:
					modbus.baudrate = BAUDRATE_9600 ;
					uart1_init(BAUDRATE_9600);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);					
					SERIAL_RECEIVE_TIMEOUT = 6;
				break ;
				case 1:
					modbus.baudrate = BAUDRATE_19200 ;
					uart1_init(BAUDRATE_19200);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 3;
				break;
				case 2:
					modbus.baudrate = BAUDRATE_38400 ;
					uart1_init(BAUDRATE_38400);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 2;
				break;
				case 3:
					modbus.baudrate = BAUDRATE_57600 ;
					uart1_init(BAUDRATE_57600);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 1;
				break;
				case 5:
					modbus.baudrate = BAUDRATE_76800 ;
					uart1_init(BAUDRATE_76800);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 1;
				break;				
				case 4:
					modbus.baudrate = BAUDRATE_115200 ;
					uart1_init(BAUDRATE_115200);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 1;	
				break;
				
				default:
				break ;				
			}
			modbus_init();
		} 
		else if(StartAdd == MODBUS_HUM_TEMPERATURE_DEGREE_C_OR_F)
		{
			if(Data_L)
				deg_c_or_f = 1;
			else
				deg_c_or_f = 0;
			write_eeprom(EEP_DEG_C_OR_F, (uint8)deg_c_or_f);
		}
		 
		else if(StartAdd == MODBUS_HUM_EXTERNAL_TEMPERATURE_CELSIUS)	
		{
			external_operation_value = (int16)(((uint16)Data_H << 8) | Data_L);
			if((output_auto_manual & 0x01) == 0x01)
			{
				output_manual_value_temp = external_operation_value;
			}
			else
				external_operation_flag = TEMP_CALIBRATION; 
		}
		else if(StartAdd == MODBUS_HUM_EXTERNAL_TEMPERATURE_FAHRENHEIT)
		{
			external_operation_value = ((int16)(((uint16)Data_H << 8) | Data_L) - 320) * 5 / 9;
			if((output_auto_manual & 0x01) == 0x01)
			{
				output_manual_value_temp = external_operation_value;
			}
			else
				external_operation_flag = TEMP_CALIBRATION;
		} 
		else if((StartAdd == MODBUS_HUM_HUMIDITY)||(StartAdd == MODBUS_HUM_HUMIDITY1))
		{
			external_operation_value =  (int16)(((uint16)Data_H << 8) | Data_L);
			
			if(output_auto_manual & 0x02)
				output_manual_value_humidity = external_operation_value;
			else if(external_operation_value < 950)  //< 95%
			{
				external_operation_flag = HUM_CALIBRATION;
				//Run_Timer = 0;
			}
		}
		 
		else if(StartAdd == MODBUS_HUM_SENSOR_HEATING)
		{
			if(Data_L)
				external_operation_value = 1;
			else
				external_operation_value = 0;
			external_operation_flag = HUM_HEATER;
		} 
		 
		else if(StartAdd == MODBUS_HUM_PASSWORD_ENABLE)
		{
			if(Data_L)
				use_password = 1;
			else
				use_password = 0;
			write_eeprom(EEP_USE_PASSWORD,use_password);
//	
//				start_data_save_timer();
//				flash_write_int(FLASH_USE_PASSWORD, use_password);
		}
		else if((StartAdd >= MODBUS_HUM_USER_PASSWORD0) && (StartAdd <= MODBUS_HUM_USER_PASSWORD3))
		{
			uint16 itemp;
			if((Data_L >= '0') && (Data_L <= '9'))
			{
				itemp = StartAdd - MODBUS_HUM_USER_PASSWORD0;
				user_password[itemp] = Data_L;
				write_eeprom(EEP_USER_PASSWORD0+itemp,user_password[itemp]);  
//					flash_write_int(FLASH_USER_PASSWORD0 + start_address - MODBUS_USER_PASSWORD0, main_data_buffer[5]);
			}
		}
		else if(StartAdd == MODBUS_HUM_LIGHT_VALUE)
		{ 
			uint32 itemp;
			itemp = (uint32)((uint16)Data_H << 8) | Data_L;
			if(light.org > 87)
			{
				light.k = ((itemp * 100)*27/light.org - 150*27)	/ (light.org-87);	
				light.pre_val = (uint16)itemp;
				light.val = light.pre_val;
				write_eeprom(EEP_LIGHT_K,light.k); 
				write_eeprom(EEP_LIGHT_K + 1,light.k>>8); 
			}
		} 
		else if(StartAdd == MODBUS_HUM_LIGHT_FILTER )
		{ 
			light.filter = Data_L;
			write_eeprom(EEP_LIGHT_FILTER,Data_L); 
		}
		else if(StartAdd == MODBUS_HUM_LIGHT_K )
		{ 
			light.k = ((uint16)Data_H << 8) | Data_L;
			write_eeprom(EEP_LIGHT_K,Data_L); 
			write_eeprom(EEP_LIGHT_K + 1,Data_H); 
		} 		
		else if(StartAdd == MODBUS_HUM_OUTPUT_AUTO_MANUAL)
		{
			output_auto_manual = Data_L;
//				start_data_save_timer();
		}
		else if(StartAdd == MODBUS_HUM_OUTPUT_MANUAL_VALUE_TEM)
		{
			output_manual_value_temp = ((uint16)Data_H << 8) | Data_L;
//				start_data_save_timer();
		}
		else if(StartAdd == MODBUS_HUM_OUTPUT_MANUAL_VALUE_HUM)
		{
			output_manual_value_humidity = ((uint16)Data_H << 8) | Data_L;
//				start_data_save_timer();
		}
		 
		else if(StartAdd == MODBUS_HUM_OUTPUT_RANGE_MIN_TEM)
		{
//				if(output_range_table[CHANNEL_TEMP].max > ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_TEMP].min = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN, Data_L);
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_HUM_OUTPUT_RANGE_MAX_TEM)
		{
//				if(output_range_table[CHANNEL_TEMP].min < ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_TEMP].max = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX, Data_L);
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_HUM_OUTPUT_RANGE_MIN_HUM)
		{
//				if(output_range_table[CHANNEL_HUM].max > ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_HUM].min = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN, Data_L);
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_HUM_OUTPUT_RANGE_MAX_HUM)
		{
//				if(output_range_table[CHANNEL_HUM].min < ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_HUM].max = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX, Data_L);
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX + 1, Data_H);
			}
		}
		   
		else if(StartAdd == MODBUS_HUM_BACKLIGHT_KEEP_SECONDS)
		{
			backlight_keep_seconds = Data_L;
			write_eeprom(EEP_BACKLIGHT_KEEP_SECONDS,backlight_keep_seconds);
			start_back_light(backlight_keep_seconds);
			
			
//				start_data_save_timer();
//				flash_write_int(FLASH_BACKLIGHT_KEEP_SECONDS, backlight_keep_seconds);
			
		}
		 
		else if(StartAdd == MODBUS_HUM_EXT_TEMPRATURE_FILTER) 
		{
			HumSensor.T_Filter = Data_L;
			write_eeprom(EEP_EXT_TEMPERATURE_FILTER,Data_L); 
		}
		else if(StartAdd == MODBUS_HUM_HUIDITY_FILTER) 
		{
			HumSensor.H_Filter = Data_L;
			write_eeprom(EEP_HUMIDITY_FILTER,Data_L); 
		} 
		else if(StartAdd == MODBUS_HUM_TABLE_SEL)
		{
			if(hum_exists == 1)
			{
				if((Data_L == USER)||(Data_L == FACTORY))
				{
						table_sel = Data_L;
						new_write_eeprom(EEP_TABLE_SEL,table_sel);  
						
						table_sel_enable = 1; 
						
						HumSensor.offset_h = 0;
						new_write_eeprom(EEP_HUM_OFFSET+1,0);
						new_write_eeprom(EEP_HUM_OFFSET,0); 
					
						hum_size_copy = 0;		 
						new_write_eeprom(EEP_USER_POINTS,0);    //hum_size_copy
				} 
			}
			else if(hum_exists == 2)
			{
				if(Data_L == 3)
					update_flag = 8;
			}
				
		}
		else if(StartAdd == MODBUS_HUM_USER_POINTS)
		{
			if(Data_L < 10)
			{
				hum_size_copy = Data_L;		 
				new_write_eeprom(EEP_USER_POINTS,Data_L);
				table_sel_enable = 1;
			}
		}
		else if((StartAdd >= MODBUS_HUM_USER_RH1)&&(StartAdd<= MODBUS_HUM_USER_FRE10))
		{
			uint8 temp,i,j;
			int16 itemp;
			temp = StartAdd - MODBUS_HUM_USER_RH1;
			i = temp /2;
			j = temp %2;
			
			table_sel_enable = 1;
			
			HumSensor.offset_h = 0;
			new_write_eeprom(EEP_HUM_OFFSET,0); 
			new_write_eeprom(EEP_HUM_OFFSET+1,0); 
			if(j == 0)//RH
			{
				itemp = ((uint16)Data_H << 8) | Data_L; 
				if(itemp == -1)
				{
					new_write_eeprom(EEP_USER_RH1 + i*4,255);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 1,255);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 2,255);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 3,255); 
				}
				else if(itemp == 0)
				{
					new_write_eeprom(EEP_USER_RH1 + i*4,0);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 1,0);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 2,0);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 3,0);
					 
				}
				else
				{	 

					if(hum_exists == 1)
					{						
						 new_write_eeprom(EEP_USER_RH1 + i*4 +2,HumSensor.frequency);			  
						 new_write_eeprom(EEP_USER_RH1 + i*4 +3,HumSensor.frequency >> 8); 
						 new_write_eeprom(EEP_USER_RH1 + i*4 ,Data_L);
						 new_write_eeprom(EEP_USER_RH1 + i*4 +1,Data_H); 
						 if((StartAdd == MODBUS_HUM_USER_RH1)&&(hum_size_copy == 1))
						 {     
								HumSensor.offset_h = (signed int)itemp- humidity_back;
								new_write_eeprom(EEP_HUM_OFFSET,HumSensor.offset_h); 
								new_write_eeprom(EEP_HUM_OFFSET+1,HumSensor.offset_h>>8); 					
						 }
					}
					else if(hum_exists == 2)
					{
//						 new_write_eeprom(EEP_USER_RH1 + i*4,hum_org);			  
//						 new_write_eeprom(EEP_USER_RH1 + i*4+1, (uint16)hum_org >> 8); 
						
						 new_write_eeprom(EEP_USER_RH1 + i*4,ctest[4]);			  
  					 new_write_eeprom(EEP_USER_RH1 + i*4+1, (uint16)ctest[4] >> 8); 
						
						 new_write_eeprom(EEP_USER_RH1 + i*4+2 ,Data_L);
						 new_write_eeprom(EEP_USER_RH1 + i*4 +3,Data_H);
						 update_flag = 9;
//						 new_write_eeprom(EEP_USER_RH1 + i*4 +2,hum_org);			  
//						 new_write_eeprom(EEP_USER_RH1 + i*4 +3, (uint16)hum_org >> 8); 
//						 new_write_eeprom(EEP_USER_RH1 + i*4 ,Data_L);
//						 new_write_eeprom(EEP_USER_RH1 + i*4 +1,Data_H); 
					}
				}
			}
			else//freq
			{
				new_write_eeprom( EEP_USER_FRE1 + i * 4, Data_L ) ;
				new_write_eeprom( EEP_USER_FRE1 + i * 4 + 1 , Data_H ) ;
			}
		} 
		else if(StartAdd == MODBUS_HUM_DIS_INFO)
		{ 
			dis_hum_info =  Data_L;  				
		}	
		else if(StartAdd == MODBUS_HUM_OUTPUT_SEL)
		{  
			analog_output_sel = Data_L; 
			write_eeprom(EEP_OUTPUT_SEL,analog_output_sel);
		}	 			
		else if(StartAdd == MODBUS_HUM_TEMP_OFFSET)
		{ 
			HumSensor.offset_t = ((uint16)Data_H << 8) | Data_L;    
			write_eeprom(EEP_TEMP_OFFSET,Data_L);
			write_eeprom(EEP_TEMP_OFFSET + 1,Data_H);				
		}
		else if(StartAdd == MODBUS_HUM_HUMIDITY_OFFSET)
		{ 
			HumSensor.offset_h = ((uint16)Data_H << 8) | Data_L;  
			write_eeprom(EEP_HUM_OFFSET,Data_L);
			write_eeprom(EEP_HUM_OFFSET + 1,Data_H);					
		}
		else if(StartAdd == MODBUS_HUM_CAL_DEFAULT_HUM)
		{ 	
			HumSensor.offset_h_default = ((uint16)Data_H << 8) | Data_L;  
			write_eeprom(EEP_CAL_DEFAULT_HUM ,Data_L);
			write_eeprom(EEP_CAL_DEFAULT_HUM + 1,Data_H);	
		} 
		else if(StartAdd == MODBUS_HUM_REPLY_DELAY)
		{  
			reply_delay_time = Data_L; 
			write_eeprom(EEP_REPLY_DELAY,reply_delay_time);
		}
		else if(StartAdd == MODBUS_HUM_RECEIVE_DELAY)
		{  
			receive_delay_time = Data_L; 
			write_eeprom(EEP_RECEIVE_DELAY,receive_delay_time);
		}
		else if(StartAdd == MODBUS_HUM_UART1_PARITY)
		{  
			if((Data_L == NONE_PARITY)||( Data_L == ODD_PARITY)||( Data_L == EVEN_PARITY))
			{
				uart1_parity = Data_L; 
				write_eeprom(EEP_UART1_PARITY,uart1_parity);
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
					case 5:
						modbus.baudrate = BAUDRATE_76800 ;
						uart1_init(BAUDRATE_76800);	
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
				Lcd_Full_Screen(0);
				Lcd_Show_String(1, 6, 0, (uint8 *)"Restarting...");
				Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
				SoftReset();
			}
		}
		else if(StartAdd == MODBUS_HUM_AUTO_HEAT_CONTROL)
		{  
			if((Data_L == 0)||(Data_L == 1))
			{
				auto_heat_enable = Data_L; 
				write_eeprom(EEP_AUTO_HEAT_CONTROL,Data_L);
				external_operation_value = 0;
				external_operation_flag = HUM_HEATER;
			}  
		}
		else if(StartAdd == MODBUS_HUM_DEW_PT_MIN)
		{
//				if(output_range_table[CHANNEL_CO2].max > ((pData[HeadLen+4] << 8) |pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_CO2].min = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_CO2_RANGE_MIN, Data_L);
				write_eeprom(EEP_OUTPUT_CO2_RANGE_MIN + 1, Data_H);
			}
		}
//------------------------pid1---------------------------------		
		else if(StartAdd == MODBUS_HUM_PID1_MODE)
		{  
			if(Data_L) Data_L = 1;
			else
				Data_L = 0;
			controllers[0].action = Data_L; 
			write_eeprom(EEP_PID1_MODE,Data_L);
			 
		}
		 
		else if(StartAdd == MODBUS_HUM_PID1_SETPOINT)
		{  
			PID[0].EEP_SetPoint =(((unsigned int)Data_H<<8)+Data_L) ;
			controllers[0].setpoint_value =  (int32)PID[0].EEP_SetPoint  * 100;
			write_eeprom(EEP_PID1_SETPOINT,Data_L);
			write_eeprom(EEP_PID1_SETPOINT+1,Data_H);
		}
		 
		else if(StartAdd == MODBUS_HUM_PID1_PTERM)
		{  
			controllers[0].proportional = Data_L;
			 
			write_eeprom(EEP_PID1_PTERM,Data_L);
		}
		else if(StartAdd == MODBUS_HUM_PID1_ITERM)
		{  
			controllers[0].reset = Data_L;
			 
			write_eeprom(EEP_PID1_ITERM,Data_L);
		}
//------------------------pid2--------------------------------			
		else if(StartAdd == MODBUS_HUM_PID2_MODE)
		{  
			if(Data_L) Data_L = 1;
			else
				Data_L = 0;
			controllers[1].action = Data_L; 
			write_eeprom(EEP_PID2_MODE,Data_L);
			 
		}
		 
		else if(StartAdd == MODBUS_HUM_PID2_SETPOINT)
		{  
			PID[1].EEP_SetPoint =(((unsigned int)Data_H<<8)+Data_L) ;
			controllers[1].setpoint_value =  (int32)PID[1].EEP_SetPoint  * 100;
			write_eeprom(EEP_PID2_SETPOINT,Data_L);
			write_eeprom(EEP_PID2_SETPOINT+1,Data_H);
		}
		 
		else if(StartAdd == MODBUS_HUM_PID2_PTERM)
		{  
			controllers[1].proportional = Data_L;
			 
			write_eeprom(EEP_PID2_PTERM,Data_L);
		}
		else if(StartAdd == MODBUS_HUM_PID2_ITERM)
		{  
			controllers[1].reset = Data_L;
			 
			write_eeprom(EEP_PID2_ITERM,Data_L);
		} 
//------------------------pid3--------------------------------			
		else if(StartAdd == MODBUS_HUM_PID3_MODE)
		{  
			if(Data_L) Data_L = 1;
			else
				Data_L = 0;
			controllers[2].action = Data_L; 
			write_eeprom(EEP_PID3_MODE,Data_L);
			 
		}
		 
		else if(StartAdd == MODBUS_HUM_PID3_SETPOINT)
		{  
			
			PID[2].EEP_SetPoint =(((unsigned int)Data_H<<8)+Data_L) ;
			controllers[2].setpoint_value =  (int32)PID[2].EEP_SetPoint  * 1000;
			write_eeprom(EEP_PID3_SETPOINT,Data_L);
			write_eeprom(EEP_PID3_SETPOINT+1,Data_H);
		}
		 
		else if(StartAdd == MODBUS_HUM_PID3_PTERM)
		{  
			controllers[2].proportional = Data_L;
			 
			write_eeprom(EEP_PID3_PTERM,Data_L);
		}
		else if(StartAdd == MODBUS_HUM_PID3_ITERM)
		{  
			controllers[2].reset = Data_L;
			 
			write_eeprom(EEP_PID3_ITERM,Data_L);
		} 
		else if(StartAdd == MODBUS_HUM_MODE_SELECT)
		{  
			if(Data_L)
				mode_select = PID_MODE; 
			else
				mode_select = TRANSMIT_MODE;
			write_eeprom(EEP_MODE_SELECT,Data_L);
		}  
	} 
	else if((StartAdd < MODBUS_PRESSURE_END)&&((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485)))
	{   
 		
		if(StartAdd == MODBUS_OUTPUT_RANGE_MIN_PRESSURE)
		{
//				if(output_range_table[CHANNEL_HUM].max > ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_HUM].min = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN, Data_L);
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_OUTPUT_RANGE_MAX_PRESSURE)
		{
//				if(output_range_table[CHANNEL_HUM].min < ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_HUM].max = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX, Data_L);
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_PRESSURE_PASSWORD_ENABLE)
		{
			if(Data_L)
				use_password = 1;
			else
				use_password = 0;
			write_eeprom(EEP_USE_PASSWORD,use_password);
//	
//				start_data_save_timer();
//				flash_write_int(FLASH_USE_PASSWORD, use_password);
		}
		else if((StartAdd >= MODBUS_PRESSURE_USER_PASSWORD0) && (StartAdd <= MODBUS_PRESSURE_USER_PASSWORD3))
		{
			uint16 itemp;
			if((Data_L >= '0') && (Data_L <= '9'))
			{
				itemp = StartAdd - MODBUS_PRESSURE_USER_PASSWORD0;
				user_password[itemp] = Data_L;
				write_eeprom(EEP_USER_PASSWORD0+itemp,user_password[itemp]);  
//					flash_write_int(FLASH_USER_PASSWORD0 + start_address - MODBUS_USER_PASSWORD0, main_data_buffer[5]);
			}
		}
	 
		else if(StartAdd == MODBUS_PRESSURE_BACKLIGHT_KEEP_SECONDS)
		{
			backlight_keep_seconds = Data_L;
			write_eeprom(EEP_BACKLIGHT_KEEP_SECONDS,backlight_keep_seconds);
			start_back_light(backlight_keep_seconds);
			 	
		}
		 
		else if(StartAdd == MODBUS_PRESSURE_SENSOR_MODEL) // 0=26PCF (0-100PSI),1=26PCG (0-250PSI), 10 = MPXV7002(-8 ~ +8 inWC),11=MPXV7007(-27 ~ +27 inWC)
		{
			Pressure.SNR_Model = Data_L;
			write_eeprom(EEP_PRESSURE_SENSOR_MODEL,Data_L);
			Pressure.default_unit = get_default_unit(Pressure.SNR_Model);
		}
		else if(StartAdd == MODBUS_PRESSURE_UNIT)
		{
			Pressure.unit = Data_L;
			Pressure.unit_change = 1;
			write_eeprom(EEP_PRESSURE_UNIT,Data_L);
		}	 	 
	//		else if(start_address == MODBUS_OUTPUT_RANGE_MIN_PRESSURE)
	//		{
	//			Pressure.range_press[0] = ((uint16)data_buffer[4] << 8) | data_buffer[5];
	//			write_eeprom(EEP_OUTPUT_RANGE_MIN_PRESSURE,data_buffer[5]);
	//			write_eeprom(EEP_OUTPUT_RANGE_MIN_PRESSURE + 1,data_buffer[4]); 
	//		}
	//		else if(start_address == MODBUS_OUTPUT_RANGE_MAX_PRESSURE)
	//		{
	//			Pressure.range_press[1] = ((uint16)data_buffer[4] << 8) | data_buffer[5];
	//			write_eeprom(EEP_OUTPUT_RANGE_MAX_PRESSURE,data_buffer[5]);
	//			write_eeprom(EEP_OUTPUT_RANGE_MAX_PRESSURE + 1,data_buffer[4]); 
	//		}
			else if(StartAdd == MODBUS_PRESSURE_FILTER)
			{
				Pressure.filter = Data_L;
				write_eeprom(EEP_PRESSURE_FILTER,Data_L);
			}	 	 
	 		else if(StartAdd == MODBUS_INPUT_AUTO_MANUAL_PRE)//0 = AUTO,1 = MANUAL
	 		{
	 			Pressure.auto_manu = Data_L;
//	 			write_eeprom(EEP_INPUT_AUTO_MANUAL_PRE,data_buffer[5]);
	 		}
	 		else if(StartAdd == MODBUS_INPUTPUT_MANUAL_VALUE_PRE)
	 		{
	 			output_manual_value_co2 =  ((uint16)Data_H << 8) |Data_L;
			} 
			else if(StartAdd == MODBUS_PRESSURE_VALUE_ORG)
			{
				s16 itemp; 
				if(output_auto_manual & 0x04)	//manu mode
				{
					output_manual_value_co2 = ((uint16)Data_H << 8) | Data_L;;
				}
				else
				{
					itemp = ((uint16)Data_H << 8) | Data_L;
					Pressure.org_val_offset += (itemp - Pressure.org_val );
					Pressure.org_val =  itemp;
					write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,Pressure.org_val_offset);
					write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,Pressure.org_val_offset >> 8);
				}	
			} 	 	
			else if(StartAdd == MODBUS_PRESSURE_VALUE_ORG_OFFSET)
			{
				Pressure.org_val_offset = ((uint16)Data_H << 8) | Data_L;  
				write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,Pressure.org_val_offset);
				write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,Pressure.org_val_offset >> 8);
			} 	 
			else if(StartAdd == MODBUS_PRESSURE_CAL_POINT)
			{
				Pressure.cal_point = Data_L;
				write_eeprom(EEP_PRESSURE_CAL_POINT,Data_L);
			}
			else if((StartAdd >= MODBUS_PRESSURE_CAL_PR0) && (StartAdd <= MODBUS_PRESSURE_CAL_AD9))
			{
				u8 temp;
				uint16 ad_temp;
				temp = StartAdd- MODBUS_PRESSURE_CAL_PR0;
				if(temp%2 == 0)
				{
					temp/=2;
					Pressure.cal_pr[temp] =  ((uint16)Data_H << 8) | Data_L;
					if(Pressure.cal_pr[temp] == 0xffff)	Pressure.cal_ad[temp] = 0xffff; 
					else
						Pressure.cal_ad[temp] = Pressure.ad;
					write_eeprom(EEP_CAL_PR0+temp*4 + 1 , Data_H); 
					write_eeprom(EEP_CAL_PR0+temp*4     , Data_L); 
					write_eeprom(EEP_CAL_AD0+temp*4 + 1 , Pressure.cal_ad[temp] >> 8);
					write_eeprom(EEP_CAL_AD0+temp*4     , Pressure.cal_ad[temp]);
					Pressure.cal_table_enable = 1;
				}
				else if(temp%2 == 1)
				{
					temp/=2;
					ad_temp = ((uint16)Data_H << 8) | Data_L;
					write_eeprom(EEP_CAL_AD0+temp*4 + 1 , Data_H);
					write_eeprom(EEP_CAL_AD0+temp*4     , Data_L );
					Pressure.cal_ad[temp] = ad_temp;
					Pressure.cal_table_enable = 1;							
				}
				 
			} 
			else if(StartAdd == MODBUS_PRESSURE_TABLE_SEL)
			{
				if((Data_L == USER_TABLE)||(Data_L == FACTORY_TABLE))
				{
					Pressure.table_sel = Data_L;
					write_eeprom(EEP_TABLE_SEL , Data_L);  
					Pressure.cal_table_enable = 1;
					Pressure.org_val_offset = 0;  
					write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,Pressure.org_val_offset);
					write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,Pressure.org_val_offset >> 8);
				} 
			} 
			else if(StartAdd == MODBUS_PRESSURE_USER_CAL_POINT)
			{
				Pressure.user_cal_point = Data_L;
				write_eeprom(EEP_USER_CAL_POINT,Data_L); 
				Pressure.cal_table_enable = 1;
				Pressure.org_val_offset = 0;  
				write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET,Pressure.org_val_offset);
				write_eeprom(EEP_PRESSURE_VALUE_ORG_OFFSET + 1,Pressure.org_val_offset >> 8);
			}
			else if((StartAdd >= MODBUS_PRESSURE_USER_CAL_PR0) && (StartAdd <= MODBUS_PRESSURE_USER_CAL_AD9))
			{
				u8 temp;
				temp = StartAdd- MODBUS_PRESSURE_USER_CAL_PR0;
				if(temp%2 == 0)
				{ 
					temp/=2;
					Pressure.user_cal_pr[temp] =  ((uint16)Data_H << 8) | Data_L;
					if(Pressure.user_cal_pr[temp] == 0xffff)	Pressure.user_cal_ad[temp] = 0xffff; 
					else
						Pressure.user_cal_ad[temp] = Pressure.ad;
					write_eeprom(EEP_USER_CAL_PR0+temp*4 + 1 , Data_H); 
					write_eeprom(EEP_USER_CAL_PR0+temp*4     , Data_L); 
					write_eeprom(EEP_USER_CAL_AD0+temp*4 + 1 , Pressure.user_cal_ad[temp] >> 8);
					write_eeprom(EEP_USER_CAL_AD0+temp*4     , Pressure.user_cal_ad[temp]);
					Pressure.cal_table_enable = 1;
				}  
			}
 
		else if(StartAdd == MODBUS_PRESSURE_REPLY_DELAY)
		{  
			reply_delay_time = Data_L; 
			write_eeprom(EEP_REPLY_DELAY,reply_delay_time);
		}
		else if(StartAdd == MODBUS_PRESSURE_RECEIVE_DELAY)
		{  
			receive_delay_time = Data_L; 
			write_eeprom(EEP_RECEIVE_DELAY,receive_delay_time);
		}
		else if(StartAdd == MODBUS_PRESSURE_UART1_PARITY)
		{  
			if((Data_L == NONE_PARITY)||( Data_L == ODD_PARITY)||( Data_L == EVEN_PARITY))
			{
				uart1_parity = Data_L; 
				write_eeprom(EEP_UART1_PARITY,uart1_parity);
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

					case 5:
						modbus.baudrate = BAUDRATE_76800 ;
						uart1_init(BAUDRATE_76800);	
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
				Lcd_Full_Screen(0);
				Lcd_Show_String(1, 6, 0, (uint8 *)"Restarting...");
				Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
				SoftReset();
			}
		}
		 
//------------------------pid1---------------------------------		
		else if(StartAdd == MODBUS_PRESSURE_PID1_MODE)
		{  
			if(Data_L) Data_L = 1;
			else
				Data_L = 0;
			controllers[0].action = Data_L; 
			write_eeprom(EEP_PID1_MODE,Data_L);
			 
		}
		 
		else if(StartAdd == MODBUS_PRESSURE_PID1_SETPOINT)
		{  
			PID[0].EEP_SetPoint =(((unsigned int)Data_H<<8)+Data_L) ;
			controllers[0].setpoint_value =  (int32)PID[0].EEP_SetPoint  * 100;
			write_eeprom(EEP_PID1_SETPOINT,Data_L);
			write_eeprom(EEP_PID1_SETPOINT+1,Data_H);
		}
		 
		else if(StartAdd == MODBUS_PRESSURE_PID1_PTERM)
		{  
			controllers[0].proportional = Data_L;
			 
			write_eeprom(EEP_PID1_PTERM,Data_L);
		}
		else if(StartAdd == MODBUS_PRESSURE_PID1_ITERM)
		{  
			controllers[0].reset = Data_L;
			 
			write_eeprom(EEP_PID1_ITERM,Data_L);
		}
//------------------------pid2--------------------------------			
		else if(StartAdd == MODBUS_PRESSURE_PID2_MODE)
		{  
			if(Data_L) Data_L = 1;
			else
				Data_L = 0;
			controllers[1].action = Data_L; 
			write_eeprom(EEP_PID2_MODE,Data_L);
			 
		}
		 
		else if(StartAdd == MODBUS_PRESSURE_PID2_SETPOINT)
		{  
			PID[1].EEP_SetPoint =(((unsigned int)Data_H<<8)+Data_L) ;
			controllers[1].setpoint_value =  (int32)PID[1].EEP_SetPoint  * 100;
			write_eeprom(EEP_PID2_SETPOINT,Data_L);
			write_eeprom(EEP_PID2_SETPOINT+1,Data_H);
		}
		 
		else if(StartAdd == MODBUS_PRESSURE_PID2_PTERM)
		{  
			controllers[1].proportional = Data_L;
			 
			write_eeprom(EEP_PID2_PTERM,Data_L);
		}
		else if(StartAdd == MODBUS_PRESSURE_PID2_ITERM)
		{  
			controllers[1].reset = Data_L;
			 
			write_eeprom(EEP_PID2_ITERM,Data_L);
		} 
//------------------------pid3--------------------------------			
		else if(StartAdd == MODBUS_PRESSURE_PID3_MODE)
		{  
			if(Data_L) Data_L = 1;
			else
				Data_L = 0;
			controllers[2].action = Data_L; 
			write_eeprom(EEP_PID3_MODE,Data_L);
			 
		}
		 
		else if(StartAdd == MODBUS_PRESSURE_PID3_SETPOINT)
		{  
			
			PID[2].EEP_SetPoint =(((unsigned int)Data_H<<8)+Data_L) ;
			controllers[2].setpoint_value =  (int32)PID[2].EEP_SetPoint  * 1000;
			write_eeprom(EEP_PID3_SETPOINT,Data_L);
			write_eeprom(EEP_PID3_SETPOINT+1,Data_H);
		}
		 
		else if(StartAdd == MODBUS_PRESSURE_PID3_PTERM)
		{  
			controllers[2].proportional = Data_L;
			 
			write_eeprom(EEP_PID3_PTERM,Data_L);
		}
		else if(StartAdd == MODBUS_PRESSURE_PID3_ITERM)
		{  
			controllers[2].reset = Data_L;
			 
			write_eeprom(EEP_PID3_ITERM,Data_L);
		} 
		else if(StartAdd == MODBUS_PRESSURE_MODE_SELECT)
		{  
			if(Data_L)
				mode_select = PID_MODE; 
			else
				mode_select = TRANSMIT_MODE;
			write_eeprom(EEP_MODE_SELECT,Data_L);
		}
 			
	}
	else if((StartAdd < MODBUS_PM25_END)&&((PRODUCT_ID == STM32_PM25)))
	{
		if(StartAdd == MODBUS_SENSOR_PERIOD)
		{
			if(Data_L	< MAX_WORK_PERIOD)
				pm25_sensor.period = Data_L;
			write_eeprom(EEP_SENSOR_WORK_PERIOD, Data_L);
		} 
		else if(StartAdd == MODBUS_PM25_VAL)
		{ 
			int16 itemp;
			itemp = ((unsigned int)Data_H<<8)+Data_L;
			if(pm25_sensor.auto_manual&0x01)
			{
				pm25_sensor.pm25 = itemp;
			}
			else if(itemp == 0)
				pm25_sensor.pm25_offset = 0;
			else if(itemp > 0)
			{	
//				itemp -= pm25_sensor.pm25;

				pm25_sensor.pm25_offset = (itemp - pm25_org_value)*100 / pm25_org_value;
				//pm25_sensor.pm25_offset = (itemp - pm25_sensor.pm25);
				//pm25_sensor.pm25 = itemp;
				write_eeprom(EEP_PM25_OFFSET, pm25_sensor.pm25_offset);
				write_eeprom(EEP_PM25_OFFSET + 1, pm25_sensor.pm25_offset>>8);
			}
		}
		else if(StartAdd == MODBUS_PM10_VAL)
		{ 
			int16 itemp;
			itemp = ((unsigned int)Data_H<<8)+Data_L;
			if((pm25_sensor.auto_manual>>1)&0x01)
			{
				pm25_sensor.pm10 = itemp;
			}
			else if(itemp == 0)
				pm25_sensor.pm10_offset = 0;
			else if(itemp > 0)
			{	
//				itemp -= pm25_sensor.pm10;
//				pm25_sensor.pm10_offset += (itemp - pm25_sensor.pm10);
//				pm25_sensor.pm10 = itemp;
				pm25_sensor.pm10_offset = (itemp - pm10_org_value)*100 / pm10_org_value;
				write_eeprom(EEP_PM10_OFFSET, pm25_sensor.pm10_offset);
				write_eeprom(EEP_PM10_OFFSET + 1, pm25_sensor.pm10_offset>>8);
			}
		}
		else if(StartAdd == MODBUS_MENU_SET)
		{ 
			pm25_sensor.menu.menu_set = Data_L;
			write_eeprom(EEP_MENU_SET, Data_L);
		}
		else if(StartAdd == MODBUS_SCROLL_SET)
		{ 
			pm25_sensor.menu.scroll_set = Data_L;
			write_eeprom(EEP_SCROLL_SET, Data_L);
		}
		else if(StartAdd == MODBUS_MENU_SWITCH_SECONDS)
		{ 
			if(Data_L >= 60)
				Data_L = 60;
			if(Data_L < 1)
				Data_L = 1;
			pm25_sensor.menu.seconds = Data_L;
			write_eeprom(EEP_MENU_SWITCH_SECONDS, Data_L);
		}
		else if(StartAdd == MODBUS_PM25_OFFSET)
		{ 
			pm25_sensor.pm25_offset = ((int16)Data_H<<8)|Data_L;
			write_eeprom(EEP_PM25_OFFSET, Data_L);
			write_eeprom(EEP_PM25_OFFSET + 1, Data_H);
		}
		else if(StartAdd == MODBUS_PM10_OFFSET)
		{ 
			pm25_sensor.pm10_offset = ((int16)Data_H<<8)|Data_L;
			write_eeprom(EEP_PM10_OFFSET, Data_L);
			write_eeprom(EEP_PM10_OFFSET + 1, Data_H);
		}
		else if(StartAdd == MODBUS_PM25_FILTER)
		{ 
			pm25_sensor.PM25_filter = Data_L;
			write_eeprom(EEP_PM25_FILTER, Data_L); 
		}
		else if(StartAdd == MODBUS_PM10_FILTER)
		{ 
			pm25_sensor.PM10_filter = Data_L;
			write_eeprom(EEP_PM10_FILTER, Data_L); 
		}
		else if(StartAdd == MODBUS_PM_AUTO_MANUAL)
		{ 
			pm25_sensor.auto_manual = Data_L; 
		}
		else if(StartAdd == MODBUS_PM25_RANGE_MIN)
		{
//				if(output_range_table[CHANNEL_TEMP].max > ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_TEMP].min = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN, Data_L);
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_PM25_RANGE_MAX)
		{
//				if(output_range_table[CHANNEL_TEMP].min < ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_TEMP].max = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX, Data_L);
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_PM10_RANGE_MIN)
		{
//				if(output_range_table[CHANNEL_HUM].max > ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_HUM].min = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN, Data_L);
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_PM10_RANGE_MAX)
		{
//				if(output_range_table[CHANNEL_HUM].min < ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_HUM].max = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX, Data_L);
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_PM25_PARTICLE_UINT)
		{
		   pm25_unit = Data_L;
			 write_eeprom(EEP_PM25_UNIT, Data_L);		
		}
		else if(StartAdd == MODBUS_PM25_RANGE)
		{
			if(Data_L <= PM25_0_1000)
			{
				pm25_sensor.pm25_range = Data_L;
				write_eeprom(EEP_PM25_RANGE, Data_L);
			}
		}
		else if(StartAdd == MODBUS_PM25_AREA)
		{
			if(Data_L <= 3)
			{
				pm25_sensor.AQI_area = Data_L;
				write_eeprom(EEP_PM25_AREA,Data_L);
			}
		}
		else if((StartAdd >= MODBUS_AQI_FIRST_LINE)&&(StartAdd <= MODBUS_AQI_FIFTH_LINE))
		{
			if((((uint16)(Data_H)<<8) + Data_L)<500)
			{
				aqi_table_customer[StartAdd -MODBUS_AQI_FIRST_LINE] = ((uint16)(Data_H)<<8) + (Data_L+5);
				write_eeprom(EEP_AQI_FIRST_LINE_LO+((StartAdd-MODBUS_AQI_FIRST_LINE)*2), Data_L);
				write_eeprom(EEP_AQI_FIRST_LINE_LO+((StartAdd-MODBUS_AQI_FIRST_LINE)*2)+1, Data_H);
			}
		}
		   
	}
	else //if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
	{ 
 		if((StartAdd == MODBUS_BAUDRATE )||(StartAdd == MODBUS_HUM_BAUDRATE )) 
		{			
			modbus.baud = Data_L ;
			switch(modbus.baud)
			{
				case 0:
					modbus.baudrate = BAUDRATE_9600 ;
					uart1_init(BAUDRATE_9600);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);					
					SERIAL_RECEIVE_TIMEOUT = 6;
				break ;
				case 1:
					modbus.baudrate = BAUDRATE_19200 ;
					uart1_init(BAUDRATE_19200);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 3;
				break;
				case 2:
					modbus.baudrate = BAUDRATE_38400 ;
					uart1_init(BAUDRATE_38400);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 2;
				break;
				case 3:
					modbus.baudrate = BAUDRATE_57600 ;
					uart1_init(BAUDRATE_57600);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 1;
				break;

				case 5:
					modbus.baudrate = BAUDRATE_76800 ;
					uart1_init(BAUDRATE_76800);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 1;
				break;			
				
				case 4:
					modbus.baudrate = BAUDRATE_115200 ;
					uart1_init(BAUDRATE_115200);
					AT24CXX_WriteOneByte(EEP_BAUDRATE, Data_L);	
					SERIAL_RECEIVE_TIMEOUT = 1;		
				default:
				break ;				
			}
			modbus_init();
		}
		else if(StartAdd == MODBUS_CO2_INTERNAL_EXIST)
		{ 				
			
				if(Data_L < SENSOR_TYPE_ALL)
				{
						internal_co2_module_type =Data_L; 
						AT24CXX_WriteOneByte(EEP_CO2_MODULE_TYPE, internal_co2_module_type);
						if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200) || (internal_co2_module_type == MH_Z19B))
						{
							set_sub_serial_baudrate(9600); 
						}
						else if((internal_co2_module_type == MAYBE_TEMCO_CO2) || (internal_co2_module_type == TEMCO_CO2) || (internal_co2_module_type == SCD30))
						{  
							set_sub_serial_baudrate(19200);
						}  
				} 	 	
		}
/*********************************after register100*********************************/	
		else if(StartAdd == MODBUS_TEMPERATURE_SENSOR_SELECT)
		{
			if(((uint16)Data_H << 8) |Data_L)
				temperature_sensor_select = 1;
			else
				temperature_sensor_select = 0; 
			write_eeprom(EEP_SENSOR_SELECT, temperature_sensor_select);
		}
		else if(StartAdd == MODBUS_TEMPERATURE_DEGREE_C_OR_F) 
		{
			if(Data_L)
				deg_c_or_f = 1;
			else
				deg_c_or_f = 0;
			write_eeprom(EEP_DEG_C_OR_F, (uint8)deg_c_or_f);
		}
		else if(StartAdd == MODBUS_INTERNAL_TEMPERATURE_CELSIUS)
		{
			internal_temperature_offset += (((uint16)Data_H << 8) | Data_L) - internal_temperature_c;
			write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 0, (uint8)(internal_temperature_offset & 0x00ff));
			write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 1, (uint8)(internal_temperature_offset >> 8));
		}
		else if(StartAdd == MODBUS_INTERNAL_TEMPERATURE_FAHRENHEIT)
		{
			internal_temperature_offset += ((int16)(((uint16)Data_H << 8) | Data_L) - internal_temperature_f) * 5 / 9;
			write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 0, (uint8)(internal_temperature_offset & 0x00ff));
			write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 1, (uint8)(internal_temperature_offset >> 8));
		}
		else if(StartAdd == MODBUS_EXTERNAL_TEMPERATURE_CELSIUS) 	
		{
			external_operation_value = (int16)(((uint16)Data_H << 8) | Data_L);
			if((output_auto_manual & 0x01) == 0x01)
			{
				output_manual_value_temp = external_operation_value;
			}
			else
				external_operation_flag = TEMP_CALIBRATION;
		}
		else if(StartAdd == MODBUS_EXTERNAL_TEMPERATURE_FAHRENHEIT) 
		{
			external_operation_value = ((int16)(((uint16)Data_H << 8) | Data_L) - 320) * 5 / 9;
			if((output_auto_manual & 0x01) == 0x01)
			{
				output_manual_value_temp = external_operation_value;
			}
			else
				external_operation_flag = TEMP_CALIBRATION;
		}
		else if(StartAdd == MODBUS_TEMPERATURE_OFFSET_INTERNAL)
		{
			internal_temperature_offset  = (int16)(((uint16)Data_H << 8) | Data_L);
			write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 0, (uint8)(internal_temperature_offset & 0x00ff));
			write_eeprom(EEP_INTERNAL_TEMPERATURE_OFFSET + 1, (uint8)(internal_temperature_offset >> 8));
		}
		else if(StartAdd == MODBUS_HUMIDITY)
		{
			external_operation_value =  (int16)(((uint16)Data_H << 8) | Data_L);
			
			if(output_auto_manual & 0x02)
				output_manual_value_humidity = external_operation_value;
			else if(external_operation_value < 950)  //< 95%
			{
				external_operation_flag = HUM_CALIBRATION;
				//Run_Timer = 0;
			}
		}
		else if(StartAdd == MODBUS_HUMIDITY_FREQUENCY)
		{
			// read only
		}
		else if(StartAdd == MODBUS_HUMIDITY_SENSOR_HEATING)
		{
			if(Data_L)
				external_operation_value = 1;
			else
				external_operation_value = 0;
			external_operation_flag = HUM_HEATER;
		} 
		else if(StartAdd == MODBUS_CO2_INTERNAL)
		{
			if((output_auto_manual & 0x04) == 0x04)
			{
				output_manual_value_co2 =  (int16)(((uint16)Data_H << 8) | Data_L);	
			}
			else
			{
				int_co2_str.co2_offset += ((int16)(((uint16)Data_H << 8) | Data_L)) - int_co2_str.co2_int;
				write_eeprom(EEP_INT_CO2_OFFSET, (uint8)(int_co2_str.co2_offset));
				write_eeprom(EEP_INT_CO2_OFFSET + 1, (uint8)(int_co2_str.co2_offset >> 8));
			}
		}
		else if(StartAdd == MODBUS_CO2_INTERNAL_OFFSET)
		{
			int_co2_str.co2_offset =  (int16)(((uint16)Data_H << 8) | Data_L);
			write_eeprom(EEP_INT_CO2_OFFSET, (uint8)(int_co2_str.co2_offset));
			write_eeprom(EEP_INT_CO2_OFFSET + 1, (uint8)(int_co2_str.co2_offset >> 8));
		}
		else if(StartAdd == MODBUS_CO2_INTERNAL_PREALARM_SETPOINT)
		{
			int_co2_str.pre_alarm_setpoint =  (int16)(((uint16)Data_H << 8) | Data_L);
			write_eeprom(EEP_INT_PRE_ALARM_SETPOINT, Data_L);
			write_eeprom(EEP_INT_PRE_ALARM_SETPOINT + 1,Data_H);
		}
		else if(StartAdd == MODBUS_CO2_INTERNAL_ALARM_SETPOINT)
		{
			int_co2_str.alarm_setpoint = ((uint16)Data_H<< 8) | Data_L;
			write_eeprom(EEP_INT_ALARM_SETPOINT, Data_L);
			write_eeprom(EEP_INT_ALARM_SETPOINT + 1, Data_H);
		}
//		else if((StartAdd >= MODBUS_CO2_EXTERNAL_START) && (StartAdd < MODBUS_CO2_EXTERANL_END))
//		{
//			int16 temp = ext_co2_str[StartAdd - MODBUS_CO2_EXTERNAL_START].co2_offset + (((uint16)Data_H << 8) | Data_L) - ext_co2_str[StartAdd - MODBUS_CO2_EXTERNAL_START].co2_int;
//			write_parameters_to_nodes(StartAdd - MODBUS_CO2_EXTERNAL_START + 1, SLAVE_MODBUS_CO2_OFFSET, (uint16)temp);
//			ext_co2_str[StartAdd - MODBUS_CO2_EXTERNAL_START].co2_offset = temp;
//		}
//		else if((StartAdd >= MODBUS_CO2_EXTERNAL_OFFSET_START) && (StartAdd < MODBUS_CO2_EXTERNAL_OFFSET_END))
//		{
//			int16 temp = ((uint16)Data_H << 8) | Data_L;
//			write_parameters_to_nodes(StartAdd - MODBUS_CO2_EXTERNAL_OFFSET_START + 1, SLAVE_MODBUS_CO2_OFFSET, (uint16)temp);
//			ext_co2_str[StartAdd - MODBUS_CO2_EXTERNAL_OFFSET_START].co2_offset = temp;
//		}
//		else if((StartAdd >= MODBUS_CO2_EXTERNAL_PREALARM_SETPOINT_START) && (StartAdd < MODBUS_CO2_EXTERNAL_PREALARM_SETPOINT_END))
//		{
//			uint16 temp = ((uint16)Data_H << 8) | Data_L;
//			write_parameters_to_nodes(StartAdd - MODBUS_CO2_EXTERNAL_PREALARM_SETPOINT_START + 1, SLAVE_MODBUS_CO2_PRE_ALARM_SETPOINT, temp);
//			ext_co2_str[StartAdd - MODBUS_CO2_EXTERNAL_PREALARM_SETPOINT_START].pre_alarm_setpoint = temp;
//		}
//		else if((StartAdd >= MODBUS_CO2_EXTERNAL_ALARM_SETPOINT_START) && (StartAdd < MODBUS_CO2_EXTERNAL_ALARM_SETPOINT_END))
//		{
//			uint16 temp = ((uint16)Data_H << 8) | Data_L;
//			write_parameters_to_nodes(StartAdd - MODBUS_CO2_EXTERNAL_ALARM_SETPOINT_START + 1, SLAVE_MODBUS_CO2_ALARM_SETPOINT, temp);
//			ext_co2_str[StartAdd - MODBUS_CO2_EXTERNAL_ALARM_SETPOINT_START].alarm_setpoint = temp;
//		}
		else if(StartAdd == MODBUS_CO2_SLOPE_DETECT_VALUE)
		{
			co2_slope_detect_value = ((uint16)Data_H << 8) | Data_L;
			write_eeprom(EEP_CO2_SLOPE_DETECT_VALUE, Data_L);
			write_eeprom(EEP_CO2_SLOPE_DETECT_VALUE + 1,Data_H);
		}
		else if(StartAdd == MODBUS_CO2_FILTER)
		{
			int_co2_filter = Data_L;
			write_eeprom(EEP_CO2_FILTER, Data_L);
		}
		else if(StartAdd == MODBUS_PASSWORD_ENABLE)
		{
			if(Data_L)
				use_password = 1;
			else
				use_password = 0;
			write_eeprom(EEP_USE_PASSWORD,use_password);
//	
//				start_data_save_timer();
//				flash_write_int(FLASH_USE_PASSWORD, use_password);
		}
		else if((StartAdd >= MODBUS_USER_PASSWORD0) && (StartAdd <= MODBUS_USER_PASSWORD3))
		{
			uint16 itemp;
			if((Data_L >= '0') && (Data_L <= '9'))
			{
				itemp = StartAdd - MODBUS_USER_PASSWORD0;
				user_password[itemp] = Data_L;
				write_eeprom(EEP_USER_PASSWORD0+itemp,user_password[itemp]);  
//					flash_write_int(FLASH_USER_PASSWORD0 + start_address - MODBUS_USER_PASSWORD0, main_data_buffer[5]);
			}
		}
		else if(StartAdd == MODBUS_RTC_CENTURY)
		{
//				if(main_data_buffer[5] == 19)
//				{
//					rtc_set_time(RTC_MONTH, TOBCD(Modbus.Time.Clk.mon) | 0x80);
//				}
//				else if(main_data_buffer[5] == 20)
//				{
//					rtc_set_time(RTC_MONTH, TOBCD(Modbus.Time.Clk.mon) & 0x7f);
//				}
		}
		else if(StartAdd == MODBUS_RTC_YEAR)
		{
			uint16 itemp;
//				rtc_set_time(RTC_YEAR, TOBCD(main_data_buffer[5]));
			itemp = ((uint16)Data_H << 8) | Data_L;
			if((itemp > 2000)&&(itemp < 2099))
			{
				calendar.w_year = itemp; 
				Time_Adjust();
			}
		}
		else if(StartAdd == MODBUS_RTC_MONTH)
		{
//				if(Modbus.Time.Clk.century == 19)
//				{
//					rtc_set_time(RTC_MONTH, TOBCD(main_data_buffer[5]) | 0x80);
//				}
//				else if(Modbus.Time.Clk.century == 20)
//				{
//					rtc_set_time(RTC_MONTH, TOBCD(main_data_buffer[5]) & 0x7f);
//				}
			calendar.w_month = Data_L;
			Time_Adjust();
		}
		else if(StartAdd == MODBUS_RTC_DAY)
		{
//				rtc_set_time(RTC_DATE, TOBCD(main_data_buffer[5]));
			calendar.w_date =Data_L;
			Time_Adjust();
		}
		else if(StartAdd == MODBUS_RTC_WEEK)
		{
//				rtc_set_time(RTC_WEEKDAY, TOBCD(main_data_buffer[5]));
			calendar.week = Data_L;
			Time_Adjust();
		}
		else if(StartAdd == MODBUS_RTC_HOUR)
		{
//				rtc_set_time(RTC_HOUR, TOBCD(main_data_buffer[5]));
			calendar.hour = Data_L;
			Time_Adjust();
		}
		else if(StartAdd == MODBUS_RTC_MINUTE)
		{
//				rtc_set_time(RTC_MINUTE, TOBCD(main_data_buffer[5]));
			calendar.min = Data_L;
			Time_Adjust();
		}
		else if(StartAdd == MODBUS_RTC_SECOND)
		{
//				rtc_set_time(RTC_SECOND, TOBCD(main_data_buffer[5]));
			calendar.sec = Data_L;
			Time_Adjust();
		}
		else if(StartAdd == MODBUS_ALARM_AUTO_MANUAL)
		{
			if(!(Data_L & ALARM_MANUAL)) // auto alarm
				alarm_state &= ~ALARM_MANUAL;
			else
				alarm_state = Data_L ;
		}
		else if(StartAdd == MODBUS_PRE_ALARM_SETTING_ON_TIME)
		{
			if(Data_L > 0)
			{
				pre_alarm_on_time = (Data_L < ALARM_ON_TIME_MAX) ? Data_L : ALARM_ON_TIME_MAX;
				write_eeprom(EEP_PRE_ALARM_SETTING_ON_TIME, pre_alarm_on_time);
			}
		}
		else if(StartAdd == MODBUS_PRE_ALARM_SETTING_OFF_TIME)
		{ 
			if(Data_L > 0)
			{
				pre_alarm_off_time = ((Data_L < ALARM_OFF_TIME_MAX) ? Data_L : ALARM_OFF_TIME_MAX);
				write_eeprom(EEP_PRE_ALARM_SETTING_OFF_TIME, pre_alarm_off_time);
			}
		}
		else if(StartAdd == MODBUS_ALARM_DELAY_TIME)
		{ 
			alarm_delay_time = Data_L;
			write_eeprom(EEP_ALARM_DELAY_TIME, alarm_delay_time);
		}
		else if(StartAdd == MODBUS_OUTPUT_AUTO_MANUAL)
		{
			output_auto_manual = Data_L;
//				start_data_save_timer();
		}
		else if(StartAdd == MODBUS_OUTPUT_MANUAL_VALUE_TEM)
		{
			output_manual_value_temp = ((uint16)Data_H << 8) | Data_L;
//				start_data_save_timer();
		}
		else if(StartAdd == MODBUS_OUTPUT_MANUAL_VALUE_HUM)
		{
			output_manual_value_humidity = ((uint16)Data_H << 8) | Data_L;
//				start_data_save_timer();
		}
		else if(StartAdd == MODBUS_OUTPUT_MANUAL_VALUE_CO2)
		{
			output_manual_value_co2 = ((uint16)Data_H << 8) | Data_L;
//				start_data_save_timer();
		}
//		else if((StartAdd == MODBUS_OUTPUT_MODE)||(StartAdd == MODBUS_HUM_OUTPUT_MODE))
//		{
//			// read only
//		}
		else if(StartAdd == MODBUS_OUTPUT_RANGE_MIN_TEM)
		{
//				if(output_range_table[CHANNEL_TEMP].max > ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_TEMP].min = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN, Data_L);
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_OUTPUT_RANGE_MAX_TEM)
		{
//				if(output_range_table[CHANNEL_TEMP].min < ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_TEMP].max = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX, Data_L);
				write_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_OUTPUT_RANGE_MIN_HUM)
		{
//				if(output_range_table[CHANNEL_HUM].max > ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_HUM].min = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN, Data_L);
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_OUTPUT_RANGE_MAX_HUM)
		{
//				if(output_range_table[CHANNEL_HUM].min < ((pData[HeadLen+4] << 8) | pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_HUM].max = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX, Data_L);
				write_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX + 1, Data_H);
			}
		}
		else if((StartAdd == MODBUS_OUTPUT_RANGE_MIN_CO2)||(StartAdd == MODBUS_DEW_PT_MIN))
		{
//				if(output_range_table[CHANNEL_CO2].max > ((pData[HeadLen+4] << 8) |pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_CO2].min = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_CO2_RANGE_MIN, Data_L);
				write_eeprom(EEP_OUTPUT_CO2_RANGE_MIN + 1, Data_H);
			}
		}
		else if((StartAdd == MODBUS_OUTPUT_RANGE_MAX_CO2)||(StartAdd == MODBUS_DEW_PT_MAX))
		{
//				if(output_range_table[CHANNEL_CO2].min < ((pData[HeadLen+4] << 8) |pData[HeadLen+5]))
			{
				output_range_table[CHANNEL_CO2].max = ((uint16)Data_H << 8) | Data_L;
				write_eeprom(EEP_OUTPUT_CO2_RANGE_MAX, Data_L);
				write_eeprom(EEP_OUTPUT_CO2_RANGE_MAX + 1, Data_H);
			}
		}
		else if(StartAdd == MODBUS_MENU_BLOCK_SECONDS)
		{
//				menu_block_seconds = main_data_buffer[5];
//				start_data_save_timer();
//				flash_write_int(FLASH_MENU_BLOCK_SECONDS, menu_block_seconds);
		}
		else if(StartAdd == MODBUS_BACKLIGHT_KEEP_SECONDS)
		{
			backlight_keep_seconds = Data_L;
			write_eeprom(EEP_BACKLIGHT_KEEP_SECONDS,backlight_keep_seconds);
			start_back_light(backlight_keep_seconds);
			
			
//				start_data_save_timer();
//				flash_write_int(FLASH_BACKLIGHT_KEEP_SECONDS, backlight_keep_seconds);
			
		}
		else if(StartAdd == MODBUS_EXTERNAL_NODES_PLUG_AND_PLAY)
		{
			if(((uint16)Data_H << 8) | Data_L)
				external_nodes_plug_and_play = 1;
			else
				external_nodes_plug_and_play = 0;
//				start_data_save_timer();
//				flash_write_int(FLASH_EXTERNAL_NODES_PLUG_AND_PLAY, external_nodes_plug_and_play);
		}

		
		
		 
		else if(StartAdd == MODBUS_RESET)
		{ 				// only for test, added it by chelsea
			
			co2_reset();
		
//				 modbus.reset = pData[HeadLen+5] ;
//				if(modbus.reset == 1)
//				{
//					for(i=0; i<255; i++)
//					AT24CXX_WriteOneByte(i, 0xff);
//				}				
//				EEP_Dat_Init();
//////				AT24CXX_WriteOneByte(16, 1);
//				SoftReset();
		}
		
		else if((StartAdd >= MODBUS_TEST1) && (StartAdd <= MODBUS_TEST20))
		{ 				
			ctest[StartAdd - MODBUS_TEST1] = Data_L + Data_H * 256;
			
			if(StartAdd == MODBUS_TEST1)//re-initial uart1
			{
				write_eeprom(EEP_RESTART_NUM, Data_L);
			}
		} 

		
		else if(StartAdd == MODBUS_INT_TEMPRATURE_FILTER) 
		{
			Temperature_Filter =Data_L;
			write_eeprom(EEP_INT_TEMPERATURE_FILTER,Data_L); 
		}
		else if(StartAdd == MODBUS_EXT_TEMPRATURE_FILTER) 
		{
			HumSensor.T_Filter = Data_L;
			write_eeprom(EEP_EXT_TEMPERATURE_FILTER,Data_L); 
		}
		else if(StartAdd == MODBUS_HUIDITY_FILTER) 
		{
			HumSensor.H_Filter = Data_L;
			write_eeprom(EEP_HUMIDITY_FILTER,Data_L); 
		}
 
		else if(StartAdd == MODBUS_TABLE_SEL)
		{
			if((Data_L == USER)||(Data_L == FACTORY))
			{
					table_sel = Data_L;
					new_write_eeprom(EEP_TABLE_SEL,table_sel);  
					
					table_sel_enable = 1; 
					
					HumSensor.offset_h = 0;
					new_write_eeprom(EEP_HUM_OFFSET+1,0);
					new_write_eeprom(EEP_HUM_OFFSET,0); 
				
					hum_size_copy = 0;		 
					new_write_eeprom(EEP_USER_POINTS,0);    //hum_size_copy
			} 
		}
		else if(StartAdd == MODBUS_USER_POINTS)
		{
			if(Data_L < 10)
			{
				hum_size_copy = Data_L;		 
				new_write_eeprom(EEP_USER_POINTS,Data_L);
				table_sel_enable = 1;
			}
		}
		else if((StartAdd >= MODBUS_USER_RH1)&&(StartAdd<= MODBUS_USER_FRE10))
		{
			uint8 temp,i,j;
			int16 itemp;
			temp = StartAdd - MODBUS_USER_RH1;
			i = temp /2;
			j = temp %2;
			
			table_sel_enable = 1;
			
			HumSensor.offset_h = 0;
			new_write_eeprom(EEP_HUM_OFFSET,0); 
			new_write_eeprom(EEP_HUM_OFFSET+1,0); 
			if(j == 0)
			{
				itemp = ((uint16)Data_H << 8) | Data_L; 
				if(itemp == -1)
				{
					new_write_eeprom(EEP_USER_RH1 + i*4,255);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 1,255);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 2,255);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 3,255); 
				}
				else if(itemp == 0)
				{
					new_write_eeprom(EEP_USER_RH1 + i*4,0);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 1,0);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 2,0);
					new_write_eeprom(EEP_USER_RH1 + i*4 + 3,0);
					 
				}
				else
				{	   
					 new_write_eeprom(EEP_USER_RH1 + i*4 +2,HumSensor.frequency);			  
					 new_write_eeprom(EEP_USER_RH1 + i*4 +3,HumSensor.frequency >> 8); 
					 new_write_eeprom(EEP_USER_RH1 + i*4 ,Data_L);
					 new_write_eeprom(EEP_USER_RH1 + i*4 +1,Data_H); 
					 if((StartAdd == MODBUS_USER_RH1)&&(hum_size_copy == 1))
					 {     
							HumSensor.offset_h = (signed int)itemp- humidity_back;
							new_write_eeprom(EEP_HUM_OFFSET,HumSensor.offset_h); 
							new_write_eeprom(EEP_HUM_OFFSET+1,HumSensor.offset_h>>8); 					
					 }
				}
			}
			else
			{
				new_write_eeprom( EEP_USER_FRE1 + i * 4, Data_L ) ;
				new_write_eeprom( EEP_USER_FRE1 + i * 4 + 1 , Data_H ) ;
			}
		} 
		else if(StartAdd == MODBUS_DIS_INFO)
		{ 
			dis_hum_info =  Data_L;  				
		}	
		else if(StartAdd == MODBUS_OUTPUT_SEL)
		{  
			analog_output_sel = Data_L; 
			write_eeprom(EEP_OUTPUT_SEL,analog_output_sel);
		}	 			
		else if(StartAdd == MODBUS_TEMP_OFFSET)
		{ 
			HumSensor.offset_t = ((uint16)Data_H << 8) | Data_L;    
			write_eeprom(EEP_TEMP_OFFSET,Data_L);
			write_eeprom(EEP_TEMP_OFFSET + 1,Data_H);				
		}
		else if(StartAdd == MODBUS_HUM_OFFSET)
		{ 
			HumSensor.offset_h = ((uint16)Data_H << 8) | Data_L;  
			write_eeprom(EEP_HUM_OFFSET,Data_L);
			write_eeprom(EEP_HUM_OFFSET + 1,Data_H);					
		}
		else if(StartAdd == MODBUS_CAL_DEFAULT_HUM)
		{ 	
			HumSensor.offset_h_default = ((uint16)Data_H << 8) | Data_L;  
			write_eeprom(EEP_CAL_DEFAULT_HUM ,Data_L);
			write_eeprom(EEP_CAL_DEFAULT_HUM + 1,Data_H);	
		} 
		else if(StartAdd == MODBUS_REPLY_DELAY)
		{  
			reply_delay_time = Data_L; 
			write_eeprom(EEP_REPLY_DELAY,reply_delay_time);
		}
		else if(StartAdd == MODBUS_RECEIVE_DELAY)
		{  
			receive_delay_time = Data_L; 
			write_eeprom(EEP_RECEIVE_DELAY,receive_delay_time);
		}
		else if(StartAdd == MODBUS_UART1_PARITY)
		{  
			if((Data_L == NONE_PARITY)||( Data_L == ODD_PARITY)||( Data_L == EVEN_PARITY))
			{
				uart1_parity = Data_L; 
				write_eeprom(EEP_UART1_PARITY,uart1_parity);
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
					
					case 5:
						modbus.baudrate = BAUDRATE_76800 ;
						uart1_init(BAUDRATE_76800);	
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
				Lcd_Full_Screen(0);
				Lcd_Show_String(1, 6, 0, (uint8 *)"Restarting...");
				Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
				SoftReset();
			}
		}
		else if(StartAdd == MODBUS_AUTO_HEAT_CONTROL)
		{  
			if((Data_L == 0)||(Data_L == 1))
			{
				auto_heat_enable = Data_L; 
				write_eeprom(EEP_AUTO_HEAT_CONTROL,Data_L);
				external_operation_value = 0;
				external_operation_flag = HUM_HEATER;
			}  
		}
//------------------------pid1---------------------------------		
		else if(StartAdd == MODBUS_PID1_MODE)
		{  
			if(Data_L) Data_L = 1;
			else
				Data_L = 0;
			controllers[0].action = Data_L; 
			write_eeprom(EEP_PID1_MODE,Data_L);
			 
		}
		 
		else if(StartAdd == MODBUS_PID1_SETPOINT)
		{  
			PID[0].EEP_SetPoint =(((unsigned int)Data_H<<8)+Data_L) ;
			controllers[0].setpoint_value =  (int32)PID[0].EEP_SetPoint  * 100;
			write_eeprom(EEP_PID1_SETPOINT,Data_L);
			write_eeprom(EEP_PID1_SETPOINT+1,Data_H);
		}
		 
		else if(StartAdd == MODBUS_PID1_PTERM)
		{  
			controllers[0].proportional = Data_L;
			 
			write_eeprom(EEP_PID1_PTERM,Data_L);
		}
		else if(StartAdd == MODBUS_PID1_ITERM)
		{  
			controllers[0].reset = Data_L;
			 
			write_eeprom(EEP_PID1_ITERM,Data_L);
		}
//------------------------pid2--------------------------------			
		else if(StartAdd == MODBUS_PID2_MODE)
		{  
			if(Data_L) 
				Data_L = 1;
			else
				Data_L = 0;
			controllers[1].action = Data_L; 
			write_eeprom(EEP_PID2_MODE,Data_L);
			 
		}
		 
		else if(StartAdd == MODBUS_PID2_SETPOINT)
		{  
			PID[1].EEP_SetPoint =(((unsigned int)Data_H<<8)+Data_L) ;
			controllers[1].setpoint_value =  (int32)PID[1].EEP_SetPoint  * 100;
			write_eeprom(EEP_PID2_SETPOINT,Data_L);
			write_eeprom(EEP_PID2_SETPOINT+1,Data_H);
		}
		 
		else if(StartAdd == MODBUS_PID2_PTERM)
		{  
			controllers[1].proportional = Data_L;
			 
			write_eeprom(EEP_PID2_PTERM,Data_L);
		}
		else if(StartAdd == MODBUS_PID2_ITERM)
		{  
			controllers[1].reset = Data_L;
			 
			write_eeprom(EEP_PID2_ITERM,Data_L);
		} 
//------------------------pid3--------------------------------			
		else if(StartAdd == MODBUS_PID3_MODE)
		{  
			if(Data_L) 
				Data_L = 1;
			else
				Data_L = 0;
			controllers[2].action = Data_L; 
			write_eeprom(EEP_PID3_MODE,Data_L);
			 
		}
		 
		else if(StartAdd == MODBUS_PID3_SETPOINT)
		{  
			
			PID[2].EEP_SetPoint =(((unsigned int)Data_H<<8)+Data_L) ;
			controllers[2].setpoint_value =  (int32)PID[2].EEP_SetPoint  * 1000;
			write_eeprom(EEP_PID3_SETPOINT,Data_L);
			write_eeprom(EEP_PID3_SETPOINT+1,Data_H);
		}
		 
		else if(StartAdd == MODBUS_PID3_PTERM)
		{  
			controllers[2].proportional = Data_L;
			 
			write_eeprom(EEP_PID3_PTERM,Data_L);
		}
		else if(StartAdd == MODBUS_PID3_ITERM)
		{  
			controllers[2].reset = Data_L;
			 
			write_eeprom(EEP_PID3_ITERM,Data_L);
		} 
		else if(StartAdd == MODBUS_MODE_SELECT)
		{  
			if(Data_L)
				mode_select = PID_MODE; 
			else
				mode_select = TRANSMIT_MODE;
			write_eeprom(EEP_MODE_SELECT,Data_L);
		}

		else if(StartAdd == MODBUS_CO2_DISABLE_AUTOCAL)
		{
			if(Data_L == 1)
				update_flag = 3;
			else if(Data_L == 0)
				update_flag = 4;
		}
		
		else if(StartAdd == MODBUS_CO2_CAL_MHZ19)
		{
			if(Data_L == 0)
				update_flag = 5;
//			else if((Data_L + ((uint16)Data_H << 8)) >= 400)
//			{
//			mhz19_cal_h = Data_H;
//			mhz19_cal_l = Data_L;
//			update_flag = 5;
//			cal_co2_MHZ19B_span[0] = 0xff;
//			cal_co2_MHZ19B_span[1] = 0x01;
//			cal_co2_MHZ19B_span[2] = 0x88;
//			cal_co2_MHZ19B_span[3] = Data_H;
//			cal_co2_MHZ19B_span[4] = Data_L;
//			cal_co2_MHZ19B_span[5] = 0;
//			cal_co2_MHZ19B_span[6] = 0;
//			cal_co2_MHZ19B_span[7] = 0;
//			check_sum = 0;
//			for(i=1;i<8;i++)
//				check_sum += cal_co2_MHZ19B_span[i];
//			cal_co2_MHZ19B_span[8] = 0xff - check_sum + 1;
//			}
		}
		
	 else if(StartAdd == MODBUS_CO2_AUTOCAL_DAY)
	 {
	   update_flag = 6;
	 }
	 #if 1//defined (COLOR_SCREEN)
	 else if(StartAdd == MODBUS_SCREEN_AREA_1)
	 {
		 if(Data_L<6)
		 {
			 screenArea1 = Data_L;
			 switch(screenArea1)
			 {
				 case SCREEN_AREA_TEMP:
					disp_icon(55, 55, sunicon, 10, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
				break;
				
				case SCREEN_AREA_HUMI:
					disp_icon(55, 55, moonicon, 10, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
				break;
				
				case SCREEN_AREA_CO2:
					disp_icon(55, 55, athome, 10, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
				break;
				
//				case SCREEN_AREA_NONE:
//					
//				break;
			 }
			 write_eeprom(EEP_SCREEN_AREA_1,screenArea1);
			 isFirstLineChange = true;
//			lastCO2 = 0;
//			lastTemp = -100;
//			lastHumi = -1;			 
		 }
	 }
	 else if(StartAdd == MODBUS_SCREEN_AREA_2)
	 {
		 if(Data_L<6)
		 {
			 screenArea2 = Data_L;
			 switch(screenArea2)
			{
				case SCREEN_AREA_TEMP:
					disp_icon(55, 55, sunicon, 10+HUM_POS, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
				break;
				
				case SCREEN_AREA_HUMI:
					disp_icon(55, 55, moonicon, 10+HUM_POS, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
				break;
				
				case SCREEN_AREA_CO2:
					disp_icon(55, 55, athome, 10+HUM_POS, THIRD_CH_POS+CO2_POSY_OFFSET*8, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
				break;
			}
			 write_eeprom(EEP_SCREEN_AREA_2,screenArea2); 
			isSecondLineChange = true;
//			lastCO2 = 0;
//			lastTemp = -100;
//			lastHumi = -1;
		 }
	 }
	 else if(StartAdd == MODBUS_ENABLE_SCROLL)
	 {
		 if(Data_L<2)
		 {
			 enableScroll = Data_L;
			 write_eeprom(EEP_ENABLE_SCROLL, enableScroll);
			 LCDtest();
		 }
	 }
	 else if(StartAdd == MODBUS_SCREEN_AREA_3)
	 {
		 if(Data_L<6)
		 {
			 screenArea3 = Data_L;
			 write_eeprom(EEP_SCREEN_AREA_3,screenArea3); 
			 isThirdLineChange = true;
//			 lastCO2 = 0;
//			lastTemp = -100;
//			lastHumi = -1;
		 }
	 }
	 else if(StartAdd == MODBUS_SCREEN_MANUAL_RESET)
	 {
		 if(Data_L == 1)
			 LCDtest();
	 }
	 #endif
	 else if(StartAdd == CO2_FRC_VALUE)
	 {
		 sensirion_co2_cmd_ForcedCalibration[4] = Data_H;	
			sensirion_co2_cmd_ForcedCalibration[5] = Data_L;
			scd30_co2_cmd_status = SCD30_SET_FRC; 
		 co2_frc = (uint16)(sensirion_co2_cmd_ForcedCalibration[4]<<8 )|sensirion_co2_cmd_ForcedCalibration[5];
	 }
			
			
		
//------------------end pid ----------------------------
		
		else if(StartAdd == MODBUS_OUTPUT_HUM_VOL_OFFSET)
		{ 	
			output_offset[0][CHANNEL_HUM] = ((uint16)Data_H << 8) | Data_L;  
			write_eeprom(EEP_OUTPUT_HUM_VOL_OFFSET ,Data_L);
			write_eeprom(EEP_OUTPUT_HUM_VOL_OFFSET + 1,Data_H);	
		}
		else if(StartAdd == MODBUS_OUTPUT_TEMP_VOL_OFFSET)
		{ 	
			output_offset[0][CHANNEL_TEMP] =((uint16)Data_H << 8) | Data_L;  
			write_eeprom(EEP_OUTPUT_TEMP_VOL_OFFSET ,Data_L);
			write_eeprom(EEP_OUTPUT_TEMP_VOL_OFFSET + 1,Data_H);	
		}
		else if(StartAdd == MODBUS_OUTPUT_CO2_VOL_OFFSET)
		{ 	
			output_offset[0][CHANNEL_CO2] = ((uint16)Data_H << 8) | Data_L ;  
			write_eeprom(EEP_OUTPUT_CO2_VOL_OFFSET ,Data_L);
			write_eeprom(EEP_OUTPUT_CO2_VOL_OFFSET + 1,Data_H);	
		}
		
		else if(StartAdd == MODBUS_OUTPUT_HUM_CUR_OFFSET)
		{ 	
			output_offset[1][CHANNEL_HUM] = ((uint16)Data_H << 8) | Data_L;  
			write_eeprom(EEP_OUTPUT_HUM_CUR_OFFSET ,Data_L);
			write_eeprom(EEP_OUTPUT_HUM_CUR_OFFSET + 1,Data_H);	
		}
		else if(StartAdd == MODBUS_OUTPUT_TEMP_CUR_OFFSET)
		{ 	
			output_offset[1][CHANNEL_TEMP] = ((uint16)Data_H << 8) | Data_L ;  
			write_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET ,Data_L);
			write_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET + 1,Data_H);	
		}
		else if(StartAdd == MODBUS_OUTPUT_CO2_CUR_OFFSET)
		{ 	
			output_offset[1][CHANNEL_CO2] = ((uint16)Data_H << 8) | Data_L;  
			write_eeprom(EEP_OUTPUT_CO2_CUR_OFFSET ,Data_L);
			write_eeprom(EEP_OUTPUT_CO2_CUR_OFFSET + 1,Data_H);	
		} 
		
		else if(StartAdd == MODBUS_TEST4)
		{
		  ctest[4] = ((uint16)Data_H << 8) | Data_L; 
		}
		

	} 
} 


u8_t bacnet_to_modbus[300];
//static void responseData(u16 start_address)
void responseCmd(u8 type, u8* pData)
{
	u8  i, temp1 =0, temp2 = 0;
	u16 send_cout = 0 ;
	u8  sendbuf[300];
	u8 HeadLen = 0 ;
	u16  RegNum;
	u8 cmd  ;
	u16 StartAdd ;
	
	if(type == 0 || type == 4)
	{
		HeadLen = 0 ;	 
	}
	else
	{
		HeadLen = 6 ;
		for(i=0; i<6; i++)
		{
			sendbuf[i] = 0 ;	
		} 
	}
	if(pData[HeadLen] == 0)
	{
		serial_restart();
		return;  // if id = 0, it is the broadcast cmd, we reply nothing.
	}
	cmd = pData[HeadLen + 1]; 
	StartAdd = (u16)(pData[HeadLen + 2] <<8 ) + pData[HeadLen + 3];
	RegNum = (u8)pData[HeadLen + 5];
	
	
	if(cmd == WRITE_VARIABLES)
	{
		
		
		send_cout = HeadLen ;

		if(type == 0 || type == 4)
		{
			for(i = 0; i < rece_size; i++)
			{
				sendbuf[send_cout++] = pData[i] ;
				

			}
			memcpy(uart_send, sendbuf, send_cout);
			USART_SendDataString(send_cout);		
		}
		else // TCP   dont have CRC 
		{
		//	SetTransactionId(6 + UIP_HEAD);
			sendbuf[0] = pData[0];//0;			//	TransID
			sendbuf[1] = pData[1];//TransID++;	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = 0;	//	Len
			sendbuf[5] = 6 ;

			for (i = 0;i < 6;i++)
			{
				sendbuf[HeadLen + i] = pData[HeadLen + i];	
			}
			
			memcpy(tcp_server_sendbuf,sendbuf,6+ HeadLen);
			tcp_server_sendlen = 6 + HeadLen;
//			if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
//			{				
//				TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, 6 + UIP_HEAD, TCPIP_SEND_NOT_FINAL); 
//				cSemaphoreGive( xSemaphore_tcp_send );
//			}
		}

		
	}
	else if(cmd == MULTIPLE_WRITE)
	{
		//send_cout = HeadLen ;

		
		if(type == 0 || type == 4)
		{		
			for(i = 0; i < 6; i++)
			{
			sendbuf[HeadLen+i] = pData[HeadLen+i] ;
			crc16_byte(sendbuf[HeadLen+i]);
			}
			sendbuf[HeadLen+i] = CRChi ;
			sendbuf[HeadLen+i+1] = CRClo ;
			memcpy(uart_send, sendbuf, 8);
			USART_SendDataString(8);
		}
		else
		{
				sendbuf[0] = pData[0] ;
				sendbuf[1] = pData[1] ;
				sendbuf[2] = 0 ;
				sendbuf[3] = 0 ;
				sendbuf[4] = 0; 
				sendbuf[5] =6;					
				for (i = 0;i < 6;i++)
				{
					sendbuf[HeadLen + i] = pData[HeadLen + i];	
				}
				memcpy(tcp_server_sendbuf,sendbuf,	6 + HeadLen);
				tcp_server_sendlen = 6 + HeadLen;
		}
		
				
	}
	else if(cmd == READ_VARIABLES)
	{
		u16 address;
		u16 address_temp ;
//		u16 div_temp ;
//		u16 address_buf ;		
		sendbuf[HeadLen] = pData[HeadLen] ;
		sendbuf[HeadLen + 1] = pData[HeadLen + 1];
		sendbuf[HeadLen + 2] = RegNum * 2;
		crc16_byte(sendbuf[HeadLen]);
		crc16_byte(sendbuf[HeadLen + 1]);
		crc16_byte(sendbuf[HeadLen + 2]);
		send_cout = HeadLen + 3 ;
		for(i = 0; i < RegNum; i++)
		{
			address = StartAdd + i;
			if(address  < ORIGINALADDRESSVALUE)
			{	
				if(address <= MODBUS_SERIALNUMBER_HIWORD + 1)
				{
					temp1 = 0 ;
					temp2 = modbus.serial_Num[address] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_VERSION_NUMBER_LO)
				{
					temp1 = 0 ;
					temp2 =  (u8)(SOFTREV) ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);		
				}
				else if(address == MODBUS_VERSION_NUMBER_HI)
				{
					temp1 = 0 ;
					temp2 =  (u8)(SOFTREV >> 8) ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);	
				}
				else if(address == MODBUS_ADDRESS)
				{
					temp1 = 0 ;
					temp2 =  modbus.address;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);		
				}
				else if(address == MODBUS_PRODUCT_MODEL)
				{
					temp1 = 0 ;
					temp2 = PRODUCT_ID;//modbus.product;//44 for test
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HARDWARE_REV)
				{
					temp1 = 0 ;
					temp2 =  modbus.hardware_Rev;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				} 
				else if(address == MODBUS_HUM_VERSION)
				{ 
					temp1= 0;
					temp2= humidity_version;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}
				else if(address == MODBUS_SENSOR_TYPE)
				{ 
					temp1 = 0 ;
					if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485)) 
						temp2 =  SENSOR_TYPE1;
					else if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) ) 
						temp2 =  SENSOR_TYPE2;
					else
						temp2 =  SENSOR_TYPE3;
					
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);	
				}
				else if(address == MODBUS_BAUDRATE) 
				{
					temp1 = 0 ;
					temp2 =  modbus.baud;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}
				
	 
				else if(address == MODBUS_UPDATE_STATUS)
				{
					temp1 = 0 ;
					temp2 =   modbus.update;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
//				else if(address == MODBUS_HUM_READ_DELAY)
//				{
//					temp1 = 0 ;
//					temp2 = hum_read_delay;
//					sendbuf[send_cout++] = temp1 ;
//					sendbuf[send_cout++] = temp2 ;
//					crc16_byte(temp1);
//					crc16_byte(temp2);					
//				}
				
				else if(address == MODBUS_SERINALNUMBER_WRITE_FLAG)
				{
					temp1 = 0 ;
					temp2 =  modbus.SNWriteflag ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PROTOCOL_TYPE)
				{
					temp1 = 0 ;
					temp2 =  modbus.protocal ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_INSTANCE_LOWORD)
				{   
					temp1 = Instance>>8 ;
					temp2 = Instance;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_INSTANCE_HIWORD)
				{   
					temp1 = Instance>>24;
					temp2 =  Instance>>16;
					sendbuf[send_cout++] = temp1;
					sendbuf[send_cout++] = temp2;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_STATION_NUMBER)
				{   
					temp1 = 0 ;
					temp2 =  Station_NUM;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_MAC_ADDRESS_1)&&(address<= MODBUS_MAC_ADDRESS_6))
				{
					address_temp = address - MODBUS_MAC_ADDRESS_1 ;
					temp1 = 0 ;
					temp2 =  modbus.mac_addr[address_temp] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_IP_MODE)
				{
					temp1 = 0 ;
					temp2 =  modbus.ip_mode ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_IP_ADDRESS_1)&&(address<= MODBUS_IP_ADDRESS_4))
				{
					address_temp = address - MODBUS_IP_ADDRESS_1 ;
					temp1 = 0 ;
					temp2 =  modbus.ip_addr[address_temp] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_SUB_MASK_ADDRESS_1)&&(address<= MODBUS_SUB_MASK_ADDRESS_4))
				{
					address_temp = address - MODBUS_SUB_MASK_ADDRESS_1 ;
					temp1 = 0 ;
					temp2 =  modbus.mask_addr[address_temp] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_GATEWAY_ADDRESS_1)&&(address<= MODBUS_GATEWAY_ADDRESS_4))
				{
					address_temp = address - MODBUS_GATEWAY_ADDRESS_1 ;
					temp1 = 0 ;
					temp2 =  modbus.gate_addr[address_temp] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_TCP_SERVER)
				{
					temp1 = 0 ;
					temp2 =  modbus.tcp_server ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_LISTEN_PORT)
				{
					temp1 = (modbus.listen_port>>8)&0xff ;
					temp2 =  modbus.listen_port &0xff ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_GHOST_IP_MODE)
				{
					temp1 = 0 ;
					temp2 =  modbus.ghost_ip_mode;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_GHOST_IP_ADDRESS_1)&&(address<= MODBUS_GHOST_IP_ADDRESS_4))
				{
					address_temp = address - MODBUS_GHOST_IP_ADDRESS_1 ;
					temp1 = 0 ;
					temp2 =  modbus.ghost_ip_addr[address_temp] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if((address >= MODBUS_GHOST_SUB_MASK_ADDRESS_1)&&(address<= MODBUS_GHOST_SUB_MASK_ADDRESS_4))
				{
					address_temp = address - MODBUS_GHOST_SUB_MASK_ADDRESS_1 ;
					temp1 = 0 ;
					temp2 =  modbus.ghost_mask_addr[address_temp] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_GHOST_GATEWAY_ADDRESS_1)&&(address<= MODBUS_GHOST_GATEWAY_ADDRESS_4))
				{
					address_temp = address - MODBUS_GHOST_GATEWAY_ADDRESS_1 ;
					temp1 = 0 ;
					temp2 =  modbus.ghost_gate_addr[address_temp] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_GHOST_TCP_SERVER)
				{
					temp1 = 0 ;
					temp2 =  modbus.ghost_tcp_server ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_GHOST_LISTEN_PORT)
				{
					temp1 = (modbus.ghost_listen_port>>8)&0xff ;
					temp2 =  modbus.ghost_listen_port &0xff ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_WRITE_GHOST_SYSTEM)
					{
					temp1 = 0 ;
					temp2 = modbus.write_ghost_system;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_IS_COLOR_SCREEN)
				{
					temp1 = 0;
					temp2 = isColorScreen;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_MAC_ENABLE)
				{				
					temp1 = 0 ;
					temp2 = modbus.mac_enable;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);	
				}
				else
				{
					temp1 = 0 ;
					temp2 = 0;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
			} 
			else if((address >= MODBUS_TEST1)&&(address <= MODBUS_TEST20))
			{ 				
				
//				test[0] = sizeof(Str_out_point); //45
//				test[1] = sizeof(Str_in_point);  //46
//				test[2] = sizeof(Str_variable_point);//39
//				test[3] = sizeof(Str_program_point); //37
//				test[4] = sizeof(Str_weekly_routine_point);//42
//				test[5] = sizeof(Str_annual_routine_point);//33
//				test[6] = sizeof(Wr_one_day); //16
//				test[7] = sizeof(Str_controller_point);//28
//				test[8] = MODBUS_CONTROLLER_BLOCK_FIRST;
//				test[9] = MODBUS_CONTROLLER_BLOCK_LAST;
//				if(address == MODBUS_TEST5)
//				{
//					temp1 = (uint16)HumSensor.temperature_c >> 8 ;
//					temp2 = (uint16)HumSensor.temperature_c & 0xff;
//				
//				}
				temp1 = ((int16)ctest[address - MODBUS_TEST1]) >> 8 ;
				temp2 = ((int16)ctest[address - MODBUS_TEST1]) & 0xff;
				
				sendbuf[send_cout++] = temp1 ;
				sendbuf[send_cout++] = temp2 ;
				crc16_byte(temp1);
				crc16_byte(temp2);
			}
 
/*********************************************************************************/
/******************* read IN OUT by block start ******************************************/
			else if(address >= MODBUS_USER_BLOCK_FIRST && address <= MODBUS_USER_BLOCK_LAST)
			{
				U16_T far temp;
				temp = read_user_data_by_block(address);
				
				temp1 = (temp >> 8) & 0xFF;;
				temp2 = temp & 0xFF;
				sendbuf[send_cout++] = temp1 ;
				sendbuf[send_cout++] = temp2 ;
				crc16_byte(temp1);
				crc16_byte(temp2);
			} 
/*******************************after register 100****************************************************/				
			else if((address < MODBUS_HUM_END )&&((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485)))
			{
				 			
				if(address == MODBUS_HUM_TEMPERATURE_DEGREE_C_OR_F)
				{ 
					temp1 = 0 ;
					temp2 = deg_c_or_f  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_EXTERNAL_TEMPERATURE_CELSIUS)	
				{  
					temp1 = HumSensor.temperature_c >> 8 ;
					temp2 = HumSensor.temperature_c  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_EXTERNAL_TEMPERATURE_FAHRENHEIT)
				{ 
					temp1 = HumSensor.temperature_f >> 8 ;
					temp2 = HumSensor.temperature_f  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address == MODBUS_HUM_HUMIDITY)||(address == MODBUS_HUM_HUMIDITY1))
				{  
					temp1 = HumSensor.humidity >> 8 ;
					temp2 = HumSensor.humidity  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_FREQUENCY)
				{  
					temp1 = HumSensor.frequency >> 8 ;
					temp2 = HumSensor.frequency  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}	
					
				else if(address == MODBUS_HUM_SENSOR_HEATING)
				{
					temp1 = 0;
					temp2 = hum_heat_status;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}	 
				else if(address == MODBUS_HUM_PASSWORD_ENABLE)
				{ 
					temp1= 0;
					temp2= use_password;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_HUM_USER_PASSWORD0) && (address <= MODBUS_HUM_USER_PASSWORD3))
				{
					temp1= 0;
					temp2= user_password[address - MODBUS_HUM_USER_PASSWORD0];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_LIGHT_FILTER)
				{
					temp1= 0;
					temp2= light.filter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_LIGHT_AD)
				{
					temp1= light.ad >> 8;
					temp2= light.ad;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_LIGHT_VALUE)
				{
					temp1= light.val >> 8;
					temp2= light.val & 0xff;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_LIGHT_K)
				{
					temp1= light.k >> 8;
					temp2= light.k;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_OUTPUT_AUTO_MANUAL)
				{ 
					temp1= 0;
					temp2= output_auto_manual;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_OUTPUT_MANUAL_VALUE_TEM)
				{ 
					temp1= HIGH_BYTE(output_manual_value_temp) ;
					temp2= LOW_BYTE(output_manual_value_temp);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_OUTPUT_MANUAL_VALUE_HUM)
				{ 
					temp1= HIGH_BYTE(output_manual_value_humidity);
					temp2= LOW_BYTE(output_manual_value_humidity);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_OUTPUT_MODE) 
				{ 
					temp1= 0 ;
					temp2= output_mode;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_OUTPUT_RANGE_MIN_TEM)
				{
					temp1= HIGH_BYTE(output_range_table[CHANNEL_TEMP].min);
					temp2= LOW_BYTE(output_range_table[CHANNEL_TEMP].min);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_OUTPUT_RANGE_MAX_TEM)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_TEMP].max);
					temp2= LOW_BYTE(output_range_table[CHANNEL_TEMP].max);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_OUTPUT_RANGE_MIN_HUM)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_HUM].min);
					temp2= LOW_BYTE(output_range_table[CHANNEL_HUM].min) ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_OUTPUT_RANGE_MAX_HUM)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_HUM].max);
					temp2= LOW_BYTE(output_range_table[CHANNEL_HUM].max);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_BACKLIGHT_KEEP_SECONDS)
				{ 
					temp1= 0 ;
					temp2= backlight_keep_seconds ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_EXT_TEMPRATURE_FILTER) 
				{ 
					temp1= 0;
					temp2= HumSensor.T_Filter ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_HUIDITY_FILTER) 
				{ 
					temp1= 0;
					temp2= HumSensor.H_Filter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	 
				
				else if(address == MODBUS_HUM_HUMDITY_SN)
				{ 
					temp1= HumSensor.sn>>8;
					temp2= HumSensor.sn;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}
				else if(address == MODBUS_HUM_CAL_FAC_PTS)
				{ 
					temp1= HumSensor.counter>>8;
					temp2= HumSensor.counter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_TABLE_SEL)
				{
					temp1= 0;
					temp2= table_sel;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	 
				else if((address >= MODBUS_HUM_COUNT1_H)&&(address<= MODBUS_HUM_RH10_H))
				{
					uint8 temp,i,j;
					temp = address - MODBUS_HUM_COUNT1_H;
					i = temp /2;
					j = temp %2;
					if(j==0)
					{	
						temp1 = read_eeprom(HUMCOUNT1_H + i*4 + 1);	
						temp2 = read_eeprom(HUMCOUNT1_H + i*4);     
						 
					}
					else
					{	
						temp1 = read_eeprom(HUMRH1_H + i*4 + 1);	
						temp2 = read_eeprom(HUMRH1_H + i*4);     
						
					} 
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_USER_POINTS)
				{ 
					temp1= 0;
					temp2= hum_size_copy ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_HUM_USER_RH1)&&(address<= MODBUS_HUM_USER_FRE10))
				{
					uint8 temp,i,j;
					temp = address - MODBUS_HUM_USER_RH1;
					i = temp /2;
					j = temp %2;
					if(j==1)
					{	
						temp1 = read_eeprom(EEP_USER_FRE1 + i*4 + 1) ;	
						temp2 = read_eeprom(EEP_USER_FRE1 + i*4) ;      
					}
					else
					{	
						temp1 = read_eeprom(EEP_USER_RH1 + + i*4 + 1);	
						temp2 = read_eeprom(EEP_USER_RH1 + i*4) ;      
					} 
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_HUM_DEW_PT)
				{ 
					temp1= HumSensor.dew_pt >> 8;
					temp2= HumSensor.dew_pt;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_DEW_PT_F)
				{ 
					temp1= HumSensor.dew_pt_F>>8;
					temp2= HumSensor.dew_pt_F ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_DIS_INFO)
				{ 
					temp1=  0;
					temp2=  dis_hum_info;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_OUTPUT_SEL)
				{ 
					temp1=  0;
					temp2=  analog_output_sel;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_HUM_PWS)
				{ 	
					temp1= HumSensor.Pws>>8 ;
					temp2= HumSensor.Pws ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_MIX_RATIO)
				{ 
					temp1= HumSensor.Mix_Ratio>>8;
					temp2= HumSensor.Mix_Ratio ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_ENTHALPY)
				{ 
					temp1= HumSensor.Enthalpy>>8;
					temp2= HumSensor.Enthalpy ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				  
				
				else if(address == MODBUS_HUM_TEMP_OFFSET)
				{ 
					temp1=  HumSensor.offset_t >> 8;
					temp2=  HumSensor.offset_t;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_HUM_HUMIDITY_OFFSET)
				{ 
					temp1= HumSensor.offset_h>>8;
					temp2= HumSensor.offset_h ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_CAL_DEFAULT_HUM)
				{ 	
					temp1= HumSensor.offset_h_default>>8;
					temp2= HumSensor.offset_h_default;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_HUM_K_LINE)
				{
					int16 itemp;
					itemp = k_line * 1000;   
					temp1= itemp >> 8;
					temp2=  itemp;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_B_LINE)
				{  
					temp1= (int16)b_line >> 8;
					temp2=  (int8)b_line;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	 
				else if(address == MODBUS_HUM_REPLY_DELAY)
				{ 
					temp1=  0;
					temp2=  reply_delay_time;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_RECEIVE_DELAY)
				{ 
					temp1=  0;
					temp2=  receive_delay_time;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_UART1_PARITY)
				{ 
					temp1=  0;
					temp2=  uart1_parity;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_AUTO_HEAT_CONTROL)
				{ 
					temp1=  0;
					temp2=  auto_heat_enable;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_ORIGINAL_HUM)
				{ 
					temp1=  HumSensor.org_hum>>8;
					temp2=  HumSensor.org_hum;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_DEW_PT_MIN)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_CO2].min);
					temp2= LOW_BYTE(output_range_table[CHANNEL_CO2].min);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	//-----------------------------pid1-------------------------//			
				else if(address == MODBUS_HUM_PID1_MODE)
				{ 
					temp1=  0;
					temp2=  controllers[0].action;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_PID1_SETPOINT)
				{ 
					temp1=  PID[0].EEP_SetPoint >>8;
					temp2=  PID[0].EEP_SetPoint;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_PID1_PTERM)
				{ 
					temp1=  0;
					temp2=  controllers[0].proportional;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_PID1_ITERM)
				{ 
					temp1=  0;
					temp2=  controllers[0].reset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_PID1_PID)
				{ 
					temp1=  PID[0].EEP_Pid>>8;
					temp2=  PID[0].EEP_Pid;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_HUM_PID1_PID+1)
				{ 
					temp1=  controllers[0].value>>8;
					temp2=  controllers[0].value;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_HUM_PID1_PID+2)
				{ 
					temp1=  controllers[0].value>>24;
					temp2=  controllers[0].value>>16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
	//-----------------------------pid2-------------------------//			
				else if(address == MODBUS_HUM_PID2_MODE)
				{ 
					temp1=  0;
					temp2=  controllers[1].action;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_PID2_SETPOINT)
				{ 
					temp1=  PID[1].EEP_SetPoint >>8;
					temp2=  PID[1].EEP_SetPoint;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_PID2_PTERM)
				{ 
					temp1=  0;
					temp2=  controllers[1].proportional;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_PID2_ITERM)
				{ 
					temp1=  0;
					temp2=  controllers[1].reset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_PID2_PID)
				{ 
					temp1=  PID[1].EEP_Pid>>8;
					temp2=  PID[1].EEP_Pid;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_PID2_PID+1)
				{ 
					temp1=  controllers[1].value>>8;
					temp2=  controllers[1].value;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_HUM_PID2_PID+2)
				{ 
					temp1=  controllers[1].value>>24;
					temp2=  controllers[1].value>>16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	//-----------------------------pid3-------------------------//			
				else if(address == MODBUS_HUM_PID3_MODE)
				{ 
					temp1=  0;
					temp2=  controllers[2].action;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_PID3_SETPOINT)
				{ 
					temp1=  PID[2].EEP_SetPoint >>8;
					temp2=  PID[2].EEP_SetPoint;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_HUM_PID3_PTERM)
				{ 
					temp1=  0;
					temp2=  controllers[2].proportional;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_PID3_ITERM)
				{ 
					temp1=  0;
					temp2=  controllers[2].reset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUM_PID3_PID)
				{ 
					temp1=  PID[2].EEP_Pid>>8;
					temp2=  PID[2].EEP_Pid;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_HUM_PID3_PID+1)
				{ 
					temp1=  controllers[2].value>>8;
					temp2=  controllers[2].value;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_HUM_PID3_PID+2)
				{ 
					temp1=  controllers[2].value>>24;
					temp2=  controllers[2].value>>16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_HUM_MODE_SELECT)
				{ 
					temp1=  0;
					temp2= mode_select;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	//---------------------end pid ------------------------			
				
				
				 
				else if( address == TSTAT_NAME_ENABLE || (address == PRESSURE_TSTAT_NAME_ENABLE))  
				{ 
					temp1= 0;
					temp2= 0x56;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}	
				else if((address >= TSTAT_NAME1) && (address <= TSTAT_NAME10))  
				{
					u16 temp = address - TSTAT_NAME1;  
					temp1= panelname[temp * 2];
					temp2= panelname[temp * 2 + 1];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= PRESSURE_TSTAT_NAME1) && (address <= PRESSURE_TSTAT_NAME8))  
				{
					u16 temp = address - TSTAT_NAME1;  
					temp1= panelname[temp * 2];
					temp2= panelname[temp * 2 + 1];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}				
				else
				{
					temp1 = 0 ;
					temp2 = 0;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
			}
			else if((address < MODBUS_PRESSURE_END)&&((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485)))
			{
 				
				if(address == MODBUS_PRESSURE_SENSOR_MODEL ) // 0=26PCF (0-100PSI),1=26PCG (0-250PSI), 10 = MPXV7002(-8 ~ +8 inWC),11=MPXV7007(-27 ~ +27 inWC)
				{ 
					temp1= 0;
					temp2= Pressure.SNR_Model;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_UNIT)
				{ 
					temp1= 0;
					temp2= Pressure.unit;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}	 
				else if(address == MODBUS_PRESSURE_UNIT_DEFAULT)
				{ 
					temp1= 0;
					temp2= Pressure.default_unit;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PRESSURE_FILTER)
				{ 
					temp1= 0;
					temp2= Pressure.filter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}	
				else if(address == MODBUS_PRESSURE_SLOPE)
				{
					u16 itemp;
					itemp =  Pressure.k_line*100; 
					temp1= itemp >> 8;
					temp2= itemp;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_INTERCEPT)  
				{
					u16 itemp;
					itemp =  Pressure.b_line; 
					temp1= itemp >> 8;
					temp2= itemp;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_RANGE_MIN_PRESSURE)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_HUM].min);
					temp2= LOW_BYTE(output_range_table[CHANNEL_HUM].min) ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_RANGE_MAX_PRESSURE)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_HUM].max);
					temp2= LOW_BYTE(output_range_table[CHANNEL_HUM].max);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	 			else if(address == MODBUS_INPUT_AUTO_MANUAL_PRE)//0 = AUTO,1 = MANUAL
	 			{ 
					temp1= 0;
					temp2= Pressure.auto_manu;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
	 			}
	 			else if(address == MODBUS_INPUTPUT_MANUAL_VALUE_PRE)
	 			{ 
					temp1= HIGH_BYTE(output_manual_value_co2);
					temp2= LOW_BYTE(output_manual_value_co2);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
	 			}
				else if(address == MODBUS_PREESURE_AD) 
				{ 
					temp1= Pressure.ad >> 8;
					temp2= Pressure.ad ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_VALUE_ORG)
				{ 
					temp1= Pressure.org_val >> 8;
					temp2= Pressure.org_val;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}	
				else if(address == MODBUS_PRESSURE_VALUE_ORG_OFFSET)
				{ 
					temp1= Pressure.org_val_offset >> 8;
					temp2= Pressure.org_val_offset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_VALUE_INDEX)
				{ 
					temp1= Pressure.index>>8;
					temp2= Pressure.index;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_VALUE_BASE_L)
				{ 
					temp1= Pressure.base >> 8;
					temp2= Pressure.base ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_VALUE_BASE_H)
				{ 
					temp1= Pressure.base >> 24;
					temp2= Pressure.base >> 16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_CAL_POINT)
				{ 
					temp1= 0;
					temp2= Pressure.cal_point;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_PRESSURE_CAL_PR0) && (address <= MODBUS_PRESSURE_CAL_AD9))
				{
					u8 temp;
					temp = address- MODBUS_PRESSURE_CAL_PR0;
					if(temp%2 == 0)
					{
						temp/=2;
						temp1 = Pressure.cal_pr[temp]>>8 ;
						temp2 = Pressure.cal_pr[temp] ;
					}
					else
					{
						temp/=2;
						temp1 = Pressure.cal_ad[temp]>>8 ;
						temp2 = Pressure.cal_ad[temp] ;
					} 
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_TABLE_SEL)
				{ 
					temp1= 0;
					temp2= Pressure.table_sel;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PRESSURE_OUTPUT_MODEL)
				{ 
					temp1= 0;
					temp2= output_mode;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PRESSURE_USER_CAL_POINT)
				{ 
					temp1= 0;
					temp2= Pressure.user_cal_point;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_PRESSURE_USER_CAL_PR0) && (address <= MODBUS_PRESSURE_USER_CAL_AD9))
				{
					u8 temp;
					temp = address- MODBUS_PRESSURE_USER_CAL_PR0;
					if(temp%2 == 0)
					{
						temp/=2;
						temp1 = Pressure.user_cal_pr[temp]>>8 ;
						temp2 = Pressure.user_cal_pr[temp] ;
					}
					else
					{
						temp/=2;
						temp1 = Pressure.user_cal_ad[temp]>>8 ;
						temp2 = Pressure.user_cal_ad[temp] ;
					} 
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_SENSOR_STATUS)
				{ 		
					temp1= Pressure.out_rng_flag;     //out range is high byte 
					temp2= Pressure.sensor_status;    // read sensor status
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PREESURE_AD)
				{  
					temp1= Pressure.ad >> 8;
					temp2= Pressure.ad ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
	 
				else if(address == MODBUS_PRESSURE_REPLY_DELAY)
				{ 
					temp1=  0;
					temp2=  reply_delay_time;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_RECEIVE_DELAY)
				{ 
					temp1=  0;
					temp2=  receive_delay_time;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_UART1_PARITY)
				{ 
					temp1=  0;
					temp2=  uart1_parity;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
	//-----------------------------pid1-------------------------//			
				else if(address == MODBUS_PRESSURE_PID1_MODE)
				{ 
					temp1=  0;
					temp2=  controllers[0].action;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PRESSURE_PID1_SETPOINT)
				{ 
					temp1=  PID[0].EEP_SetPoint >>8;
					temp2=  PID[0].EEP_SetPoint;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PRESSURE_PID1_PTERM)
				{ 
					temp1=  0;
					temp2=  controllers[0].proportional;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_PID1_ITERM)
				{ 
					temp1=  0;
					temp2=  controllers[0].reset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_PID1_PID)
				{ 
					temp1=  PID[0].EEP_Pid>>8;
					temp2=  PID[0].EEP_Pid;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_PRESSURE_PID1_PID+1)
				{ 
					temp1=  controllers[0].value>>8;
					temp2=  controllers[0].value;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PRESSURE_PID1_PID+2)
				{ 
					temp1=  controllers[0].value>>24;
					temp2=  controllers[0].value>>16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
	//-----------------------------pid2-------------------------//			
				else if(address == MODBUS_PRESSURE_PID2_MODE)
				{ 
					temp1=  0;
					temp2=  controllers[1].action;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PRESSURE_PID2_SETPOINT)
				{ 
					temp1=  PID[1].EEP_SetPoint >>8;
					temp2=  PID[1].EEP_SetPoint;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PRESSURE_PID2_PTERM)
				{ 
					temp1=  0;
					temp2=  controllers[1].proportional;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_PID2_ITERM)
				{ 
					temp1=  0;
					temp2=  controllers[1].reset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_PID2_PID)
				{ 
					temp1=  PID[1].EEP_Pid>>8;
					temp2=  PID[1].EEP_Pid;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_PID2_PID+1)
				{ 
					temp1=  controllers[1].value>>8;
					temp2=  controllers[1].value;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PRESSURE_PID2_PID+2)
				{ 
					temp1=  controllers[1].value>>24;
					temp2=  controllers[1].value>>16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	//-----------------------------pid3-------------------------//			
				else if(address == MODBUS_PRESSURE_PID3_MODE)
				{ 
					temp1=  0;
					temp2=  controllers[2].action;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PRESSURE_PID3_SETPOINT)
				{ 
					temp1=  PID[2].EEP_SetPoint >>8;
					temp2=  PID[2].EEP_SetPoint;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PRESSURE_PID3_PTERM)
				{ 
					temp1=  0;
					temp2=  controllers[2].proportional;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_PID3_ITERM)
				{ 
					temp1=  0;
					temp2=  controllers[2].reset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_PID3_PID)
				{ 
					temp1=  PID[2].EEP_Pid>>8;
					temp2=  PID[2].EEP_Pid;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PRESSURE_PID3_PID+1)
				{ 
					temp1=  controllers[2].value>>8;
					temp2=  controllers[2].value;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PRESSURE_PID3_PID+2)
				{ 
					temp1=  controllers[2].value>>24;
					temp2=  controllers[2].value>>16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PRESSURE_MODE_SELECT)
				{ 
					temp1=  0;
					temp2= mode_select;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_PASSWORD_ENABLE)
				{ 
					temp1= 0;
					temp2= use_password;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_PRESSURE_USER_PASSWORD0) && (address <= MODBUS_PRESSURE_USER_PASSWORD3))
				{
					temp1= 0;
					temp2= user_password[address - MODBUS_PRESSURE_USER_PASSWORD0];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRESSURE_BACKLIGHT_KEEP_SECONDS)
				{ 
					temp1= 0 ;
					temp2= backlight_keep_seconds ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if((address >= PRESSURE_TSTAT_NAME1) && (address <= PRESSURE_TSTAT_NAME10))  
				{
					u16 temp = address - PRESSURE_TSTAT_NAME1;  
					temp1= panelname[temp * 2];
					temp2= panelname[temp * 2 + 1];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}				
				
	//---------------------end pid ------------------------			
				else
				{
					temp1 = 0 ;
					temp2 = 0;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
 				
			}
			
			else if((address < MODBUS_PM25_END)&&((PRODUCT_ID == STM32_PM25)))
			{
				if(address == MODBUS_PM25_VAL)
				{ 
					temp1 = pm25_sensor.pm25 >>8;
					temp2 = pm25_sensor.pm25;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM10_VAL)
				{ 
					temp1 =pm25_sensor.pm10 >> 8 ;
					temp2 = pm25_sensor.pm10;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_AQI)
				{ 
					temp1 = pm25_sensor.AQI >> 8 ;
					temp2 = pm25_sensor.AQI;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM25_LEVEL)
				{ 
					temp1 =0 ;
					temp2 = pm25_sensor.level;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_SENSOR_ID)
				{ 
					temp1 =pm25_sensor.id>>8 ;
					temp2 = pm25_sensor.id;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_SENSOR_MODE)
				{ 
					temp1 =0 ;
					temp2 = pm25_sensor.mode;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_SENSOR_PERIOD)
				{ 
					temp1 =0 ;
					temp2 = pm25_sensor.period;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_SENSOR_STATUS)
				{ 
					temp1 =0 ;
					temp2 = pm25_sensor.status;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_SENSOR_CMD_STATUS)
				{ 
					temp1 =0 ;
					temp2 = pm25_sensor.cmd_status;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_MENU_SET)
				{ 
					temp1 =0 ;
					temp2 = pm25_sensor.menu.menu_set;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_SCROLL_SET)
				{ 
					temp1 =0 ;
					temp2 = pm25_sensor.menu.scroll_set;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_MENU_SWITCH_SECONDS)
				{ 
					temp1 =0 ;
					temp2 = pm25_sensor.menu.seconds;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PM25_OFFSET)
				{ 
					temp1 = pm25_sensor.pm25_offset >> 8 ;
					temp2 = pm25_sensor.pm25_offset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PM10_OFFSET)
				{ 
					temp1 = pm25_sensor.pm10_offset >> 8 ;
					temp2 = pm25_sensor.pm10_offset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM25_FILTER)
				{ 
					temp1 = 0 ;
					temp2 = pm25_sensor.PM25_filter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM10_FILTER)
				{ 
					temp1 = 0 ;
					temp2 = pm25_sensor.PM10_filter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM_AUTO_MANUAL)
				{ 
					temp1 = 0 ; 
					temp2 = pm25_sensor.auto_manual;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM25_RANGE_MIN)
				{
					temp1= HIGH_BYTE(output_range_table[CHANNEL_TEMP].min);
					temp2= LOW_BYTE(output_range_table[CHANNEL_TEMP].min);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM25_RANGE_MAX)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_TEMP].max);
					temp2= LOW_BYTE(output_range_table[CHANNEL_TEMP].max);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM10_RANGE_MIN)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_HUM].min);
					temp2= LOW_BYTE(output_range_table[CHANNEL_HUM].min) ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM10_RANGE_MAX)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_HUM].max);
					temp2= LOW_BYTE(output_range_table[CHANNEL_HUM].max);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM25_OUTPUT_MODE) 
				{ 
					temp1= 0 ;
					temp2= output_mode;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_PM25_PARTICLE_UINT)
				{
					temp1= 0 ;
					temp2= pm25_unit;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_PM25_WEIGHT_1_0)
				{
					temp1 = pm25_weight_10 >> 8 ;
					temp2 = pm25_weight_10;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}
				else if(address == MODBUS_PM25_WEIGHT_2_5)
				{
					temp1 = pm25_weight_25 >> 8 ;
					temp2 = pm25_weight_25;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}

				else if(address == MODBUS_PM25_WEIGHT_4_0)
				{
					temp1 = pm25_weight_40 >> 8 ;
					temp2 = pm25_weight_40;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}	

			  else if(address == MODBUS_PM25_WEIGHT_10)
				{
					temp1 = pm25_weight_100 >> 8 ;
					temp2 = pm25_weight_100;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}	
				else if(address == MODBUS_PM25_NUMBER_0_5)
				{
					temp1 = pm25_number_05 >> 8 ;
					temp2 = pm25_number_05;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}

				else if(address == MODBUS_PM25_NUMBER_1_0)
				{
					temp1 = pm25_number_10 >> 8 ;
					temp2 = pm25_number_10;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}

				else if(address == MODBUS_PM25_NUMBER_2_5)
				{
					temp1 = pm25_number_25 >> 8 ;
					temp2 = pm25_number_25;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}				

				else if(address == MODBUS_PM25_NUMBER_4_0)
				{
					temp1 = pm25_number_40 >> 8 ;
					temp2 = pm25_number_40;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}		

				else if(address == MODBUS_PM25_NUMBER_10)
				{
					temp1 = pm25_number_100 >> 8 ;
					temp2 = pm25_number_100;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}	
				else if(address == MODBUS_PM25_RANGE)
				{
					temp1 = 0;
					temp2 = pm25_sensor.pm25_range;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PM25_AREA)
				{
					temp1 = 0;
					temp2 = pm25_sensor.AQI_area;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if((address >= TSTAT_NAME1) && (address <= TSTAT_NAME10))  
				{
					u16 temp = address - TSTAT_NAME1;  
					temp1= panelname[temp * 2];
					temp2= panelname[temp * 2 + 1];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else
				{
					temp1 =0 ;
					temp2 = 0;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
			}		
			else //if((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
			{  
				if(address == MODBUS_TEMPERATURE_SENSOR_SELECT)
				{ 
					temp1 =0 ;
					temp2 = temperature_sensor_select  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_TEMPERATURE_DEGREE_C_OR_F) 
				{ 
					temp1 = 0 ;
					temp2 = deg_c_or_f  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_INTERNAL_TEMPERATURE_CELSIUS)
				{  
					temp1 = internal_temperature_c >> 8 ;
					temp2 = internal_temperature_c  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_INTERNAL_TEMPERATURE_FAHRENHEIT)
				{ 
					temp1 = internal_temperature_f >> 8 ;
					temp2 = internal_temperature_f  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_EXTERNAL_TEMPERATURE_CELSIUS) 	
				{  
					temp1 = HumSensor.temperature_c >> 8 ;
					temp2 = HumSensor.temperature_c  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_EXTERNAL_TEMPERATURE_FAHRENHEIT) 
				{ 
					temp1 = HumSensor.temperature_f >> 8 ;
					temp2 = HumSensor.temperature_f  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_TEMPERATURE_OFFSET_INTERNAL)
				{ 
					temp1 = internal_temperature_offset >> 8 ;
					temp2 = internal_temperature_offset  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUMIDITY)
				{  
					temp1 = HumSensor.humidity >> 8 ;
					temp2 = HumSensor.humidity  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUMIDITY_FREQUENCY)
				{  
					temp1 = HumSensor.frequency >> 8 ;
					temp2 = HumSensor.frequency  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}	
					
				else if(address == MODBUS_HUMIDITY_SENSOR_HEATING)
				{
					temp1 = 0;
					temp2 = hum_heat_status;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}	
					
				else if(address == MODBUS_CO2_INTERNAL_EXIST)
				{ 				
					
					temp1 = internal_co2_module_type >> 8 ;
					temp2 = internal_co2_module_type  ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_CO2_INTERNAL)
				{ 
					temp1= HIGH_BYTE(int_co2_str.co2_int);
					temp2= LOW_BYTE(int_co2_str.co2_int);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_CO2_INTERNAL_OFFSET)
				{ 
					temp1= HIGH_BYTE(int_co2_str.co2_offset);
					temp2= LOW_BYTE(int_co2_str.co2_offset);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_CO2_INTERNAL_PREALARM_SETPOINT)
				{ 
					temp1= HIGH_BYTE(int_co2_str.pre_alarm_setpoint) ;
					temp2=LOW_BYTE(int_co2_str.pre_alarm_setpoint);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_CO2_INTERNAL_ALARM_SETPOINT)
				{ 
					temp1= HIGH_BYTE(int_co2_str.alarm_setpoint);
					temp2= LOW_BYTE(int_co2_str.alarm_setpoint);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
//				else if((address >= MODBUS_CO2_EXTERNAL_START) && ( address < MODBUS_CO2_EXTERANL_END))
//				{ 
//					temp1= HIGH_BYTE(ext_co2_str[address - MODBUS_CO2_EXTERNAL_START].co2_int) ;
//					temp2= LOW_BYTE(ext_co2_str[address - MODBUS_CO2_EXTERNAL_START].co2_int) ;
//					sendbuf[send_cout++] = temp1 ;
//					sendbuf[send_cout++] = temp2 ;
//					crc16_byte(temp1);
//					crc16_byte(temp2);
//				}
//				else if((address >= MODBUS_CO2_EXTERNAL_OFFSET_START) && (address < MODBUS_CO2_EXTERNAL_OFFSET_END))
//				{ 
//					temp1= HIGH_BYTE(ext_co2_str[address - MODBUS_CO2_EXTERNAL_OFFSET_START].co2_offset);
//					temp2= LOW_BYTE(ext_co2_str[address - MODBUS_CO2_EXTERNAL_OFFSET_START].co2_offset);
//					sendbuf[send_cout++] = temp1 ;
//					sendbuf[send_cout++] = temp2 ;
//					crc16_byte(temp1);
//					crc16_byte(temp2);
//				}
//				else if((address >= MODBUS_CO2_EXTERNAL_PREALARM_SETPOINT_START) && (address < MODBUS_CO2_EXTERNAL_PREALARM_SETPOINT_END))
//				{ 
//					temp1= HIGH_BYTE(ext_co2_str[address - MODBUS_CO2_EXTERNAL_PREALARM_SETPOINT_START].pre_alarm_setpoint) ;
//					temp2= LOW_BYTE(ext_co2_str[address - MODBUS_CO2_EXTERNAL_PREALARM_SETPOINT_START].pre_alarm_setpoint);
//					sendbuf[send_cout++] = temp1 ;
//					sendbuf[send_cout++] = temp2 ;
//					crc16_byte(temp1);
//					crc16_byte(temp2);
//				}
//				else if((address >= MODBUS_CO2_EXTERNAL_ALARM_SETPOINT_START) && (i + address < MODBUS_CO2_EXTERNAL_ALARM_SETPOINT_END))
//				{ 
//					temp1= HIGH_BYTE(ext_co2_str[i + address - MODBUS_CO2_EXTERNAL_ALARM_SETPOINT_START].alarm_setpoint);
//					temp2= LOW_BYTE(ext_co2_str[i + address - MODBUS_CO2_EXTERNAL_ALARM_SETPOINT_START].alarm_setpoint);
//					sendbuf[send_cout++] = temp1 ;
//					sendbuf[send_cout++] = temp2 ;
//					crc16_byte(temp1);
//					crc16_byte(temp2);
//				}
				else if(address == MODBUS_CO2_SLOPE_DETECT_VALUE)
				{ 
					temp1= HIGH_BYTE(co2_slope_detect_value);
					temp2= LOW_BYTE(co2_slope_detect_value);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_CO2_FILTER)
				{ 
					temp1= 0;
					temp2= int_co2_filter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PASSWORD_ENABLE)
				{ 
					temp1= 0;
					temp2= use_password;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_USER_PASSWORD0) && (address <= MODBUS_USER_PASSWORD3))
				{
					temp1= 0;
					temp2= user_password[address - MODBUS_USER_PASSWORD0];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	//			else if(i + address == MODBUS_RTC_CENTURY)
	//			{
	//				main_send_byte(0, TRUE);
	//				main_send_byte(Modbus.Time.Clk.century, TRUE);
	//			}
				else if(address == MODBUS_RTC_YEAR)
				{
					temp1= HIGH_BYTE(calendar.w_year);
					temp2= LOW_BYTE(calendar.w_year);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address== MODBUS_RTC_MONTH)
				{
					temp1= 0;
					temp2= LOW_BYTE(calendar.w_month);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address== MODBUS_RTC_DAY)
				{
					temp1= 0;
					temp2= LOW_BYTE(calendar.w_date);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_RTC_WEEK)
				{
					temp1= 0;
					temp2= LOW_BYTE(calendar.week);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_RTC_HOUR)
				{
					temp1= 0;
					temp2= LOW_BYTE(calendar.hour);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_RTC_MINUTE)
				{
					temp1= 0;
					temp2= LOW_BYTE(calendar.min);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_RTC_SECOND)
				{
					temp1= 0;
					temp2= LOW_BYTE(calendar.sec);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_ALARM_AUTO_MANUAL)
				{ 
					temp1= 0 ;
					temp2= alarm_state;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRE_ALARM_SETTING_ON_TIME)
				{ 
					temp1= 0;
					temp2= pre_alarm_on_time;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PRE_ALARM_SETTING_OFF_TIME)
				{ 
					temp1= 0;
					temp2= pre_alarm_off_time;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_ALARM_DELAY_TIME)
				{ 
					temp1= 0;
					temp2= alarm_delay_time;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_AUTO_MANUAL)
				{ 
					temp1= 0;
					temp2= output_auto_manual;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_MANUAL_VALUE_TEM)
				{ 
					temp1= HIGH_BYTE(output_manual_value_temp) ;
					temp2= LOW_BYTE(output_manual_value_temp);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_MANUAL_VALUE_HUM)
				{ 
					temp1= HIGH_BYTE(output_manual_value_humidity);
					temp2= LOW_BYTE(output_manual_value_humidity);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_MANUAL_VALUE_CO2)
				{ 
					temp1= HIGH_BYTE(output_manual_value_co2);
					temp2= LOW_BYTE(output_manual_value_co2);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_OUTPUT_MODE) 
				{ 
					temp1= 0 ;
					temp2= output_mode;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_RANGE_MIN_TEM) 
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_TEMP].min);
					temp2= LOW_BYTE(output_range_table[CHANNEL_TEMP].min);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_RANGE_MAX_TEM)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_TEMP].max);
					temp2= LOW_BYTE(output_range_table[CHANNEL_TEMP].max);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_RANGE_MIN_HUM)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_HUM].min);
					temp2= LOW_BYTE(output_range_table[CHANNEL_HUM].min) ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_RANGE_MAX_HUM)
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_HUM].max);
					temp2= LOW_BYTE(output_range_table[CHANNEL_HUM].max);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address == MODBUS_OUTPUT_RANGE_MIN_CO2)||(address == MODBUS_DEW_PT_MIN))
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_CO2].min);
					temp2= LOW_BYTE(output_range_table[CHANNEL_CO2].min);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address == MODBUS_OUTPUT_RANGE_MAX_CO2)||(address == MODBUS_DEW_PT_MAX))
				{ 
					temp1= HIGH_BYTE(output_range_table[CHANNEL_CO2].max);
					temp2= LOW_BYTE(output_range_table[CHANNEL_CO2].max);
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	//			else if(address == MODBUS_MENU_BLOCK_SECONDS)
	//			{
	//				main_send_byte(0, TRUE);
	//				main_send_byte(menu_block_seconds, TRUE);
	//			}
				else if(address == MODBUS_BACKLIGHT_KEEP_SECONDS)
				{ 
					temp1= 0 ;
					temp2= backlight_keep_seconds ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_EXTERNAL_NODES_PLUG_AND_PLAY)
				{ 
					temp1= 0 ;
					temp2= external_nodes_plug_and_play ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_SCAN_DB_CTR)
				{ 
					temp1= 0 ;
					temp2= db_ctr;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_RESET_SCAN_DB)
				{ 
					temp1= 0;
					temp2= reset_scan_db_flag;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_SCAN_START) && (address < MODBUS_SCAN_END))
				{
					uint8 A, B;
					A = (i + address - MODBUS_SCAN_START) / SCAN_DB_SIZE;
					B = (i + address - MODBUS_SCAN_START) % SCAN_DB_SIZE;
					temp1 = 0;
					switch(B)
					{
						case 0:
							temp2 = scan_db[A].id ;  
							break;
						case 1:
							temp2 = (uint8)(scan_db[A].sn >> 0) ;
							break;
						case 2:
							temp2 = (uint8)(scan_db[A].sn >> 8) ;
							break;
						case 3:
							temp2 = (uint8)(scan_db[A].sn >> 16);
							break;
						case 4:
							temp2 = (uint8)(scan_db[A].sn >> 24);
							break;
					} 
					
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_GET_NODES_PARA_START) && (address < MODBUS_GET_NODES_PARA_END))
				{ 
					temp1= 0;
					temp2= get_para[i + address - MODBUS_GET_NODES_PARA_START] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_SCAN_OCCUPY_START) && (address < MODBUS_SCAN_OCCUPY_END))
				{ 
					temp1= 0;
					temp2= db_occupy[i + address - MODBUS_SCAN_OCCUPY_START] ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_SCAN_ONLINE_START) && (address < MODBUS_SCAN_ONLINE_END))
				{ 
					temp1= 0;
					temp2= db_online[i + address - MODBUS_SCAN_ONLINE_START];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_INT_TEMPRATURE_FILTER) 
				{
					temp1= 0;
					temp2= Temperature_Filter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_EXT_TEMPRATURE_FILTER) 
				{ 
					temp1= 0;
					temp2= HumSensor.T_Filter ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_HUIDITY_FILTER) 
				{ 
					temp1= 0;
					temp2= HumSensor.H_Filter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if((address == MODBUS_HUM_VERSION)||(address == MODBUS_HUM_VERSION_CP))
				{ 
					temp1= 0;
					temp2= humidity_version;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}
				else if(address == MODBUS_HUM_SN)
				{ 
					temp1= HumSensor.sn>>8;
					temp2= HumSensor.sn;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);				
				}
				else if(address == MODBUS_CAL_FAC_PTS)
				{ 
					temp1= HumSensor.counter>>8;
					temp2= HumSensor.counter;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_TABLE_SEL)
				{
					temp1= 0;
					temp2= table_sel;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	 
				else if((address >= MODBUS_HUMCOUNT1_H)&&(address<= MODBUS_HUMRH10_H))
				{
					uint8 temp,i,j;
					temp = address - MODBUS_HUMCOUNT1_H;
					i = temp /2;
					j = temp %2;
					if(j==0)
					{	
						temp1 = read_eeprom(HUMCOUNT1_H + i*4 + 1);	
						temp2 = read_eeprom(HUMCOUNT1_H + i*4);     
						 
					}
					else
					{	
						temp1 = read_eeprom(HUMRH1_H + i*4 + 1);	
						temp2 = read_eeprom(HUMRH1_H + i*4);     
						
					} 
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_USER_POINTS)
				{ 
					temp1= 0;
					temp2= hum_size_copy ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= MODBUS_USER_RH1)&&(address<= MODBUS_USER_FRE10))
				{
					uint8 temp,i,j;
					temp = address - MODBUS_USER_RH1;
					i = temp /2;
					j = temp %2;
					if(j==1)
					{	
						temp1 = read_eeprom(EEP_USER_FRE1 + i*4 + 1) ;	
						temp2 = read_eeprom(EEP_USER_FRE1 + i*4) ;      
					}
					else
					{	
						temp1 = read_eeprom(EEP_USER_RH1 + + i*4 + 1);	
						temp2 = read_eeprom(EEP_USER_RH1 + i*4) ;      
					} 
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_DEW_PT)
				{ 
					temp1= HumSensor.dew_pt >> 8;
					temp2= HumSensor.dew_pt;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_DEW_PT_F)
				{ 
					temp1= HumSensor.dew_pt_F>>8;
					temp2= HumSensor.dew_pt_F ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_DIS_INFO)
				{ 
					temp1=  0;
					temp2=  dis_hum_info;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_SEL)
				{ 
					temp1=  0;
					temp2=  analog_output_sel;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PWS)
				{ 	
					temp1= HumSensor.Pws>>8 ;
					temp2= HumSensor.Pws ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_MIX_RATIO)
				{ 
					temp1= HumSensor.Mix_Ratio>>8;
					temp2= HumSensor.Mix_Ratio ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_ENTHALPY)
				{ 
					temp1= HumSensor.Enthalpy>>8;
					temp2= HumSensor.Enthalpy ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				  
				
				else if(address == MODBUS_TEMP_OFFSET)
				{ 
					temp1=  HumSensor.offset_t >> 8;
					temp2=  HumSensor.offset_t;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_HUM_OFFSET)
				{ 
					temp1= HumSensor.offset_h>>8;
					temp2= HumSensor.offset_h ;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_CAL_DEFAULT_HUM)
				{ 	
					temp1= HumSensor.offset_h_default>>8;
					temp2= HumSensor.offset_h_default;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_K_LINE)
				{
					int16 itemp;
					itemp = k_line * 1000;   
					temp1= itemp >> 8;
					temp2=  itemp;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_B_LINE)
				{  
					temp1= (int16)b_line >> 8;
					temp2=  (int8)b_line;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 			
				else if(address == MODBUS_REPLY_DELAY)
				{ 
					temp1=  0;
					temp2=  reply_delay_time;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_RECEIVE_DELAY)
				{ 
					temp1=  0;
					temp2=  receive_delay_time;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_UART1_PARITY)
				{ 
					temp1=  0;
					temp2=  uart1_parity;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_AUTO_HEAT_CONTROL)
				{ 
					temp1=  0;
					temp2=  auto_heat_enable;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_ORIGINAL_HUM)
				{ 
					temp1=  HumSensor.org_hum>>8;
					temp2=  HumSensor.org_hum;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
	//-----------------------------pid1-------------------------//			
				else if(address == MODBUS_PID1_MODE)
				{ 
					temp1=  0;
					temp2=  controllers[0].action;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PID1_SETPOINT)
				{ 
					temp1=  PID[0].EEP_SetPoint >>8;
					temp2=  PID[0].EEP_SetPoint;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PID1_PTERM)
				{ 
					temp1=  0;
					temp2=  controllers[0].proportional;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PID1_ITERM)
				{ 
					temp1=  0;
					temp2=  controllers[0].reset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PID1_PID)
				{ 
					temp1=  PID[0].EEP_Pid>>8;
					temp2=  PID[0].EEP_Pid;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_PID1_PID+1)
				{ 
					temp1=  controllers[0].value>>8;
					temp2=  controllers[0].value;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PID1_PID+2)
				{ 
					temp1=  controllers[0].value>>24;
					temp2=  controllers[0].value>>16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
	//-----------------------------pid2-------------------------//			
				else if(address == MODBUS_PID2_MODE)
				{ 
					temp1=  0;
					temp2=  controllers[1].action;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PID2_SETPOINT)
				{ 
					temp1=  PID[1].EEP_SetPoint >>8;
					temp2=  PID[1].EEP_SetPoint;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PID2_PTERM)
				{ 
					temp1=  0;
					temp2=  controllers[1].proportional;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PID2_ITERM)
				{ 
					temp1=  0;
					temp2=  controllers[1].reset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PID2_PID)
				{ 
					temp1=  PID[1].EEP_Pid>>8;
					temp2=  PID[1].EEP_Pid;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PID2_PID+1)
				{ 
					temp1=  controllers[1].value>>8;
					temp2=  controllers[1].value;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PID2_PID+2)
				{ 
					temp1=  controllers[1].value>>24;
					temp2=  controllers[1].value>>16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
	//-----------------------------pid3-------------------------//			
				else if(address == MODBUS_PID3_MODE)
				{ 
					temp1=  0;
					temp2=  controllers[2].action;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PID3_SETPOINT)
				{ 
					temp1=  PID[2].EEP_SetPoint >>8;
					temp2=  PID[2].EEP_SetPoint;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				 
				else if(address == MODBUS_PID3_PTERM)
				{ 
					temp1=  0;
					temp2=  controllers[2].proportional;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PID3_ITERM)
				{ 
					temp1=  0;
					temp2=  controllers[2].reset;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_PID3_PID)
				{ 
					temp1=  PID[2].EEP_Pid>>8;
					temp2=  PID[2].EEP_Pid;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PID3_PID+1)
				{ 
					temp1=  controllers[2].value>>8;
					temp2=  controllers[2].value;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_PID3_PID+2)
				{ 
					temp1=  controllers[2].value>>24;
					temp2=  controllers[2].value>>16;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				} 
				else if(address == MODBUS_MODE_SELECT)
				{ 
					temp1=  0;
					temp2= mode_select;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_CO2_DISABLE_AUTOCAL)
				{
					temp1=  0;
					temp2= co2_autocal_disable;	
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);					
				}
				
				else if(address == MODBUS_CO2_CAL_MHZ19)
				{
					temp1 = mhz19_cal_h;
					temp2 = mhz19_cal_l;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				#if 1//defined (COLOR_SCREEN)
				else if(address == MODBUS_SCREEN_AREA_1)
				{
					temp1 = 0;
					temp2 = screenArea1;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_SCREEN_AREA_2)
				{
					temp1 = 0;
					temp2 = screenArea2;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_SCREEN_AREA_3)
				{
					temp1 = 0;
					temp2 = screenArea3;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_ENABLE_SCROLL)
				{
					temp1 = 0;
					temp2 = enableScroll;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_ALARM_ENABLE)
				{
					temp1 = 0;
					temp2 = alarmEnable;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				#endif
				else if(address == CO2_FRC_GET)
				{
					temp1 = co2_asc>>8;
					temp2 = co2_asc & 0xff;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == CO2_FRC_VALUE)
				{
					temp1 = co2_frc>>8;
					temp2 = co2_frc & 0xff;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
	//---------------------end pid ------------------------			
				
				
				else if(address == MODBUS_AD_OUTPUT_FB_CO2)
				{  
					temp1= analog_input[CHANNEL_CO2] >> 8;
					temp2= analog_input[CHANNEL_CO2];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_AD_OUTPUT_FB_TEMP)
				{  
					temp1= analog_input[CHANNEL_TEMP]>> 8;
					temp2= analog_input[CHANNEL_TEMP];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_AD_OUTPUT_FB_HUM)
				{  
					temp1= analog_input[CHANNEL_HUM]>> 8;
					temp2= analog_input[CHANNEL_HUM];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_AD_INTERNAL_TEMP)
				{  
					temp1= internal_temp_ad>> 8;
					temp2= internal_temp_ad;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_OUTPUT_CO2)
				{  
					temp1= analog_output[CHANNEL_CO2] >> 8;
					temp2= analog_output[CHANNEL_CO2];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address ==  MODBUS_HUM_SENSOR_STATE)
				{  
					temp1= 0;
					temp2= display_state;
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_TEMP)
				{  
					temp1= analog_output[CHANNEL_TEMP] >> 8;
					temp2= analog_output[CHANNEL_TEMP];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_HUM)
				{  
					temp1= analog_output[CHANNEL_HUM] >> 8;
					temp2= analog_output[CHANNEL_HUM];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_HUM_VOL_OFFSET)
				{ 	
					temp1= output_offset[0][CHANNEL_HUM] >> 8;
					temp2= output_offset[0][CHANNEL_HUM];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if(address == MODBUS_OUTPUT_TEMP_VOL_OFFSET)
				{ 	
					temp1= output_offset[0][CHANNEL_TEMP] >> 8;
					temp2= output_offset[0][CHANNEL_TEMP];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);	
				}
				else if(address == MODBUS_OUTPUT_CO2_VOL_OFFSET)
				{ 	
					temp1= output_offset[0][CHANNEL_CO2] >> 8;
					temp2= output_offset[0][CHANNEL_CO2];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				
				else if(address == MODBUS_OUTPUT_HUM_CUR_OFFSET)
				{ 	
					temp1= output_offset[1][CHANNEL_HUM] >> 8;
					temp2= output_offset[1][CHANNEL_HUM];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);	
				}
				else if(address == MODBUS_OUTPUT_TEMP_CUR_OFFSET)
				{ 	
					temp1= output_offset[1][CHANNEL_TEMP] >> 8;
					temp2= output_offset[1][CHANNEL_TEMP];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);	
				}
				else if(address == MODBUS_OUTPUT_CO2_CUR_OFFSET)
				{ 	
					temp1= output_offset[1][CHANNEL_CO2] >> 8;
					temp2= output_offset[1][CHANNEL_CO2];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else if((address >= TSTAT_NAME1) && (address <= TSTAT_NAME10))  
				{
					u16 temp = address - TSTAT_NAME1;  
					temp1= panelname[temp * 2];
					temp2= panelname[temp * 2 + 1];
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}
				else
				{
					temp1 = 0 ;
					temp2 =  0; 
					sendbuf[send_cout++] = temp1 ;
					sendbuf[send_cout++] = temp2 ;
					crc16_byte(temp1);
					crc16_byte(temp2);
				}				
			}  
			

		}//end of number
		temp1 = CRChi ;
		temp2 =  CRClo; 
		sendbuf[send_cout++] = temp1 ;
		sendbuf[send_cout++] = temp2 ;
		
		if(type == 4)
		{
			 memcpy(&bacnet_to_modbus,&sendbuf[3],RegNum * 2);
		}
		else	if(type == 0)
		{
			memcpy(uart_send, sendbuf, send_cout);
			USART_SendDataString(send_cout);
		}
		else
		{
				sendbuf[0] = pData[0] ;
				sendbuf[1] = pData[1] ;
				sendbuf[2] = 0 ;
				sendbuf[3] = 0 ;
				sendbuf[4] = (3 + RegNum * 2) >> 8; 
				sendbuf[5] =(u8)(3 + RegNum * 2) ;
				memcpy(tcp_server_sendbuf,sendbuf,RegNum * 2 + 3 + HeadLen);
				tcp_server_sendlen = RegNum * 2 + 3 + HeadLen;
		}
	}

	
	else if (USART_RX_BUF[1] == CHECKONLINE)
	{
		// send first byte of information
			
			temp2 =  pData[HeadLen+0]; 
			uart_send[send_cout++] = temp2 ;
			crc16_byte(temp2);
		

			temp2 = pData[HeadLen+1]; 
			uart_send[send_cout++] = temp2 ;
			crc16_byte(temp2);
		

			temp2 =  modbus.address; 
			uart_send[send_cout++] = temp2 ;
			crc16_byte(temp2);
			

			temp2 =  modbus.serial_Num[0]; 
			uart_send[send_cout++] = temp2 ;
			crc16_byte(temp2);
			
			temp2 =  modbus.serial_Num[1]; 
			uart_send[send_cout++] = temp2 ;
			crc16_byte(temp2);
			

			temp2 =  modbus.serial_Num[2]; 
			uart_send[send_cout++] = temp2 ;
			crc16_byte(temp2);
			

			temp2 =  modbus.serial_Num[3]; 
			uart_send[send_cout++] = temp2 ;
			crc16_byte(temp2);
			
			temp2 =  CRChi; 
			uart_send[send_cout++] = temp2 ;
			temp2 =  CRClo; 
			uart_send[send_cout++] = temp2 ;
			USART_SendDataString(send_cout);
	}
}
u8 checkData(u16 address)
{
	//static unsigned char xdata rand_read_ten_count = 0 ;
	u16 crc_val;
	u8 minaddr,maxaddr, variable_delay;
	u8 i;
	static u8 srand_count =0 ;
	srand_count ++ ;
	// check if packet completely received
	if(revce_count != rece_size)
		return 0;

	// check if talking to correct device ID
	if(USART_RX_BUF[0] != 255 && USART_RX_BUF[0] != modbus.address && USART_RX_BUF[0] != 0)
		return 0;
 
	//  --- code to verify what is on the network ---------------------------------------------------
	if( USART_RX_BUF[1] == CHECKONLINE)
	{

	crc_val = crc16(USART_RX_BUF,4) ;
	if(crc_val != (USART_RX_BUF[4]<<8) + USART_RX_BUF[5] )
	{
		return 0;
	}
	minaddr = (USART_RX_BUF[2] >= USART_RX_BUF[3] ) ? USART_RX_BUF[3] : USART_RX_BUF[2] ;	
	maxaddr = (USART_RX_BUF[2] >= USART_RX_BUF[3] ) ? USART_RX_BUF[2] : USART_RX_BUF[3] ;
	if(modbus.address < minaddr || modbus.address > maxaddr)
		return 0;
	else
	{	// in the TRUE case, we add a random delay such that the Interface can pick up the packets
		srand(srand_count);
		variable_delay = rand() % 20;
		
		for ( i=0; i<variable_delay; i++)
			delay_ms(5);
		return 1;
	}

	}
	// ------------------------------------------------------------------------------------------------------



	// check that message is one of the following
	if( (USART_RX_BUF[1]!=READ_VARIABLES) && (USART_RX_BUF[1]!=WRITE_VARIABLES) && (USART_RX_BUF[1]!=MULTIPLE_WRITE) )
		return 0;
	// ------------------------------------------------------------------------------------------------------
		// ------------------------------------------------------------------------------------------------------
		
	if(USART_RX_BUF[2]*256 + USART_RX_BUF[3] ==  MODBUS_ADDRESS_PLUG_N_PLAY)
	{
		if(USART_RX_BUF[1] == WRITE_VARIABLES)
		{
			if(USART_RX_BUF[6] != modbus.serial_Num[0]) 
			return FALSE;
			if(USART_RX_BUF[7] != modbus.serial_Num[1]) 
			return FALSE;
			if(USART_RX_BUF[8] != modbus.serial_Num[2])  
			return FALSE;
			if(USART_RX_BUF[9] != modbus.serial_Num[3]) 
			return FALSE;
		}
		if (USART_RX_BUF[1] == READ_VARIABLES)
		{
			randval = rand() % 5 ;
		}
		if(randval != RESPONSERANDVALUE)
		{

				return FALSE;
		}
		else
		{	// in the TRUE case, we add a random delay such that the Interface can pick up the packets
			variable_delay = rand() % 20;
			
			for ( i=0; i<variable_delay; i++)
				delay_ms(5);
		}
		
	}

	// if trying to write the Serial number, first check to see if it has been already written
	// note this does not take count of multiple-write, thus if try to write into those reg with multiple-write, command will accept
	if( (USART_RX_BUF[1]==WRITE_VARIABLES)  && (address<= MODBUS_HARDWARE_REV) )
	{
		// Return false if trying to write SN Low word that has already been written
		if(USART_RX_BUF[3] < 2)
		{
			if(modbus.SNWriteflag & 0x01)                // low byte of SN writed
				return FALSE;
		}
		// Return false if trying to write SN High word that has already been written
		else if (USART_RX_BUF[3] < 4)
		{
			if(modbus.SNWriteflag  & 0x02)                 // high byte of SN writed
				return FALSE;
		}
		else if (USART_RX_BUF[3] ==  MODBUS_HARDWARE_REV)
		{
			if(modbus.SNWriteflag  & 0x04)                 // hardware byte writed
				return FALSE;
		}
		else if (USART_RX_BUF[3] ==  MODBUS_PRODUCT_MODEL)
		{
			if(modbus.SNWriteflag  & 0x08)                 // hardware byte writed
				return FALSE;
		}

	}


	crc_val = crc16(USART_RX_BUF, rece_size-2);

	if(crc_val == (USART_RX_BUF[rece_size-2]<<8) + USART_RX_BUF[rece_size-1] )
	{
		return 1;
	}
	else
	{
		return 0;
	}
	//return TRUE;

 }

 
 
 void dealwithData(void)
{	
	u16 address;
	// given this is used in multiple places, decided to put it as an argument
	address = (u16)(USART_RX_BUF[2]<<8) + USART_RX_BUF[3];
	if (checkData(address))
	{		
//		// Initialize tranmission
		initSend_COM();	
		// Initialize CRC
		init_crc16();		

//		// Respond with any data requested
		responseCmd(0,USART_RX_BUF);
//		// Store any data being written
		internalDeal(0, USART_RX_BUF);
		uart.tx_count++;
		if(uart.tx_count > 99) uart.tx_count = 0;
	}
	else
	{
		serial_restart();
	}
}


void modbus_data_cope(u8 XDATA* pData, u16 length, u8 conn_id)
{
	
}

void SoftReset(void)
{
	__set_FAULTMASK(1);      // 关闭所有中断
	NVIC_SystemReset();      // 复位
}

void stack_detect(u16 *p)
{ 
//	unsigned portBASE_TYPE temp;
	*p = uxTaskGetStackHighWaterMark(NULL); 
}
