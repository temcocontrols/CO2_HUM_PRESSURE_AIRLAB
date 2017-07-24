#include "config.h"
unsigned char PRODUCT_ID;  
static void vFlashTask( void *pvParameters );  
static void vCOMMTask( void *pvParameters );

//static void vUSBTask( void *pvParameters );

static void vNETTask( void *pvParameters );
 

 
 
static void vMSTP_TASK(void *pvParameters );
void uip_polling(void);

static void watchdog_init(void);
void watchdog(void);
#define	BUF	((struct uip_eth_hdr *)&uip_buf[0])	
	
u8 update = 0 ;
u8 read_cal = 0 ;

u32 Instance = 0x0c;
uint8_t  PDUBuffer[MAX_APDU];

//u8 global_key = KEY_NON;

static void debug_config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

int main(void)
{
  	
// 	int8 i; 
//   	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);
// 	debug_config(); 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
 	delay_init(72);	
  
	watchdog_init();
	qSubSerial3 = xQueueCreate(UART3_MAX_BUF, 1);
	
	qSubSerial = xQueueCreate(SUB_BUF_LEN, 1);
	
	xMutex = xQueueCreateMutex();
	IicMutex = xQueueCreateMutex();
	qKey = xQueueCreate(5, 2);
	if(( qSubSerial3 == NULL )||( qSubSerial == NULL )  ||( xMutex == NULL )||( IicMutex == NULL ))    
	{
		while(1);
	}
	
	Lcd_Initial();
	Lcd_Show_String(0, 0, DISP_NOR, (uint8 *)"Initialization...");
	EEP_Dat_Init();
 	
	start_back_light(backlight_keep_seconds);
//	print("EEP Init Done!\r\n");
   	
 	mass_flash_init() ;
	
	
//	print("FLASH Init Done!\r\n");
//	beeper_gpio_init();
//	beeper_on();
//	delay_ms(1000);
//	beeper_off();
	//Lcd_Initial();
	SPI1_Init();
//	print("SPI1 Init Done!\r\n");
	Lcd_Show_String(1, 0, DISP_NOR, (uint8 *)"EEP is Done");
//	SPI2_Init();
//	mem_init(SRAMIN);
//	TIM3_Int_Init(5000, 7199);
//	TIM6_Int_Init(100, 7199);
   	
//	uart3_modbus_init();
	
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485))
		RTC_Init();

//	print("RTC Init Done!\r\n");
	Lcd_Show_String(2, 0, DISP_NOR, (uint8 *)"RTC is Done");
	watchdog(); 
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PM25))
	{	
		while(tapdev_init())	//初始化ENC28J60错误
		{								   
	// 		print("ENC28J60 Init Error!\r\n");
			delay_ms(100);
		}
	}
// 	print("ENC28J60 Init Done!\r\n");
//	watchdog();  
	
	Lcd_Show_String(3, 0, DISP_NOR, (uint8 *)"Net is Done");
	print("CO2_NET\n\r");
 	Lcd_Show_String(4, 0, DISP_NOR, (uint8 *)"Done");
	delay_ms(100);
	watchdog(); 
	
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PM25))  
		xTaskCreate( vNETTask, ( signed portCHAR * ) "NET",  configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 5 , NULL );
  
  	xTaskCreate( vMSTP_TASK, ( signed portCHAR * ) "MSTP", configMINIMAL_STACK_SIZE + 256  , NULL, tskIDLE_PRIORITY + 5, NULL );
 	xTaskCreate( vCOMMTask, ( signed portCHAR * ) "COMM", configMINIMAL_STACK_SIZE + 128, NULL, tskIDLE_PRIORITY + 2, NULL );

	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) )
	{
		xTaskCreate( Co2_task,       ( signed portCHAR * ) "Co2Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
		xTaskCreate( Alarm_task,   ( signed portCHAR * ) "AlarmTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3,  NULL);
//		vStartScanTask(tskIDLE_PRIORITY + 1);
	}
	if(PRODUCT_ID == STM32_PM25)
	{
		xTaskCreate( PM25_task,       ( signed portCHAR * ) "Co2Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	}
	
	if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) )
		xTaskCreate(vUpdate_Pressure_Task, (signed portCHAR *)"Update_Pressure_Task", configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 3, NULL);
	else
		xTaskCreate(vUpdate_Temperature_Task, (signed portCHAR *)"Update_Temperature_Task", configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 3, NULL);
	 
	xTaskCreate(vStartPIDTask, (signed portCHAR *)"vStartPIDTask", configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 1, NULL);
 
 	xTaskCreate( vFlashTask, ( signed portCHAR * ) "FLASH", configMINIMAL_STACK_SIZE + 1500, NULL, tskIDLE_PRIORITY + 2, NULL );
  
    xTaskCreate( vOutPutTask		,( signed portCHAR * ) "OutPut" 		, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
 
    xTaskCreate( vKEYTask, ( signed portCHAR * ) "KEY", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL );
   
    vStartMenuTask(tskIDLE_PRIORITY + 1);
  	
  
	 
	 
//	xTaskCreate( vFlashTask, ( signed portCHAR * ) "INPUTS", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL );

//	xTaskCreate( vCOMMTask, ( signed portCHAR * ) "COMM", configMINIMAL_STACK_SIZE + 512, NULL, tskIDLE_PRIORITY + 2, NULL );
//	xTaskCreate( vNETTask, ( signed portCHAR * ) "NET",  configMINIMAL_STACK_SIZE + 512, NULL, tskIDLE_PRIORITY + 1 , NULL );
// 	xTaskCreate( vMSTP_TASK, ( signed portCHAR * ) "MSTP", configMINIMAL_STACK_SIZE  + 512   , NULL, tskIDLE_PRIORITY + 1, NULL );


//	xTaskCreate( vOutPutTask		,( signed portCHAR * ) "OutPut" 		, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );

//	xTaskCreate( Co2_task,       ( signed portCHAR * ) "Co2Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL);
//	xTaskCreate( Alarm_task,   ( signed portCHAR * ) "AlarmTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4,  NULL);

//	xTaskCreate(vUpdate_Temperature_Task, (signed portCHAR *)"Update_Temperature_Task", configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 5, NULL);
 	  
	vTaskStartScheduler();
}

 
void vCOMMTask(void *pvParameters )
{
	modbus_init(); 
	reply_done =receive_delay_time;
	print("COMM Task\r\n");
	delay_ms(100);
	
	for( ;; )
	{
		if (dealwithTag)
		{  
		 dealwithTag--;
		  if(dealwithTag == 1)//&& !Serial_Master )	
			dealwithData();
		}
		if(serial_receive_timeout_count>0)  
		{
				serial_receive_timeout_count -- ; 
				if(serial_receive_timeout_count == 0)
				{
					serial_restart();
				}
		}
		
//		stack_detect(&test[2]);
		vTaskDelay(5 / portTICK_RATE_MS);
		
	}
	
}

 


 
 
void vFlashTask( void *pvParameters )
{ 
	uint8 i;
	modbus.write_ghost_system = 0;
	print("Flash Task\r\n");
	delay_ms(100);
	
	for( ;; )
	{  
		Flash_Write_Mass(); 
		poll_main_net_status();
		if(modbus.write_ghost_system == 1)
		{
			modbus.ip_mode = modbus.ghost_ip_mode ;
			modbus.tcp_server = modbus.ghost_tcp_server ;
			modbus.listen_port = modbus.ghost_listen_port ;
			AT24CXX_WriteOneByte(EEP_IP_MODE, modbus.ip_mode);
			AT24CXX_WriteOneByte(EEP_TCP_SERVER, modbus.tcp_server);				
			AT24CXX_WriteOneByte(EEP_LISTEN_PORT_HI, modbus.listen_port>>8);
			AT24CXX_WriteOneByte(EEP_LISTEN_PORT_LO, modbus.listen_port &0xff);
			for(i=0; i<4; i++)
			{
				modbus.ip_addr[i] = modbus.ghost_ip_addr[i] ;
				modbus.mask_addr[i] = modbus.ghost_mask_addr[i] ;
				modbus.gate_addr[i] = modbus.ghost_gate_addr[i] ;
				
				AT24CXX_WriteOneByte(EEP_IP_ADDRESS_1+i, modbus.ip_addr[i]);
				AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_1+i, modbus.mask_addr[i]);
				AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_1+i, modbus.gate_addr[i]);						
			}

			IP_Change = 1; 
			modbus.write_ghost_system = 0 ;
		}
//		stack_detect(&test[6]);
		
		vTaskDelay(1000 / portTICK_RATE_MS);
	}	
}
  










void Inital_Bacnet_Server(void)
{
 uint32 ltemp; 
	Set_Object_Name((char *)panelname); 
	Device_Init();
	
	ltemp = 0;
	ltemp |= AT24CXX_ReadOneByte(EEP_INSTANCE_LOWORD);
	ltemp |= (uint32)AT24CXX_ReadOneByte(EEP_INSTANCE_LOWORD+1)<<8;
	ltemp |= (uint32)AT24CXX_ReadOneByte(EEP_INSTANCE_LOWORD+2)<<16;
	ltemp |= (uint32)AT24CXX_ReadOneByte(EEP_INSTANCE_LOWORD+3)<<24;
	if(ltemp == 0xffffffff)
		Instance = ((uint16)modbus.serial_Num[1]<<8)|modbus.serial_Num[0];
	else
		Instance = ltemp;
	Station_NUM = AT24CXX_ReadOneByte(EEP_STATION_NUMBER);
	Device_Set_Object_Instance_Number(Instance);  
	address_init();
	bip_set_broadcast_addr(0xffffffff); 
 
	AIS = MAX_INS + 1;
	AOS = MAX_AOS + 1;
	BOS = 0;
	AVS = MAX_AVS + 1;
  
}
//#define SWITCH_TIMER	 600
void vMSTP_TASK(void *pvParameters )
{
	uint16_t pdu_len = 0; 
	BACNET_ADDRESS  src;
//	static u16 protocal_timer = SWITCH_TIMER;
	modbus.protocal_timer_enable = 0;
//	bacnet_inital();	
	Inital_Bacnet_Server();
	dlmstp_init(NULL);
	Recievebuf_Initialize(0);
	print("MSTP Task\r\n");
	delay_ms(100);
	
	for (;;)
    {
			
		
//		if(modbus.protocal_timer_enable == 1) // switch to bac_mstp,delay 3000 ms
//		{	
//			if(protocal_timer !=0) protocal_timer--;
//			else
//			{
//				u8 i;
//				modbus.protocal_timer_enable = 0;
//				protocal_timer  =SWITCH_TIMER;
//				
//				for(i=0;i < USART_REC_LEN;i++)
//					USART_RX_BUF[revce_count++] = 0; //clear receiver buf
//	 
//				serial_restart();
//				
//				modbus.protocal= BAC_MSTP;
//				AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, BAC_MSTP);
//			}
//		} 
			
		
		if(modbus.protocal == BAC_MSTP)
		{
			pdu_len = datalink_receive(&src, &PDUBuffer[0], sizeof(PDUBuffer), 0,	modbus.protocal);
			if(pdu_len) 
			{
				npdu_handler(&src, &PDUBuffer[0], pdu_len, BAC_MSTP);	
			} 
						
		}
// 		stack_detect(&test[1]);
		vTaskDelay(5 / portTICK_RATE_MS);
	} 	
}

////extern u32 count_out;
////extern u8 buffer_out[64];
//void vUSBTask( void *pvParameters )
//{
////	u8 i;
//	for( ;; )
//	{
////		if((count_out != 0) && (bDeviceState == CONFIGURED))
////		{
//////			USB_To_USART_Send_Data(&buffer_out[0], count_out);
////			for(i = 0; i < count_out; i++)
////			{

////			}
////			count_out = 0;
////		}
//		delay_ms(200);

//	}
//}

void vNETTask( void *pvParameters )
{
	//uip_ipaddr_t ipaddr;
	
	//u8 tcp_server_tsta = 0XFF;
	//u8 tcp_client_tsta = 0XFF;
	//print("Temco ARM Demo\r\n");
//	delay_ms(500);
	u8 count = 0 ;
//	while(tapdev_init())	//初始化ENC28J60错误
//	{								   
////	 	print("ENC28J60 Init Error!\r\n");
////		delay_ms(100);
//		;
//	};		
	//print("ENC28J60 Init OK!\r\n");
//	uip_init();							//uIP初始化
//	
//	uip_ipaddr(ipaddr, 192, 168, 0, 163);	//设置本地设置IP地址
//	uip_sethostaddr(ipaddr);					    
//	uip_ipaddr(ipaddr, 192, 168, 0, 4); 	//设置网关IP地址(其实就是你路由器的IP地址)
//	uip_setdraddr(ipaddr);						 
//	uip_ipaddr(ipaddr, 255, 255, 255, 0);	//设置网络掩码
//	uip_setnetmask(ipaddr);

//	print("IP:192.168.0.163\r\n"); 
//	print("MASK:255.255.255.0\r\n"); 
//	print("GATEWAY:192.168.0.4\r\n"); 	
//	
//	uip_listen(HTONS(1200));			//监听1200端口,用于TCP Server
//	uip_listen(HTONS(80));				//监听80端口,用于Web Server
//	tcp_client_reconnect();	   		    //尝试连接到TCP Server端,用于TCP Client
	print("N e t Task\r\n");
	delay_ms(100);
	
    for( ;; )
	{
		uip_polling();	//处理uip事件，必须插入到用户程序的循环体中 
		
		if((IP_Change == 1)||(update == 1))
		{
			count++ ;
			if(count == 10)
			{
				count = 0 ;
				IP_Change = 0 ;	
//				//if(!tapdev_init()) print("Init fail\n\r");				
//				while(tapdev_init())	//初始化ENC28J60错误
//				{								   
//				//	print("ENC28J60 Init Error!\r\n");
//				delay_ms(50);
//				};	
				if(update == 1)
				{	
					Lcd_Full_Screen(0);
					Lcd_Show_String(1, 6, 0, (uint8 *)"Updating...");
					Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
				}
				else
				{
					Lcd_Full_Screen(0);
					Lcd_Show_String(1, 6, 0, (uint8 *)"Restarting...");
					Lcd_Show_String(2, 3, 0, (uint8 *)"Don't power off!");
				}
				
				SoftReset();
			}
			
		}
//		stack_detect(&test[0]);
		
//		if(tcp_server_tsta != tcp_server_sta)		//TCP Server状态改变
//		{															 
//			if(tcp_server_sta & (1 << 7))
//				print("TCP Server Connected   \r\n");
//			else
//				print("TCP Server Disconnected\r\n"); 
//			
// 			if(tcp_server_sta & (1 << 6))			//收到新数据
//			{
//   			print("TCP Server RX:%s\r\n", tcp_server_databuf);//打印数据
//				tcp_server_sta &= ~(1 << 6);		//标记数据已经被处理	
//			}
//			tcp_server_tsta = tcp_server_sta;
//		}
//		
//		if(global_key == KEY_1)						//TCP Server 请求发送数据
//		{
//			global_key = KEY_NON;
//			if(tcp_server_sta & (1 << 7))			//连接还存在
//			{
//				sprint((char*)tcp_server_databuf, "TCP Server OK\r\n");	 
//				tcp_server_sta |= 1 << 5;			//标记有数据需要发送
//			}
//		}
//		
//		if(tcp_client_tsta != tcp_client_sta)		//TCP Client状态改变
//		{
//			if(tcp_client_sta & (1 << 7))
//				print("TCP Client Connected   \r\n");
//			else
//				print("TCP Client Disconnected\r\n");
//			
// 			if(tcp_client_sta & (1 << 6))			//收到新数据
//			{
//    			print("TCP Client RX:%s\r\n", tcp_client_databuf);//打印数据
//				tcp_client_sta &= ~(1 << 6);		//标记数据已经被处理	
//			}
//			tcp_client_tsta = tcp_client_sta;
//		}
//		
//		if(global_key == KEY_2)						//TCP Client 请求发送数据
//		{
//			global_key = KEY_NON;
//			if(tcp_client_sta & (1 << 7))			//连接还存在
//			{
//				sprint((char*)tcp_client_databuf, "TCP Client OK\r\n");	 
//				tcp_client_sta |= 1 << 5;			//标记有数据需要发送
//			}
//		}
		
//		if(global_key == KEY_3)
//		{
//			global_key = KEY_NON;
//			print("global_key == KEY_3\r\n");
//			mf_mount(0);
//			mf_scan_files("0:");
//			mf_showfree("0:");
//		}
		
 		vTaskDelay(5 / portTICK_RATE_MS);
    }
}



//uip事件处理函数
//必须将该函数插入用户主循环,循环调用.
void uip_polling(void)
{
	u8 i;
	static struct timer periodic_timer, arp_timer;
	static u8 timer_ok = 0;	 
	if(timer_ok == 0)		//仅初始化一次
	{
		timer_ok = 1;
		timer_set(&periodic_timer, CLOCK_SECOND / 2); 	//创建1个0.5秒的定时器 
		timer_set(&arp_timer, CLOCK_SECOND * 10);	   	//创建1个10秒的定时器 
	}
	
	uip_len = tapdev_read();							//从网络设备读取一个IP包,得到数据长度.uip_len在uip.c中定义
	if(uip_len > 0)							 			//有数据
	{   
		//处理IP数据包(只有校验通过的IP包才会被接收) 
		if(BUF->type == htons(UIP_ETHTYPE_IP))			//是否是IP包? 
		{
			uip_arp_ipin();								//去除以太网头结构，更新ARP表
			uip_input();   								//IP包处理			
			//当上面的函数执行后，如果需要发送数据，则全局变量 uip_len > 0
			//需要发送的数据在uip_buf, 长度是uip_len  (这是2个全局变量)		    
			if(uip_len > 0)								//需要回应数据
			{
				uip_arp_out();							//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();							//发送数据到以太网
			}
		}
		else if (BUF->type == htons(UIP_ETHTYPE_ARP))	//处理arp报文,是否是ARP请求包?
		{
			uip_arp_arpin();
			
 			//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len(这是2个全局变量)
 			if(uip_len > 0)
				tapdev_send();							//需要发送数据,则通过tapdev_send发送	 
		}
	}
	else if(timer_expired(&periodic_timer))				//0.5秒定时器超时
	{
		timer_reset(&periodic_timer);					//复位0.5秒定时器 
		
		//轮流处理每个TCP连接, UIP_CONNS缺省是40个  
		for(i = 0; i < UIP_CONNS; i++)
		{
			 uip_periodic(i);							//处理TCP通信事件
			
	 		//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
	 		if(uip_len > 0)
			{
				uip_arp_out();							//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();							//发送数据到以太网
			}
		}
		
#if UIP_UDP	//UIP_UDP 
		//轮流处理每个UDP连接, UIP_UDP_CONNS缺省是10个
		for(i = 0; i < UIP_UDP_CONNS; i++)
		{
			uip_udp_periodic(i);						//处理UDP通信事件
			
	 		//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
			if(uip_len > 0)
			{
				uip_arp_out();							//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();							//发送数据到以太网
			}
		}
#endif 
		//每隔10秒调用1次ARP定时器函数 用于定期ARP处理,ARP表10秒更新一次，旧的条目会被抛弃
		if(timer_expired(&arp_timer))
		{
			timer_reset(&arp_timer);
			uip_arp_timer();
		}
	}
}


void EEP_Dat_Init(void)
{
		u8 loop ;
		u8 temp[6];
		int16 itemp;
		AT24CXX_Init();
	    initial_hum_eep();
		modbus.serial_Num[0] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD);
		modbus.serial_Num[1] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD+1);
		modbus.serial_Num[2] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_HIWORD);
		modbus.serial_Num[3] = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_HIWORD+1);
	
//		if((modbus.serial_Num[0]==0xff)&&(modbus.serial_Num[1]== 0xff)&&(modbus.serial_Num[2] == 0xff)&&(modbus.serial_Num[3] == 0xff))
//		{
//					modbus.serial_Num[0] = 1 ;
//					modbus.serial_Num[1] = 1 ;
//					modbus.serial_Num[2] = 2 ;
//					modbus.serial_Num[3] = 2 ;
//					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD, modbus.serial_Num[0]);
//					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD+1, modbus.serial_Num[1]);
//					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD+2, modbus.serial_Num[2]);
//					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD+3, modbus.serial_Num[3]);
//		}
//		Instance = modbus.serial_Num[0] + (U16_T)(modbus.serial_Num[1] << 8);
		temp[0] = AT24CXX_ReadOneByte(EEP_INSTANCE_1);
		temp[1] = AT24CXX_ReadOneByte(EEP_INSTANCE_2);
		temp[2] = AT24CXX_ReadOneByte(EEP_INSTANCE_3);
		temp[3] = AT24CXX_ReadOneByte(EEP_INSTANCE_4);
		if((temp[0]==0xff)&&(temp[1]== 0xff)&&(temp[2] == 0xff)&&(temp[3] == 0xff))
		{
				temp[0] = 1 ;
				temp[1] = 2 ;
				temp[2] = 3 ;
				temp[3] = 4 ;
				AT24CXX_WriteOneByte(EEP_INSTANCE_1, temp[0]);
				AT24CXX_WriteOneByte(EEP_INSTANCE_1+1, temp[1]);
				AT24CXX_WriteOneByte(EEP_INSTANCE_1+2, temp[2]);
				AT24CXX_WriteOneByte(EEP_INSTANCE_1+3, temp[3]);
		}
		Instance = ((u32)temp[0]<<24) +((u32)temp[1]<<16)+((u32)temp[2]<<8)+(u32)temp[0] ;
		AT24CXX_WriteOneByte(EEP_VERSION_NUMBER_LO, SOFTREV&0XFF);
		AT24CXX_WriteOneByte(EEP_VERSION_NUMBER_HI, (SOFTREV>>8)&0XFF);
		modbus.address = AT24CXX_ReadOneByte(EEP_ADDRESS);
		if((modbus.address == 255)||(modbus.address == 0))
		{
					modbus.address = 254 ;
					AT24CXX_WriteOneByte(EEP_ADDRESS, modbus.address);
		}
		modbus.product = AT24CXX_ReadOneByte(EEP_PRODUCT_MODEL);
		PRODUCT_ID = modbus.product;
//		if(modbus.product != PRODUCT_ID)//((modbus.product == 255)||(modbus.product == 0))
//		{
//			modbus.product = PRODUCT_ID ;
//			AT24CXX_WriteOneByte(EEP_PRODUCT_MODEL, modbus.product);
//		}
		modbus.hardware_Rev = AT24CXX_ReadOneByte(EEP_HARDWARE_REV);
		if((modbus.hardware_Rev == 255)||(modbus.hardware_Rev == 0))
		{
					modbus.hardware_Rev = HW_VER ;
					AT24CXX_WriteOneByte(EEP_HARDWARE_REV, modbus.hardware_Rev);
		}
		modbus.update = AT24CXX_ReadOneByte(EEP_UPDATE_STATUS);
		modbus.SNWriteflag = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_WRITE_FLAG);
		
 		if ((PRODUCT_ID == STM32_HUM_RS485) ||(PRODUCT_ID == STM32_HUM_NET))
			uart1_parity =  AT24CXX_ReadOneByte(EEP_UART1_PARITY);
			if(( uart1_parity!= NONE_PARITY)&&( uart1_parity!= ODD_PARITY)&&( uart1_parity!= EVEN_PARITY))
				uart1_parity = NONE_PARITY;	
		else
			uart1_parity = NONE_PARITY;	 
		modbus.baud = AT24CXX_ReadOneByte(EEP_BAUDRATE);
		if(modbus.baud > 4) 
		{	
			modbus.baud = 1 ;
			AT24CXX_WriteOneByte(EEP_BAUDRATE, modbus.baud);
		}
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
 				modbus.protocal= AT24CXX_ReadOneByte(EEP_MODBUS_COM_CONFIG);
				if((modbus.protocal!=MODBUS)&&(modbus.protocal!=BAC_MSTP ))
				{
					modbus.protocal = MODBUS ;
					AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, modbus.protocal);
				}
//				modbus.protocal= AT24CXX_ReadOneByte(EEP_MODBUS_COM_CONFIG);
//				if(modbus.protocal >10)
//				{
//					modbus.protocal = MODBUS;
//					AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, modbus.protocal);
//				}
//				modbus.protocal = MODBUS ;
//				modbus.protocal = BAC_MSTP;
				for(loop = 0 ; loop<6; loop++)
				{
					temp[loop] = AT24CXX_ReadOneByte(EEP_MAC_ADDRESS_1+loop); 
				}
				if((temp[0]== 0xff)&&(temp[1]== 0xff)&&(temp[2]== 0xff)&&(temp[3]== 0xff)&&(temp[4]== 0xff)&&(temp[5]== 0xff) )
				{
					temp[0] = 0x04 ;
					temp[1] = 0x02 ;
					temp[2] = 0x35 ;
					temp[3] = 0xaF ;
					temp[4] = 0x00 ;
					temp[5] = 0x01 ;
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_1, temp[0]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_2, temp[1]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_3, temp[2]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_4, temp[3]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_5, temp[4]);
					AT24CXX_WriteOneByte(EEP_MAC_ADDRESS_6, temp[5]);		
				}
				for(loop =0; loop<6; loop++)
				{
					modbus.mac_addr[loop] =  temp[loop]	;
				}
				
				for(loop = 0 ; loop<4; loop++)
				{
					temp[loop] = AT24CXX_ReadOneByte(EEP_IP_ADDRESS_1+loop); 
				}
				if((temp[0]== 0xff)&&(temp[1]== 0xff)&&(temp[2]== 0xff)&&(temp[3]== 0xff) )
				{
					temp[0] = 192 ;
					temp[1] = 168 ;
					temp[2] = 0 ;
					temp[3] = 183 ;
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_1, temp[0]);
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_2, temp[1]);
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_3, temp[2]);
					AT24CXX_WriteOneByte(EEP_IP_ADDRESS_4, temp[3]);
				}
				for(loop = 0 ; loop<4; loop++)
				{
					modbus.ip_addr[loop] = 	temp[loop] ;
					modbus.ghost_ip_addr[loop] = modbus.ip_addr[loop] ;
				}
				
				temp[0] = AT24CXX_ReadOneByte(EEP_IP_MODE);
				if(temp[0] >1)
				{
					temp[0] = 0 ;
					AT24CXX_WriteOneByte(EEP_IP_MODE, temp[0]);	
				}
				modbus.ip_mode = temp[0] ;
				modbus.ghost_ip_mode = modbus.ip_mode ;
				
				
				for(loop = 0 ; loop<4; loop++)
				{
					temp[loop] = AT24CXX_ReadOneByte(EEP_SUB_MASK_ADDRESS_1+loop); 
				}
				if((temp[0]== 0xff)&&(temp[1]== 0xff)&&(temp[2]== 0xff)&&(temp[3]== 0xff) )
				{
					temp[0] = 0xff ;
					temp[1] = 0xff ;
					temp[2] = 0xff ;
					temp[3] = 0 ;
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_1, temp[0]);
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_2, temp[1]);
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_3, temp[2]);
					AT24CXX_WriteOneByte(EEP_SUB_MASK_ADDRESS_4, temp[3]);
				
				}				
				for(loop = 0 ; loop<4; loop++)
				{
					modbus.mask_addr[loop] = 	temp[loop] ;
					modbus.ghost_mask_addr[loop] = modbus.mask_addr[loop] ;
				}
				
				for(loop = 0 ; loop<4; loop++)
				{
					temp[loop] = AT24CXX_ReadOneByte(EEP_GATEWAY_ADDRESS_1+loop); 
				}
				if((temp[0]== 0xff)&&(temp[1]== 0xff)&&(temp[2]== 0xff)&&(temp[3]== 0xff) )
				{
					temp[0] = 192 ;
					temp[1] = 168 ;
					temp[2] = 0 ;
					temp[3] = 4 ;
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_1, temp[0]);
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_2, temp[1]);
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_3, temp[2]);
					AT24CXX_WriteOneByte(EEP_GATEWAY_ADDRESS_4, temp[3]);
				
				}				
				for(loop = 0 ; loop<4; loop++)
				{
					modbus.gate_addr[loop] = 	temp[loop] ;
					modbus.ghost_gate_addr[loop] = modbus.gate_addr[loop] ;
				}
				
				temp[0] = AT24CXX_ReadOneByte(EEP_TCP_SERVER);
				if(temp[0] == 0xff)
				{
					temp[0] = 0 ;
					AT24CXX_WriteOneByte(EEP_TCP_SERVER, temp[0]);
				}
				modbus.tcp_server = temp[0];
				modbus.ghost_tcp_server = modbus.tcp_server  ;
				
				temp[0] =AT24CXX_ReadOneByte(EEP_LISTEN_PORT_HI);
				temp[1] =AT24CXX_ReadOneByte(EEP_LISTEN_PORT_LO);
				if(temp[0] == 0xff && temp[1] == 0xff )
				{
					modbus.listen_port = 502 ;
					temp[0] = (modbus.listen_port>>8)&0xff ;
					temp[1] = modbus.listen_port&0xff ;				
				}
				modbus.listen_port = (temp[0]<<8)|temp[1] ;
				modbus.ghost_listen_port = modbus.listen_port ;
				
				modbus.write_ghost_system = 0 ;
				modbus.reset = 0 ;
	 
				
					
//	alarm_state = read_eeprom(MODBUS_ALARM_AUTO_MANUAL);
//	if((alarm_state == 0x00) || (alarm_state == 0xff))
		alarm_state = STOP_ALARM;
//	else
//		alarm_state &= 0x83;

	pre_alarm_on_time = read_eeprom(EEP_PRE_ALARM_SETTING_ON_TIME);
	if((pre_alarm_on_time == 0x00) || (pre_alarm_on_time == 0xff))
		pre_alarm_on_time = 2;
	else if(pre_alarm_on_time > ALARM_ON_TIME_MAX)
		pre_alarm_on_time = ALARM_ON_TIME_MAX;

	pre_alarm_off_time = read_eeprom(EEP_PRE_ALARM_SETTING_OFF_TIME);
	if((pre_alarm_off_time == 0x00) || (pre_alarm_off_time == 0xff))
		pre_alarm_off_time = 2;
	else if(pre_alarm_off_time > ALARM_OFF_TIME_MAX)
		pre_alarm_off_time = ALARM_OFF_TIME_MAX;

// co2 output range
	itemp = ((int16)read_eeprom(EEP_OUTPUT_CO2_RANGE_MIN + 1) << 8) | read_eeprom(EEP_OUTPUT_CO2_RANGE_MIN);
//	if(itemp != -1)
 		output_range_table[CHANNEL_CO2].min = itemp;
//	else
//		output_range_table[CHANNEL_CO2].min = 0;

	itemp = ((int16)read_eeprom(EEP_OUTPUT_CO2_RANGE_MAX + 1) << 8) | read_eeprom(EEP_OUTPUT_CO2_RANGE_MAX);
//	if(itemp != -1)
 		output_range_table[CHANNEL_CO2].max = itemp;
//	else
//		#ifdef CO2_SENSOR
//		output_range_table[CHANNEL_CO2].max = 2000;
//	    #else 
//		output_range_table[CHANNEL_CO2].max = 1000;
//		#endif

	if(output_range_table[CHANNEL_CO2].min > output_range_table[CHANNEL_CO2].max)
	{
		itemp = output_range_table[CHANNEL_CO2].max;
		output_range_table[CHANNEL_CO2].min = output_range_table[CHANNEL_CO2].max;
		output_range_table[CHANNEL_CO2].max = itemp;
	}

// temperature output range
	itemp = ((int16)read_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN + 1) << 8) | read_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MIN);
//	if(itemp != -1)
 		output_range_table[CHANNEL_TEMP].min = itemp;
//	else
//		output_range_table[CHANNEL_TEMP].min = 0;

	itemp = ((int16)read_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX + 1) << 8) | read_eeprom(EEP_OUTPUT_TEMPERATURE_RANGE_MAX);
//	if(itemp != -1)
 		output_range_table[CHANNEL_TEMP].max = itemp;
//	else
//		output_range_table[CHANNEL_TEMP].max = 1000;

	if(output_range_table[CHANNEL_TEMP].min > output_range_table[CHANNEL_TEMP].max)
	{
		itemp = output_range_table[CHANNEL_TEMP].max;
		output_range_table[CHANNEL_TEMP].min = output_range_table[CHANNEL_TEMP].max;
		output_range_table[CHANNEL_TEMP].max = itemp;
	}

// humidity output range
	itemp = ((int16)read_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN + 1) << 8) | read_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MIN);
//	if(itemp != -1)
 		output_range_table[CHANNEL_HUM].min = itemp;
//	else
//		output_range_table[CHANNEL_HUM].min = 0;

	itemp = ((int16)read_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX + 1) << 8) | read_eeprom(EEP_OUTPUT_HUMIDITY_RANGE_MAX);
//	if(itemp != -1)
 		output_range_table[CHANNEL_HUM].max = itemp;
//	else
//		output_range_table[CHANNEL_HUM].max = 1000;

	if(output_range_table[CHANNEL_HUM].min > output_range_table[CHANNEL_HUM].max)
	{
		itemp = output_range_table[CHANNEL_HUM].max;
		output_range_table[CHANNEL_HUM].min = output_range_table[CHANNEL_HUM].max;
		output_range_table[CHANNEL_HUM].max = itemp;
	}

 

// password
	itemp = read_eeprom(EEP_USE_PASSWORD);
	if((itemp == 0x00) || (itemp == 0xff))
		use_password = 0;
	else
		use_password = 1;

	user_password[0] = read_eeprom(EEP_USER_PASSWORD0);
	user_password[1] = read_eeprom(EEP_USER_PASSWORD1);
	user_password[2] = read_eeprom(EEP_USER_PASSWORD2);
	user_password[3] = read_eeprom(EEP_USER_PASSWORD3);
	if((user_password[0] > '9') || (user_password[0] < '0')
	|| (user_password[1] > '9') || (user_password[1] < '0')
	|| (user_password[2] > '9') || (user_password[2] < '0')
	|| (user_password[3] > '9') || (user_password[3] < '0'))
	{
		user_password[0] = '1';
		user_password[1] = '2';
		user_password[2] = '3';
		user_password[3] = '4';
		write_eeprom(EEP_USER_PASSWORD0, user_password[0]);
		write_eeprom(EEP_USER_PASSWORD1, user_password[1]);
		write_eeprom(EEP_USER_PASSWORD2, user_password[2]);
		write_eeprom(EEP_USER_PASSWORD3, user_password[3]);
	}

//	menu_block_seconds = read_eeprom(EEP_MENU_BLOCK_SECONDS);
//	if((menu_block_seconds == 0) || (menu_block_seconds == 0xff))
//		menu_block_seconds = MENU_BLOCK_SECONDS_DEFAULT;
//	refresh_menu_block_timer();

	backlight_keep_seconds = read_eeprom(EEP_BACKLIGHT_KEEP_SECONDS);
//	if(backlight_keep_seconds == 0xff)
//		backlight_keep_seconds = BACKLIGHT_KEEP_SECONDS_DEFAULT;
	
	
	HumSensor.H_Filter = read_eeprom(EEP_HUMIDITY_FILTER);
	if(HumSensor.H_Filter == 0xff)
	{
		HumSensor.H_Filter = DEFAULT_FILTER;
		write_eeprom(EEP_HUMIDITY_FILTER,DEFAULT_FILTER); 
	}
	
	HumSensor.T_Filter = read_eeprom(EEP_EXT_TEMPERATURE_FILTER);
	if(HumSensor.T_Filter == 0xff)
	{
		HumSensor.T_Filter = DEFAULT_FILTER;
		write_eeprom(EEP_EXT_TEMPERATURE_FILTER,DEFAULT_FILTER);
	}

	Temperature_Filter = read_eeprom(EEP_INT_TEMPERATURE_FILTER);
	if(Temperature_Filter == 0xff)
	{
		Temperature_Filter = DEFAULT_FILTER;
		write_eeprom(EEP_INT_TEMPERATURE_FILTER,DEFAULT_FILTER); 
	}
	HumSensor.pre_temperature_c = 0;
	HumSensor.pre_humidity = 0; 	
	
	AT24CXX_Read(EEP_TSTAT_NAME1, panelname, 21); 
	
	
	output_offset[0][CHANNEL_HUM] = ((uint16)read_eeprom(EEP_OUTPUT_HUM_VOL_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_HUM_VOL_OFFSET);
	output_offset[0][CHANNEL_TEMP] =((uint16)read_eeprom(EEP_OUTPUT_TEMP_VOL_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_TEMP_VOL_OFFSET);
	output_offset[0][CHANNEL_CO2] =((uint16)read_eeprom(EEP_OUTPUT_CO2_VOL_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_CO2_VOL_OFFSET);
	output_offset[1][CHANNEL_HUM] =((uint16)read_eeprom(EEP_OUTPUT_HUM_CUR_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_HUM_CUR_OFFSET);
	output_offset[1][CHANNEL_TEMP] =((uint16)read_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_TEMP_CUR_OFFSET);
	output_offset[1][CHANNEL_CO2] =((uint16)read_eeprom(EEP_OUTPUT_CO2_CUR_OFFSET + 1) << 8) | read_eeprom(EEP_OUTPUT_CO2_CUR_OFFSET);
 	
}


u16 swap_int16( u16 value)
{
	u8 temp1, temp2 ;
	temp1 = value &0xff ;
	temp2 = (value>>8)&0xff ;
	
	return  (temp1<<8)|temp2 ;
}

u32 swap_int32( u32 value)
{
	u8 temp1, temp2, temp3, temp4 ;
	temp1 = value &0xff ;
	temp2 = (value>>8)&0xff ;
	temp3 = (value>>16)&0xff ;
	temp4 = (value>>24)&0xff ;
	
	return  ((u32)temp1<<24)|((u32)temp2<<16)|((u32)temp3<<8)|temp4 ;
}


void watchdog_init(void)
{
		/* Enable write access to IWDG_PR and IWDG_RLR registers */ 
		IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
		/* IWDG counter clock: 40KHz(LSI) / 128 = 0.3125 KHz */ 
		IWDG_SetPrescaler(IWDG_Prescaler_128); 
		/* Set counter reload value to 4000 = 12.8s */ 
		IWDG_SetReload(4000); 
		IWDG_Enable();  			//enable the watchdog
		IWDG_ReloadCounter(); // reload the value
}

 
 
void watchdog(void)
{
	IWDG_ReloadCounter(); // reload the value     
}


