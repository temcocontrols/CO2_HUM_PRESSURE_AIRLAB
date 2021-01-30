#include "stm32f10x.h"
#include "bsp_esp8266.h"
#include "types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "modbus.h"
#include "define.h"
#include "inputs.h"
//#include "../output/output.h"
#include "dlmstp.h"
#include "rs485.h"
#include "datalink.h"
//#include "config.h"
#include "handlers.h"
#include "device.h"	
#include "registerlist.h"
#include "types.h"
#include "modbus.h"
#include "bacnet.h"
#include "ProductModel.h"
#include "stmflash.h"
#include "bsp_esp8266.h"
#include "delay.h"
#include "store.h"
#include "24cxx.h"
#include "LCD.h"

extern u16 Test[50];
char * wifi_itoa( int value, char *string, int radix );	

char Get_SSID_RSSI(void);
void connect_AP(void);
uint8 flag_wifi;
extern uint8_t panelname[21] ;

#if WIFITEST
uint8 flag_set_wifi;

uint8_t bacnet_wifi_buf[500];
uint16_t bacnet_wifi_len;
uint8_t modbus_wifi_buf[500];
uint16_t modbus_wifi_len;
uint8_t packet[1024];
uint8_t cStr [ 1024 ] = { 0 };
STR_SSID	SSID_Info;
extern uint8 state;

void check_linkStatus(void)
{
	char i;
	for(i = 0;i < 10;i++)
	{	
		SSID_Info.IP_Wifi_Status = ESP8266_Get_LinkStatus();
		if(	(SSID_Info.IP_Wifi_Status < 4) && (SSID_Info.IP_Wifi_Status > 0))
		{
			break;	
		}
		else
			delay_ms(500);
		
		IWDG_ReloadCounter();
	}	
	if(SSID_Info.IP_Wifi_Status == 3) // 已建立连接
	{
		ESP8266_Cmd ( "AT+CIPCLOSE=5", "OK", 0, 500 );
	}
}

uint8_t get_ip;

void Restore_WIFI(void)
{
	char count;
	count = 0;
	while((ESP8266_Cmd ( "AT+RESTORE", "OK", 0, 1000) == 0) && (count++ < 20))	
	{		
		IWDG_ReloadCounter();
		delay_ms(100);
	}
}

void Start_Smart_Config(void)
{
	char count;
	count = 0;
	while((ESP8266_Cmd ( "AT+CWSTARTSMART", "OK", 0, 100) == 0) && (count++ < 20))	
	{		
		IWDG_ReloadCounter();
		delay_ms(100);
	}	
}

void Stop_Smart_Config(void)
{
	char count;
	count = 0;
	while((ESP8266_Cmd ( "AT+CWSTOPSMART", "OK", 0, 100) == 0) && (count++ < 20))	
	{		
		IWDG_ReloadCounter();
		delay_ms(100);
	}	
}

char flag_start_smart = 0;
uint8 flag_connect_AP;
void Check_connect_AP(void)
{	
	if(flag_connect_AP == 1)
	{
		connect_AP();
		SoftReset();
		flag_connect_AP = 0;
	}
}

void connect_AP(void)
{
	char count;
	get_ip = 0;
	if(SSID_Info.IP_Wifi_Status == WIFI_NO_WIFI)		
		return;
	if(SSID_Info.MANUEL_EN != 0)
	{
		flag_start_smart = 0;
		ESP8266_Cmd( "AT+CWSTOPSMART", "OK", 0, 500 );
		delay_ms(200);
	}
	if(SSID_Info.IP_Auto_Manual == 1)	
	{
		count = 0;
		ESP8266_CIPSTA_DEF();
		delay_ms(5000);
		count = 0;
		ESP8266_Cmd ( "AT+RST", "OK", "ready", 1000 );
		delay_ms(200);
		get_ip = 0;
	}
	if(SSID_Info.MANUEL_EN == 1 /*&& SSID_Info.IP_Auto_Manual == 1*/)	
	{		
	
		ESP8266_Net_Mode_Choose ( STA );
		delay_ms(200);
		ESP8266_JoinAP(SSID_Info.name,SSID_Info.password);
		//ESP8266_JoinAP("TEMCO_TEST_2.4G","Travel321");
		//需要等待
		count = 0;
		while(count++ < 5)
		{
			memcpy(cStr,ESP8266_ReceiveString(DISABLE),1024);
			if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "OK" ) )
			{
				break;
			}
			delay_ms(1000);
		}
	}
	
	check_linkStatus();

}
	

void vWifitask( void *pvParameters )
{
	uint8 i;
	uint8_t ip[4];
	uint16_t count;
	char overtime = 20;
	char ucID;
	char ATret;
	uint16_t packet_len;
//	static uint8 get_ip = 0;
//	uint8 link_value = 0;	
	char tcp_port[5];
	uint16_t count_checkip;
	flag_start_smart = 0;
	flag_connect_AP = 0;
	/* intial SSID, read value form E2PROM */
	SSID_Info.rev = 4;
	flag_set_wifi = 0;
//	SSID_Info.IP_Auto_Manual = read_eeprom(WIFI_IP_AM);
//	if(SSID_Info.IP_Auto_Manual = 0xff)
//	{
//		SSID_Info.IP_Auto_Manual = 0;
//		write_eeprom(WIFI_IP_AM,0);
//	}
//	SSID_Info.modbus_port = 
//		read_eeprom(WIFI_MODBUS_PORT) + 256L * read_eeprom(WIFI_MODBUS_PORT + 1);	

	
	if(SSID_Info.modbus_port == 0 || SSID_Info.modbus_port == 0xffff)
	{
		SSID_Info.modbus_port = 502;
	}
	if((SSID_Info.name[0] == 0xff) && (SSID_Info.name[1] == 0xff))
	{
		memset(SSID_Info.name,0,64);
	}
	if((SSID_Info.password[0] == 0xff) && (SSID_Info.password[1] == 0xff))
	{
		memset(SSID_Info.password,0,32);
	}

	flag_connect_AP = 0;
	
		
	if(SSID_Info.bacnet_port == 0 || SSID_Info.bacnet_port == 0xffff)
	{
		SSID_Info.bacnet_port = 47808;
	}

	wifi_itoa(SSID_Info.modbus_port,tcp_port,10);
	
	ESP8266_Init();	
	delay_ms(2000);

	ESP8266_Rst();
	delay_ms(2000);

	ATret = ESP8266_AT_Test();	
	if(ATret != 2)  // not response OK
	{
		SSID_Info.IP_Wifi_Status = WIFI_NO_WIFI; // no wifi
	}
	else
	{
		uint8_t temp[7];
		temp[0] = AT24CXX_ReadOneByte(EEP_WRITE_WIFI_MAC);

		if(temp[0] != 0x44)
		{
			temp[1] = 0x18;
			temp[2] = 0xfe;
			if(((modbus.serial_Num[0] == 0) && (modbus.serial_Num[1] == 0) && (modbus.serial_Num[2] == 0) && (modbus.serial_Num[3] == 0))
				|| ((modbus.serial_Num[0] == 0xff) && (modbus.serial_Num[1] == 0xff) && (modbus.serial_Num[2] == 0xff) && (modbus.serial_Num[3] == 0xff)))
			{
				// do nothing
			}
			else
			{
				if(modbus.serial_Num[0] < 0x10)
					temp[3] = modbus.serial_Num[0] + 0x10;
				else
					temp[3] = modbus.serial_Num[0];
				
				if(modbus.serial_Num[1] < 0x10)
					temp[4] = modbus.serial_Num[1] + 0x10;
				else
					temp[4] = modbus.serial_Num[1];
				
				if(modbus.serial_Num[2] < 0x10)
					temp[5] = modbus.serial_Num[2] + 0x10;
				else
					temp[5] = modbus.serial_Num[2];
				
				if(modbus.serial_Num[3] < 0x10)
					temp[6] = modbus.serial_Num[3] + 0x10;
				else
					temp[6] = modbus.serial_Num[3];			
				
				ESP8266_Set_MAC(&temp[1]);			
				
				flag_set_wifi = 1;		
			}
		}
		else
			ESP8266_Get_MAC(SSID_Info.mac_addr);
		SSID_Info.IP_Wifi_Status = WIFI_NO_CONNECT;
		ESP8266_Net_Mode_Choose( STA );
		connect_AP();
	}
	for(;;)
	{	
		delay_ms(5) ;
		IWDG_ReloadCounter();
		
		
		//if(isWifiExist)
		
		if(ATret == 2)
		{
			if(flag_set_wifi == 1)
			{
				ESP8266_Get_MAC(SSID_Info.mac_addr);
				flag_set_wifi = 0;
				AT24CXX_WriteOneByte(EEP_WRITE_WIFI_MAC,0x44);
			}
		}
		if(ATret != 2)  // not response OK
			continue;
		if((SSID_Info.IP_Wifi_Status != WIFI_NORMAL) && (SSID_Info.IP_Wifi_Status != WIFI_NO_WIFI ))//go into smart config mode
			//go into smart config mode
		{
			if(SSID_Info.IP_Auto_Manual == 0)
			{
				SSID_Info.ip_addr[0] = 0;
				SSID_Info.ip_addr[1] = 0;
				SSID_Info.ip_addr[2] = 0;
				SSID_Info.ip_addr[3] = 0;
			}
			if(flag_start_smart == 0)
			{
				if(ESP8266_Cmd ("AT+CWSTARTSMART", "OK", 0, 500 ) )
					flag_start_smart = 1;
			}
			if(flag_start_smart)
			{						
					count = 0;	
					do
					{
						SSID_Info.IP_Wifi_Status = ESP8266_Get_LinkStatus();
						delay_ms(100);
						IWDG_ReloadCounter();
					}while(SSID_Info.IP_Wifi_Status > 3 && count++ < 200);
					
					
					if(count>= 200)
					{
						connect_AP();						
						continue;
					}
					
					
					// Clear SSID
					// 一旦通过smartconfig方式配置SSID_Info.MANUEL_EN要设为0					
					SSID_Info.MANUEL_EN = 0;
					memset(SSID_Info.name,0,64);
					memset(SSID_Info.password,0,32);
					write_page_en[WIFI_TYPE] = 1; 
					Flash_Write_Mass();
					ESP8266_Cmd( "AT+CWSTOPSMART", "OK", 0, 500 );
					delay_ms(100);				
					
			}
			delay_ms(200) ;
		}
		else
		{
			if(!get_ip)
			{
				if(ESP8266_Inquire_ApIp(SSID_Info.mac_addr,SSID_Info.ip_addr,60))
				{
					if(SSID_Info.ip_addr[0] == 0 && SSID_Info.ip_addr[1] == 0
						&& SSID_Info.ip_addr[2] == 0 && SSID_Info.ip_addr[3] == 0)
						get_ip = 0;
					else
						get_ip = 1;
				}
				ESP8266_CIPSTA_CUR(0); 
				
				ESP8266_Cmd( "AT+CIPMODE=0", "OK", 0, 500 );
				
				ESP8266_Enable_MultipleId ( ENABLE );
				IWDG_ReloadCounter();
				
				ESP8266_Cmd( "AT+CIPCLOSE=5", "OK", 0, 500 );
				
				count = 0;
				while ((!ESP8266_Link_UDP("255.255.255.255",SSID_Info.bacnet_port,SSID_Info.bacnet_port,2,UCID_BACNET)) && (count++ < 5))
					IWDG_ReloadCounter();
				count = 0;
				while ((!ESP8266_Link_UDP("255.255.255.255",1234,1234,2,UCID_SCAN)) && (count++ < 5))
					IWDG_ReloadCounter();		
				count = 0;				
				while ((!ESP8266_StartOrShutServer(ENABLE,tcp_port,&overtime)) && (count++ < 5)) 
					IWDG_ReloadCounter();

			}
			else
			{
				memset(packet,0,1024);
				memset(cStr,0,1024);
				packet_len = 0;
				memcpy(cStr,ESP8266_ReceiveString(DISABLE),1024);
				ucID = cStr[7] - '0';
				packet_len = check_packet(cStr,packet);
				if(packet_len > 0)
				{
					if(ucID >= 2 && ucID <= 4)  // modbus TCP 502
					{
				// check modbus data
						if((packet[0] == 0xee) && (packet[1] == 0x10) &&
						(packet[2] == 0x00) && (packet[3] == 0x00) &&
						(packet[4] == 0x00) && (packet[5] == 0x00) &&
						(packet[6] == 0x00) && (packet[7] == 0x00) )
						{		
				//			Udtcp_server_databuf(0);
				//			send_flag = 1;
				//			update_firmware = 1;
							AT24CXX_WriteOneByte(EEP_UPDATE_STATUS, 127);
							SoftReset();
						}
						else if(packet[6] ==  modbus.address 
						|| ((packet[6] == 255) && (packet[7] != 0x19)))
						{
							responseCmd(WIFI, packet);
							internalDeal(WIFI, packet);
							if(modbus_wifi_len > 0)
							{
								ESP8266_SendString ( DISABLE, (uint8_t *)&modbus_wifi_buf, modbus_wifi_len,cStr[7] - '0' );
								modbus_wifi_len = 0;								
							}
							
							Check_connect_AP();  // check whether wirte SSID
				
						}
						

					}	
					else 	if(ucID == UCID_BACNET) // udp bacnet port 47808
					{
						uint16_t pdu_len = 0;  
						BACNET_ADDRESS far src;
						count = 0;
						if(flag_wifi == 0)
						{
							flag_wifi = 1;
							pdu_len = datalink_receive(&src, &packet[0], packet_len, 0 ,BAC_IP);
							{
								if((pdu_len > 0) && (pdu_len < 512)) 
								{
									npdu_handler(&src, &packet[0], pdu_len, BAC_IP);	
									if(bacnet_wifi_len > 0)
									{
										ESP8266_SendString ( DISABLE, (uint8_t *)&bacnet_wifi_buf, bacnet_wifi_len,cStr[7] - '0' );
										bacnet_wifi_len = 0;
									}
								}			
							}								
						}
						flag_wifi = 0;
					}					
					else if(ucID == UCID_SCAN) // private scan port
					{ 
						u8 n;
						u8 i;
						if(packet[0] == 0x64)
						{
							state = 1;
							for(n = 0;n < (u8)packet_len / 4;n++)
							{       
								if((packet[4*n+1] == SSID_Info.ip_addr[0]) && (packet[4*n+2] == SSID_Info.ip_addr[1])
									 &&(packet[4*n+3] == SSID_Info.ip_addr[2]) && (packet[4*n+4] == SSID_Info.ip_addr[3]))
								{ 
									 state=0;
								}
							}				
							
							if(state)
							{          
								//use broadcast when scan			
								UdpData(0);
								//serialnumber 4 bytes
								Scan_Infor.master_sn[0] = 0;
								Scan_Infor.master_sn[1] = 0;
								Scan_Infor.master_sn[2] = 0;
								Scan_Infor.master_sn[3] = 0;
								
								for(i = 0;i < 20;i++)
									Scan_Infor.panelname[i] = panelname[i];			
								//uip_send((char *)&Scan_Infor, sizeof(STR_SCAN_CMD));
								//Scan_Infor.zigbee_exist = zigbee_exist | 0x02; 
								Scan_Infor.zigbee_exist |= 0x02; 
								ESP8266_SendString ( DISABLE, (uint8_t *)&Scan_Infor, sizeof(STR_SCAN_CMD),cStr[7] - '0' );
							}
						}
					}						
				}
				if(count_checkip % 100 == 0)
				{
					static u8 count_abnormal = 0;
					
					if(Get_SSID_RSSI() == 0)
					{
						count_abnormal++;
					}
					else
					{
						SSID_Info.IP_Wifi_Status = WIFI_NORMAL;
						count_abnormal = 0;
					}
					
					if(count_abnormal == 3)
						SSID_Info.IP_Wifi_Status = WIFI_NO_WIFI;
				}
				if(count_checkip++ > 300)
				{
					count_checkip = 0;
			
					if(ESP8266_CIPSTA_CUR(1) == 2)
						get_ip = 0;
				}
			}
		}		
	}
}



//uint8_t SendBuff[SENDBUFF_SIZE]; //用于DMA 串口发送

//// DAM1_Channel2

////dma2的初始化，注意tx和rx的dma初始化时不同的，包括存储器地址（外设地址相同，都是uart->DR），另外dma的方向也不相同。
//void dma_init_uart()
//{
//    DMA_InitTypeDef DMA_InitTypeStruct;
//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

//    DMA_DeInit(DMA1_Channel2);   //串口1的DMA传输通道是通道4
//    DMA_InitTypeStruct.DMA_BufferSize = SENDBUFF_SIZE; //传输大小
//    DMA_InitTypeStruct.DMA_DIR = DMA_DIR_PeripheralDST; //外设作为DMA的目的端
//    DMA_InitTypeStruct.DMA_M2M = DMA_M2M_Disable;  //??????????
//    DMA_InitTypeStruct.DMA_MemoryBaseAddr = (u32)SendBuff; //??????
//    DMA_InitTypeStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //???????? ??
//    DMA_InitTypeStruct.DMA_MemoryInc = DMA_MemoryInc_Enable; //???????
//    DMA_InitTypeStruct.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal（只传送一次）, DMA_Mode_Circular （不停地传送）
//                                                     //DMA_InitTypeStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR); //;(u32)&USART1->DR; //?????
//    DMA_InitTypeStruct.DMA_PeripheralBaseAddr = (u32)&USART3->DR; //?????
//    DMA_InitTypeStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //?????? ??
//    DMA_InitTypeStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址不增加
//    DMA_InitTypeStruct.DMA_Priority = DMA_Priority_Medium;  //(DMA传送优先级为中等)
//    DMA_Init(DMA1_Channel2, &DMA_InitTypeStruct);

//}


//void dma_send_uart_data(uint16 nlength)
//{
//    USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
//    DMA_SetCurrDataCounter(DMA1_Channel2, nlength );
//    DMA_Cmd(DMA1_Channel2, ENABLE);


//    while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);
//    DMA_ClearFlag(DMA1_FLAG_TC2);

//    DMA_Cmd(DMA1_Channel2, DISABLE);
//    USART_DMACmd(USART3, USART_DMAReq_Tx, DISABLE);
//}



//unsigned char Send_Uart_Data(char * m_send_data, uint16 nlength)
//{
//    memcpy(SendBuff, m_send_data, nlength);
//    dma_send_uart_data(nlength);
//}

#endif

