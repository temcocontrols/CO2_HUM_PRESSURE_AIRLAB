#include "../co2/sub_net.h"

//U8_T subnet_idle = TRUE;
//xSemaphoreHandle sem_subnet_tx;
//xTaskHandle Handle_SubSerial;

//U8_T sub_transmit_finished;
U8_T subnet_response_buf[60];
U8_T subnet_rec_package_size;

//U8_T subnet_status = SUBNET_IDLE;



static uint8   rece_count1;
static uint8   rece_size1; 
//uint8   data_buffer1[DATABUFLEN1]; 
static bit dealwithTag1 			=0;
static u8 Sub_Uart_Send_Buf[SUB_BUF_LEN]; 
static u8 Co2_Sendbyte_Num; 
static uint16 Slave_address;
static int8 co2_uart_send_count = 0;

/*********************************************\
*	SUB NET CODE START
\*********************************************/
xQueueHandle qSubSerial;
//U8_T sub_data_buffer[SUB_BUF_LEN];



 


//void sub_init_send_com(void)
//{
//	sub_transmit_finished = 0;
//	SUB_DIR = SEND;
//	DELAY_Us(100);
//}



//void sub_net_isr(void) interrupt SUB_INTERRUPT
//{
//	U8_T buf;
//	U8_T cTaskWokenByPost = FALSE;
//	if(RI0 == 1)
//	{
//		RI0 = 0;
//		buf = SUB_BUF;
//		cQueueSendFromISR(qSubSerial, (void *)&buf, cTaskWokenByPost);
//		
//		if(cTaskWokenByPost != pdFALSE)
//	        taskYIELD();
//  	}
//	else if(TI0 == 1)
//	{
//		TI0 = 0;
//		sub_transmit_finished = 1;
//	}
//}




void set_sub_serial_baudrate(U32_T BR)
{
	uart2_init(BR);
//	delay_ms(1);
}

//void sub_serial_restart(void)
//{
//	SUB_DIR = SEND;
//}

void sub_net_init(void)
{  
	set_sub_serial_baudrate(19200);
//	qSubSerial = xQueueCreate(SUB_BUF_LEN, 1);
}



//void forward_to_slave(U8_T *buf, U8_T length)
//{
//	U8_T i;
//	sub_init_send_com();
//	sub_init_crc16();

//	for(i = 0; i < length; i++)
//		sub_send_byte(buf[i], TRUE);

//	sub_send_byte(SubCRChi, FALSE);					//crchi
//	sub_send_byte(SubCRClo, FALSE);					//crclo
//}






void set_subnet_parameters(U8_T io, U8_T length)
{
//	U8_T temp;
	subnet_rec_package_size = length;
//	SUB_DIR = io;
//	// clear qSubSerial queue
//	while(cQueueReceive(qSubSerial, &temp, 0) == pdTRUE);
}

U8_T wait_subnet_response(U8_T nDoubleTick)
{
	U8_T i, length;
	for(i = 0; i < nDoubleTick; i++)
	{
		if((length = uxQueueMessagesWaiting(qSubSerial)) >= subnet_rec_package_size)
			return length;

		vTaskDelay(2/portTICK_RATE_MS);
	}
	return 0;
}

//U8_T wait_for_subnet_idle(U8_T nDoubleTick)
//{
//	U8_T i;
//	for(i = 0; i < nDoubleTick; i++)
//	{
////		if(subnet_idle == TRUE)
//			return TRUE;

//		vTaskDelay(2/portTICK_RATE_MS);
//	} 
//	return FALSE;
//}

//U8_T wiat_subnet_idle(U8_T nDoubleTick)
//{
//	U8_T i;
//	for(i = 0; i < nDoubleTick; i++)
//	{
//		if(subnet_status == SUBNET_IDLE)
//			return SUBNET_IDLE;
//
//		vTaskDelay(2);
//	}
//	return SUBNET_BUSY;
//}
//
//void set_subnet_status(U8_T status)
//{
//	subnet_status = status;
//}

  
#define READY   0
#define DONE    1
static void Sub_USART_SendDataString( u8 status)
{ 
	if(status == READY)
		Co2_Sendbyte_Num = 0;
	else if(status == DONE)
	{
		 
		co2_uart_send_count = 0;
		
// 		USART_SendData(USART2, Sub_Uart_Send_Buf[co2_uart_send_count++]);
   		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);//start to transmit
		
//		USART_SendData(USART2, USART_FLAG_TC); 
//		USART_SendData(USART2, Sub_Uart_Send_Buf[co2_uart_send_count++] ); 
	}
}
static void sub_send_byte(uint8 buffer, uint8 crc)
{
   Sub_Uart_Send_Buf[Co2_Sendbyte_Num]  = buffer; 
   if(Co2_Sendbyte_Num < SUB_BUF_LEN) Co2_Sendbyte_Num++;
   if(crc == TRUE)
   {
      crc16_byte(buffer);
   }  
}


void sub_send_string(U8_T *p, U8_T length)
{
	U8_T i;
	Sub_USART_SendDataString(READY);
	for(i = 0; i < length; i++)
		sub_send_byte(p[i], FALSE);
	Sub_USART_SendDataString(DONE);
}
 
void serial1_restart(void)
{ 
	rece_count1 = 0;
	dealwithTag1 = FALSE; 		    		  
}

void USART2_IRQHandler(void)                	//串口2中断服务程序
{
//	static u8 sub_uart_send_count = 0;
	unsigned portBASE_TYPE uxSavedInterruptStatus;
	U8_T cTaskWokenByPost;
	U8_T buf;
	
	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	
	cTaskWokenByPost = FALSE;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)	//接收中断
	{ 
		buf = USART_ReceiveData(USART2); 
// 		USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
		if(PRODUCT_ID == STM32_PM25)
		{
			if(pm25_sensor_type == SENSIRION)
			{
			  if(pm25_current_cmd == SENSIRION_READ_MEASUREMENT)
				{
				  if((buf == 0x7E)&&(sensirion_rev_cnt == 0))//rcv first byte
					{
						subnet_response_buf[sensirion_rev_cnt++] = buf;				
					}
					else if((sensirion_rev_cnt) && (buf != 0x7E))
					{
						if(shift_flag == 1)
						{
							if(buf == 0x5e)
								subnet_response_buf[sensirion_rev_cnt++] = 0x7e;
							else if(buf == 0x5d)
								subnet_response_buf[sensirion_rev_cnt++] = 0x7d;
							else if(buf == 0x31)
								subnet_response_buf[sensirion_rev_cnt++] = 0x11;
							else if(buf == 0x33)
								subnet_response_buf[sensirion_rev_cnt++] = 0x13;
							shift_flag = 0;
						}
						else
						{
							if(buf == 0x7D)
								shift_flag = 1;
							else
								subnet_response_buf[sensirion_rev_cnt++] = buf;
						}
						
						
						
					}

					else if(buf == 0x7E && sensirion_rev_cnt > 40)
					{
						sensirion_rev_end = PACKAGE_END;
					}
				}
			}
//			else
//			{
//				if(rece_count1 < SUB_BUF_LEN)
//				{	
//					subnet_response_buf[rece_count1++] =buf; 
//				}
//				else
//					serial1_restart();
//			}
		}
		else if((internal_co2_module_type == MAYBE_OGM200) || (internal_co2_module_type == OGM200) || (internal_co2_module_type == MH_Z19B) || (internal_co2_module_type == SCD30))//add new co2 sensor 
		{
			 
			xQueueSendFromISR(qSubSerial, (void *)&buf, (void *)&cTaskWokenByPost);
			if(cTaskWokenByPost != pdFALSE)
				taskYIELD(); 	
		}
		else
		{
			if(rece_count1 < SUB_BUF_LEN)
			{	
				subnet_response_buf[rece_count1++] =buf; 
			}
			else
				serial1_restart();
			
			if(rece_count1 == 1)	   
			{	    
				if(subnet_response_buf[1] == WRITE_VARIABLES) 
					rece_size1 = 8;
				else //if (slave_mbpoll.cmd == READ_VARIABLES) 
					rece_size1 = 7;	
//				serial_receive_timeout_count1 = 12;
			} 
				// As soon as you receive the final byte, switch to SEND mode
			else if((rece_count1 == rece_size1)&& ((subnet_response_buf[1] == READ_VARIABLES)||(subnet_response_buf[1] == WRITE_VARIABLES)))
			{	 
			// full packet received - turn off serial timeout
//				serial_receive_timeout_count1 = 0;	  
				dealwithTag1 = TRUE;   
				
			}
		}
	} 
	else if( USART_GetITStatus(USART2, USART_IT_TXE) == SET  )
	{
//		 USART_ClearFlag(USART2, USART_FLAG_TC);
// 		USART_SendData(USART2, 0X55); 
		 if( co2_uart_send_count < Co2_Sendbyte_Num)
		 {
//			 Sub_Uart_Send_Buf[co2_uart_send_count] = 0x01 + co2_uart_send_count;
 			
			 USART_SendData(USART2, Sub_Uart_Send_Buf[co2_uart_send_count++] );  
			

		 }
		 else
		 { 
//			 
 			 while(USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET){;}//等待所有数据发送完成
//			 USART_ClearFlag(USART2, USART_FLAG_TC);
  			 USART_ITConfig(USART2, USART_IT_TXE, DISABLE); 
 			 co2_uart_send_count = 0 ;   
 		 }
	}
	
	portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}


 

static bit checkData1(void)
{
	uint16 crc_val; 
	
	if((subnet_response_buf[0] != 255)&& (subnet_response_buf[0] != 0))
		return FALSE;		  
	if((subnet_response_buf[1] != READ_VARIABLES) && (subnet_response_buf[1] != WRITE_VARIABLES)  )
		return FALSE;
	
	crc_val = crc16(subnet_response_buf, rece_size1 - 2);
 
	if(crc_val == (((uint16)subnet_response_buf[rece_size1-2] << 8) | subnet_response_buf[rece_size1 - 1]))
	{    
		return TRUE;
	}
	else
	{	 
		return FALSE;
	}	
}
static void internalDeal1(uint16 start_address)
{
	if( subnet_response_buf[1] ==WRITE_VARIABLES) 
	{
	
	}
	else if ( subnet_response_buf[1] ==READ_VARIABLES) 
	{
//		if(Slave_address == REG_CO2_VALUE)
//			co2_data_temp= ((uint16)subnet_response_buf[3] << 8) + subnet_response_buf[4];
	 
	} 		
}


static void dealwithData1(void)
{
	//MDF 6/25/04 changed the timing of responses to valid and invalid packets. 
		 
		if(checkData1() == TRUE)
		{ 
		   uint16 address; 
			//given this is used in multiple places, decided to put it as an argument
		   address = ((uint16)subnet_response_buf[2] << 8) + subnet_response_buf[3];
		// Initialize tranmission
		//	initSend_COM1();
		// Initialize CRC
		//	init_crc16();
		// Store any data being written 
			internalDeal1(address );	 
		} 
		 
	// Restart the serial receive.
	serial1_restart();
}

int8 DEAL_TEMCO_SENSOR(void)
{ 
	uint8   serial_receive_timeout_count1;
	serial_receive_timeout_count1 = 10;
	do
	{ 
		
		if(serial_receive_timeout_count1 > 0)
		{
			serial_receive_timeout_count1--;
			if(serial_receive_timeout_count1 == 0)
			{
				serial1_restart();
			}
		}
		if(dealwithTag1)
		{  
			dealwithData1();
			return 1;
		}
		delay_ms(2);  
	}while(serial_receive_timeout_count1);
	
	return 0;
}


void read_from_slave(uint16 addr)
{
	uint8 temp;	
	uint8 cmd;
	uint8 id;	
    Slave_address = REG_CO2_VALUE;
	id = 0xFF;
	cmd = READ_VARIABLES;
	init_crc16();
	//send FF 06 00 79 00 64 4C 26 
	//temp = 0XFF;
	Sub_USART_SendDataString(READY);
	sub_send_byte(id, TRUE);		 //device id
	//temp = 0X06;
	sub_send_byte(cmd, TRUE);	 //function code 
	temp = (addr >> 8) & 0xff;
	sub_send_byte(temp, TRUE);	 //address high
	temp = addr & 0xff;
	sub_send_byte(temp, TRUE);	 //address low
	 
	sub_send_byte(0, TRUE);	     //value high
	 
	sub_send_byte(1, TRUE);	     //value low

	sub_send_byte(CRChi, FALSE);	 //CRC
	
	sub_send_byte(CRClo, FALSE);	 //CRC
	 
	Sub_USART_SendDataString(DONE);
	
	serial1_restart();
}

uint8 get_uart2_length(void)
{
	return rece_count1;
}

