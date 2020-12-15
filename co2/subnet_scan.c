 #include "../co2/subnet_scan.h"
#define UART3_SERIAL_RECEIVE_TIMEOUT		10  
uint8 uart3_rece_count = 0;
uint8 uart3_dealwithTag = 0; 
uint8 serial3_receive_timeout_count = UART3_SERIAL_RECEIVE_TIMEOUT;  
uint8 Uart3_Sendbyte_Num;
uint8 Uart3_Send_Buf[UART3_MAX_BUF];
uint8 uart3_rec_package_size;
xQueueHandle qSubSerial3;
uint8 uart3_idle = TRUE;
void uart3_serial_restart(void)
{
	UART3_TXEN = RECEIVE;//TXEN_RECEIVE();
	uart3_rece_count = 0;
	uart3_dealwithTag = 0; 
}

void uart3_modbus_init(void)
{
//	qSubSerial3 = xQueueCreate(UART3_MAX_BUF, 1);
	uart3_init(19200);
	uart3_serial_restart(); 
	
//	serial3_receive_timeout_count = UART3_SERIAL_RECEIVE_TIMEOUT;
}

//it is ready to send data by serial port . 
static void init_uart3_send_com(void)
{ 
	UART3_TXEN = SEND;//TXEN_SEND(); 
}
#define READY   0
#define DONE    1
static void PRI_USART3_SendDataString( u8 status)
{ 
	if(status == READY)
		Uart3_Sendbyte_Num = 0;
	else if(status == DONE)
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);//start to transmit
}
void uart3_send_byte(uint8 buffer, uint8 crc)
{
   Uart3_Send_Buf[Uart3_Sendbyte_Num]  = buffer; 
   if(Uart3_Sendbyte_Num < UART3_MAX_BUF) Uart3_Sendbyte_Num++;
   if(crc == TRUE)
   {
      crc16_byte(buffer);
   }  
}

void uart3_send_string(U8_T *p, U8_T length)
{
	U8_T i;
	init_uart3_send_com();
	PRI_USART3_SendDataString(READY);     //INITIAL SEN
	
	for(i = 0; i < length; i++)
		uart3_send_byte(p[i], FALSE);
	
	PRI_USART3_SendDataString(DONE);   //START TO TRANSMIT 
}

U8_T wait_uart3_response(U8_T nDoubleTick)
{
	U8_T i, length;
	for(i = 0; i < nDoubleTick; i++)
	{
		if((length = uxQueueMessagesWaiting(qSubSerial3)) >= uart3_rec_package_size)
			return length;

		vTaskDelay(10/portTICK_RATE_MS);
	}
	return 0;
}
void set_uart3_parameters(U8_T io, U8_T length)
{
 	U8_T temp;
	uart3_rec_package_size = length;
//	SUB_DIR = io;
//	// clear qSubSerial queue
 	while(xQueueReceive(qSubSerial3, &temp, 0) == pdTRUE);
}

void receive_external_co2_ppm(U8_T index, U8_T *p)
{
//	U16_T crc_check;

//	if(p[0] != scan_db[index].id)
//		return;

//	crc_check =	crc16(p, 5);
//	if(crc_check != (((U16_T)p[5] << 8) | p[6]))
//		return;
//	 
//	ext_co2_str[index - 1].co2_int = ((U16_T)p[3] << 8) | p[4];
//	if(ext_co2_str[index - 1].co2_int == ISP_PPM)
//	{
//		ext_co2_str[index - 1].co2_int = EXCEPTION_PPM;
//		ext_co2_str[index - 1].warming_time = TRUE;
//		remove_id_from_db(index);
//	}
//	else
//	{
//		if(ext_co2_str[index - 1].co2_int == EXCEPTION_PPM)
//		{
//			 ext_co2_str[index - 1].warming_time = TRUE;
//		}
//		else
//		{
//			ext_co2_str[index - 1].co2_int	+= ext_co2_str[index - 1].co2_offset;
//			ext_co2_str[index - 1].warming_time = FALSE;
//		}

//		ext_co2_str[index - 1].fail_counter = 0;
//		if((current_online[scan_db[index].id / 8] & (1 << (scan_db[index].id % 8))) == 0x00)
//		{
//			current_online[scan_db[index].id / 8] |= (1 << (scan_db[index].id % 8));
//			current_online_ctr++;
//		}
//	}
}

void request_external_co2(U8_T index)
{
//	U8_T buf[8], length;
//	U16_T crc_check;
//	U8_T uart3_response_buf[UART3_BUF_LEN];
//	
//	buf[0] = scan_db[index].id;
//	if(db_occupy[buf[0]	/ 8] & (1 << (buf[0] % 8)))
//		return;

//	 
//	
////	if(cSemaphoreTake(sem_subnet_tx, 5) == pdFALSE)
////		return;
////	if(wait_for_subnet_idle(5) == FALSE)
////		return;
////	subnet_idle = FALSE;

//	buf[1] = READ_VARIABLES;
//	buf[2] = HIGH_BYTE(SLAVE_MODBUS_CO2);
//	buf[3] = LOW_BYTE(SLAVE_MODBUS_CO2); // start address
//	buf[4] = 0;
//	buf[5] = 1; // read one register

//	crc_check = crc16(buf, 6); // crc16
//	buf[6] = HIGH_BYTE(crc_check);
//	buf[7] = LOW_BYTE(crc_check);

//	
// 	uart3_send_string(buf, 8);
//	
// 	set_uart3_parameters(RECEIVE, 7);
//	
// 	 
// 	if((length = wait_uart3_response(10)) != 0)
//	{
//		U8_T i; 
//		for(i = 0; i < length; i++)
//		{
//			xQueueReceive(qSubSerial3, uart3_response_buf+i, 0); 
//		}
// 		 
// 		receive_external_co2_ppm(index, uart3_response_buf);
//	}
//	
//	else
//	{
//		if(ext_co2_str[index - 1].fail_counter < MAX_CO2_REQUIRE_FAIL_CTR)
//			ext_co2_str[index - 1].fail_counter++;
//		else
//		{
//			ext_co2_str[index - 1].co2_int	= EXCEPTION_PPM;
//			if(current_online[scan_db[index].id / 8] & (1 << (scan_db[index].id % 8)))
//			{
//				current_online[scan_db[index].id / 8] &= ~(1 << (scan_db[index].id % 8));
//				current_online_ctr--;
//			}
//		}
//	} 
//  	set_uart3_parameters(SEND, 0);
//	cSemaphoreGive(sem_subnet_tx);
//	subnet_idle = TRUE;
}


 


void send_test(void)
{
  	 
//	if(dealwithTag == VALID_PACKET)
//	{
//		init_main_send_com();
	//	data_buffer[0] = 0x20;
	//	data_buffer[1] = 0x21;
	//	data_buffer[2] = 0x22;
	//	data_buffer[3] = 0x23;
	//	data_buffer[4] = 0x24;
	//	data_buffer[5] = 0x25;
	//	data_buffer[6] = 0x26;
	//	data_buffer[7] = 0x27;
		init_uart3_send_com();
		PRI_USART3_SendDataString(READY);     //INITIAL SEN
		uart3_send_byte(0X20, FALSE);// crchi
		uart3_send_byte(0X21, FALSE);// crclo
		uart3_send_byte(0X22, FALSE);// crchi
		uart3_send_byte(0X23, FALSE);// crclo
		uart3_send_byte(0X24, FALSE);// crchi
		uart3_send_byte(0X25, FALSE);// crclo
		uart3_send_byte(0X26, FALSE);// crclo
		uart3_send_byte(0X27, FALSE);// crclo
		uart3_send_byte(0X28, FALSE);// crclo
		PRI_USART3_SendDataString(DONE);   //START TO TRANSMIT  
//	}
 	  
}
// when had received data ,the routine begin to dealwith interal by command external.
//static void uart3_internalDeal(uint16 start_address)
//{
//	if(data_buffer[1] == MULTIPLE_WRITE_VARIABLES)
//	{
//		 
//	}

//	else if( data_buffer[1] == WRITE_VARIABLES)  
//	{
//		if(start_address < ORIGINALADDRESSVALUE)
//		{		
//		
//		}  
//		else
//		{
//			//reserved
//		}

//	}
//	else if(data_buffer[1] == READ_VARIABLES)
//	{
//		//clear the isp state flag
//	}

//	 
//}

//the routine organize  data back and send the data to buffer.
//static void responseData(uint16 address)
//{
//	uint8 flash_data;
//	uint8 num;
//	uint8 i;
//	PRI_USART_SendDataString(READY);     //INITIAL SEND BUFFER
//	if(data_buffer[1] == WRITE_VARIABLES)
//	{
//		for(i = 0; i < rece_size; i++)
//		{
//			send_byte(data_buffer[i], FALSE);//FALSE: do not need crc

//		}
//	}
//	else if(data_buffer[1] == MULTIPLE_WRITE_VARIABLES)
//	{
//		// --- response to a multiple write function ---
//		// the 6 first bits are the same and then send the crc bits
//		for (i = 0; i < 6; i++)
//		{
//			send_byte(mul_buffer[i], TRUE);//TRUE: need crc

//		}	
//		// send the two last CRC bits
//		send_byte(CRChi, FALSE);
//		send_byte(CRClo, FALSE);
//	}
//	else if(data_buffer[1] == READ_VARIABLES)
//	{
//		num = data_buffer[5];		
//		send_byte(data_buffer[0], TRUE);
//		send_byte(data_buffer[1], TRUE);
//		send_byte(num << 1, TRUE);

//		for(i = 0; i < num; i++)
//		{ 
//			 if(i + address < ORIGINALADDRESSVALUE)
//			{ 	   
//				send_byte(0, TRUE);
//				send_byte(flash_data, TRUE);
//			} 
//			else
//			{  
//				send_byte(0, TRUE);
//				send_byte(0, TRUE);
//			}
//		}//end of number
//		send_byte(CRChi, FALSE);// crchi
//		send_byte(CRClo, FALSE);// crclo
//	}
//	else if(data_buffer[1] == CHECKONLINE)
//	{ 
//	}
//	PRI_USART_SendDataString(DONE);   //START TO TRANSMIT   
//}




//bit checkData(void)
//{
//	uint16 crc_val;
//	uint8 minaddr, maxaddr;
//	uint16 address;

//	// given this is used in multiple places, decided to put it as an argument
//	address = ((uint16)data_buffer[2] << 8) + data_buffer[3];

//	if((data_buffer[0] != 255) && (data_buffer[0] != laddress) && (data_buffer[0] != 0))
//		return FALSE;	

//	if(data_buffer[1] == CHECKONLINE)
//	{
//		crc_val = crc16(data_buffer, 4);
//		if(crc_val != (((uint16)data_buffer[4]<< 8) + data_buffer[5]))
//		{
//			return FALSE;
//		}
//		minaddr = (data_buffer[2] >= data_buffer[3]) ? data_buffer[3] : data_buffer[2];	
//		maxaddr = (data_buffer[2] >= data_buffer[3]) ? data_buffer[2] : data_buffer[3];	
//		
//		if(laddress < minaddr || laddress > maxaddr)
//			return FALSE;
//		else			
//		{	//in the TRUE case, we add a random delay such that the Interface can pick up the packets
//			uint8 i, variable_delay;
//			srand(tick_ctr);
//			variable_delay = rand() % 10;
//			for(i = 0; i < variable_delay; i++)
//				delay_us(20);
//			return TRUE;
//		}
//	}

//	if((data_buffer[1] != READ_VARIABLES) && (data_buffer[1] != WRITE_VARIABLES) && (data_buffer[1] != MULTIPLE_WRITE_VARIABLES))
//		return FALSE;
// 
//	// Modified by Evan
//	if((address == MODBUS_ADDRESS_PLUG_N_PLAY) && (data_buffer[1] == WRITE_VARIABLES))
//	{
//		if(data_buffer[6] != eeprom_read_byte(EEP_SERIALNUMBER_LOWORD + 0)) 
//			return FALSE;
//		if(data_buffer[7] != eeprom_read_byte(EEP_SERIALNUMBER_LOWORD + 1)) 
//			return FALSE;
//		if(data_buffer[8] != eeprom_read_byte(EEP_SERIALNUMBER_LOWORD + 2))  
//			return FALSE;
//		if(data_buffer[9] != eeprom_read_byte(EEP_SERIALNUMBER_LOWORD + 3)) 
//			return FALSE;
//	}

//	if(data_buffer[5] == 161 && data_buffer[3] == 16 && data_buffer[0] == 255)
//		return FALSE;

//	crc_val = crc16(data_buffer, rece_size -2);

//	if(crc_val == (((uint16)data_buffer[rece_size-2] << 8) | data_buffer[rece_size-1]))
//		return TRUE;
//	else
//		return FALSE;
//}

//	void dealwithData(void)
//	{

//		if(dealwithTag == VALID_PACKET)
//		{
//		if(checkData())
//		{	
//			uint16 address;
//			//given this is used in multiple places, decided to put it as an argument
//			address = ((uint16)data_buffer[2] << 8) + data_buffer[3];
//			//MDF 6/25/04 changed the timing of responses to valid and invalid packets.
//			//Initialize tranmission
//			init_main_send_com();
//			//Initialize CRC
//			init_crc16();
//			//Store any data being written
//			internalDeal(address);
//			//Respond with any data requested
//			responseData(address);
//			
//			is_comm_good = TRUE;
//		}
//		else
//			init_main_send_com();
//			
//		dealwithTag = INVALID_PACKET;

//		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//		}

//	}
void USART3_IRQHandler(void)                	//串口1中断服务程序
{
	static u8 uart3_send_count = 0;
	U8_T cTaskWokenByPost = FALSE;
	U8_T buf;
	
	unsigned portBASE_TYPE uxSavedInterruptStatus;
	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)	//接收中断
	{   
		buf = USART_ReceiveData(USART3);   
		xQueueSendFromISR(qSubSerial3, (void *)&buf, (void *)&cTaskWokenByPost);
		 
		
		if(cTaskWokenByPost != pdFALSE) 
			taskYIELD(); 	
	} 
	else  if( USART_GetITStatus(USART3, USART_IT_TXE) == SET  )
	{  
		 if( uart3_send_count <   Uart3_Sendbyte_Num)
		 {
 			 USART_SendData(USART3, Uart3_Send_Buf[uart3_send_count++] );  
			 //USART_ClearFlag(USART1, USART_FLAG_TC);	 
		 }
		 else
		 { 
			 while(USART_GetFlagStatus(USART3, USART_FLAG_TC) != SET);//等待所有数据发送完成
			
			 USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
			 uart3_send_count = 0 ;  
			 uart3_serial_restart();  
		 } 
	}
	
	portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}



//void vUARTTask(void *pvParameters)
//{
//	
//	modbus_init();  
//	dealwithTag = VALID_PACKET;
////	delay_ms(100);
//	for( ; ; )
//	{ 
//		if(dealwithTag)
//		{
//			dealwithData();
//		}

//		if(serial_receive_timeout_count > 0)
//		{
//			serial_receive_timeout_count--;
//			if(serial_receive_timeout_count == 0)
//			{
//				main_serial_restart();
//			}
//		}  
//		delay_ms(5);
//	} 	
//}
 
U8_T wait_for_uart3_idle(U8_T nDoubleTick)
{
	U8_T i;
	for(i = 0; i < nDoubleTick; i++)
	{
		if(uart3_idle == TRUE)
			return TRUE;

		vTaskDelay(2/ portTICK_RATE_MS);
	}

	return FALSE;
}


