
#include "config.h" 


#define ParameterOperationSTACK_SIZE	portMINIMAL_STACK_SIZE//512
xTaskHandle Handle_Scan, Handle_ParameterOperation;

//extern U8_T subnet_idle;
//extern xSemaphoreHandle sem_subnet_tx;
//extern xQueueHandle qSubSerial;

SCAN_DB far scan_db[MAX_ID];
SCAN_DB far current_db;
U8_T db_ctr = 1;
U8_T reset_scan_db_flag = 0;

U8_T db_online[32];	// Should be added by scan
U8_T db_occupy[32]; // Added/subtracted by scan
U8_T current_online[32]; // Added/subtracted by co2 request command
U8_T current_online_ctr = 0;

U8_T get_para[32];

static U8_T scan_db_changed = FALSE;

static U8_T node_operate_index = 0;
static U16_T node_operate_reg = 0;
static U16_T node_operate_value = 0;
static U8_T node_operate_flag = 0;

U8_T external_nodes_plug_and_play = 0;


U8_T scan_idle_id = 0;

U8_T receive_scan_reply(U8_T *sendbuf, U8_T *p, U8_T len)
{
U16_T crc_check = crc16(p, 7); // crc16
//	if((subnet_response_buf[0] = 0xff) && (subnet_response_buf[1] == CHECKONLINE)
//		&& (HIGH_BYTE(crc_check) == p[7]) && (LOW_BYTE(crc_check) == p[8]))
	if((p[0] = sendbuf[0]) && (p[1] == sendbuf[1]) && (HIGH_BYTE(crc_check) == p[7]) && (LOW_BYTE(crc_check) == p[8]))
	{
		current_db.id = p[2];
		current_db.sn = ((U32_T)p[6] << 24) | ((U32_T)p[5] << 16) | 
						((U32_T)p[4] << 8) | p[3];

		if((len == uart3_rec_package_size) && (sendbuf[2] == sendbuf[3]))
		{	 
			return UNIQUE_ID;
		}
		else
		{  
			
			return UNIQUE_ID_FROM_MULTIPLE;
		}
	}
	else
	{	 
		return MULTIPLE_ID;
	}
}

U8_T send_scan_cmd(U8_T max_id, U8_T min_id)
{
	U16_T wCrc16;	
	U8_T buf[6];
	U8_T length;
	U8_T ret;
	U8_T uart3_response_buf[UART3_BUF_LEN];
//	if(cSemaphoreTake(sem_subnet_tx, 10) == pdFALSE)
//		return SCAN_BUSY;
	if(wait_for_uart3_idle(5) == FALSE)
		return SCAN_BUSY; 
	uart3_idle = FALSE; 
	buf[0] = 0xff;
	buf[1] = 0x19;
	buf[2] = max_id;
	buf[3] = min_id;

	wCrc16 = crc16(buf, 4);

	buf[4] = HIGH_BYTE(wCrc16);
	buf[5] = LOW_BYTE(wCrc16);

 	uart3_send_string(buf, 6); 
	
	set_uart3_parameters(RECEIVE, 9); 
	length = wait_uart3_response(10);
 
	if(length)
	{
		U8_T i;
		for(i = 0; i < length; i++)
			xQueueReceive(qSubSerial3, uart3_response_buf+i, 0);
		
		ret = receive_scan_reply(buf, uart3_response_buf, length);
		 
	}
	else // NONE_ID || MULTIPLE_ID
	{
		if(uxQueueMessagesWaiting(qSubSerial3))
			ret = MULTIPLE_ID;
		else
			ret = NONE_ID;
		 
		
	}
 
	set_uart3_parameters(SEND, 0);
//	cSemaphoreGive(sem_subnet_tx); 
	uart3_idle = TRUE;
	return ret;
}

U8_T receive_assign_id_reply(U8_T *p, U8_T length)
{
	U16_T crc_check = crc16(p, 10); // crc16
	if((length == uart3_rec_package_size) && (HIGH_BYTE(crc_check) == p[10]) && (LOW_BYTE(crc_check) == p[11]))
		return ASSIGN_ID;
	else
		return NONE_ID;
}

U8_T assignment_id_with_sn(U8_T old_id, U8_T new_id, U32_T current_sn)
{
	U8_T buf[12];
	U16_T wCrc16;
	U8_T length, ret;
	U8_T uart3_response_buf[UART3_BUF_LEN];
//	if(cSemaphoreTake(sem_subnet_tx, 10) == pdFALSE)
//		return SCAN_BUSY;
	if(wait_for_uart3_idle(5) == FALSE)
		return SCAN_BUSY; 
	uart3_idle = FALSE;
	buf[0] = old_id;
	buf[1] = 0x06;
	buf[2] = 0;
	buf[3] = 0x0a;	//MODBUS_ADDRESS_PLUG_N_PLAY = 10
	buf[4] = 0x55;
	buf[5] = new_id;

	buf[6] = (U8_T)current_sn;
	buf[7] = (U8_T)(current_sn >> 8);
	buf[8] = (U8_T)(current_sn >> 16);
	buf[9] = (U8_T)(current_sn >> 24);

	wCrc16 = crc16(buf, 10);

	buf[10] = HIGH_BYTE(wCrc16);
	buf[11] = LOW_BYTE(wCrc16);

 	uart3_send_string(buf, 12);
	set_uart3_parameters(RECEIVE, 12);

	if((length = wait_uart3_response(10)) != 0)
	{
		U8_T i;
		for(i = 0; i < length; i++)
			xQueueReceive(qSubSerial3, uart3_response_buf+i, 0);
		
		ret = receive_assign_id_reply(uart3_response_buf, length);
	}

	set_uart3_parameters(SEND, 0);
//	cSemaphoreGive(sem_subnet_tx); 
	uart3_idle = TRUE;
	return ret;
}

U8_T get_idle_id(void)
{
	U8_T i;
	for(i = 1; i < MAX_ID; i++)
	{
		if(((db_online[i / 8] & (1 << (i % 8))) == 0) && ((db_occupy[i / 8] & (1 << (i % 8))) == 0))
			return i;
	}
	return 0xff;
}

void check_id_in_database(U8_T id, U32_T sn)
{
	if(db_online[id / 8] & (1 << (id % 8))) // in the database
	{
		U8_T i;
		for(i = 0; i < db_ctr; i++)
		{
			if(id == scan_db[i].id) // id already in the database
			{
				if(sn != scan_db[i].sn) // if there exist the same id with defferent sn, push it into the db_occupy list
				{
					if(external_nodes_plug_and_play == 0)
						remove_id_from_db(i);
					else
						db_occupy[id / 8] |= 1 << (id % 8);
				}
				break;
			}
			// if the device is already in the database, return without doing anything
		}
	}
	else
	{
		db_online[id / 8] |= 1 << (id % 8);
		db_occupy[id / 8] &= ~(1 << (id % 8));
		get_para[db_ctr / 8] |= 1 << (db_ctr % 8);
		scan_db[db_ctr].id = id;
		scan_db[db_ctr].sn = sn;
		db_ctr++;
		scan_db_changed = TRUE;
	}
}

void bin_search(U8_T min_id, U8_T max_id)  
{
	U8_T scan_status;

	if(min_id > max_id) return;

	scan_status = send_scan_cmd(max_id, min_id);

	switch(scan_status)	// wait for response from nodes scan command
	{
		case UNIQUE_ID:
			 
			// unique id means it is the only id in the range.
			// if the id is already in the database, set it occupy and then change the id with sn in the dealwith_conflict_id routine.
			check_id_in_database(current_db.id, current_db.sn);
			if(min_id != max_id) // to avoid the miss reply some nodes
			{
				bin_search(min_id, (U8_T)(((U16_T)min_id + (U16_T)max_id) / 2));
				bin_search((U8_T)(((U16_T)min_id + (U16_T)max_id) / 2) + 1, max_id);
			}			
			break;
		case MULTIPLE_ID:
			// multiple id means there is more than one id in the range.
			// if the min_id == max_id, there is same id, set the id occupy and return
			// if the min_id != max_id, there is multi id in the range, divide the range and do the sub scan
			if(min_id == max_id)
			{
				db_occupy[min_id / 8] |= 1 << (min_id % 8);
				if((db_online[min_id / 8] & (1 << (min_id % 8))) && (external_nodes_plug_and_play == 0))
				{
					U8_T i = 0;
					for(i = 0; i < db_ctr; i++)
						if(scan_db[i].id == min_id)
							break;

					remove_id_from_db(i);
				}
			}
			else
			{
				bin_search(min_id, (U8_T)(((U16_T)min_id + (U16_T)max_id) / 2));
				
				bin_search((U8_T)(((U16_T)min_id + (U16_T)max_id) / 2) + 1, max_id);
			}
			break;
		case UNIQUE_ID_FROM_MULTIPLE:
			// there are multiple ids in the range, but the fisrt reply is good.
			if(min_id == max_id)
			{
				db_occupy[min_id / 8] |= 1 << (min_id % 8);
				if((db_online[min_id / 8] & (1 << (min_id % 8))) && (external_nodes_plug_and_play == 0))
				{
					U8_T i = 0;
					for(i = 0; i < db_ctr; i++)
						if(scan_db[i].id == min_id)
							break;

					remove_id_from_db(i);
				}
			}
			else
			{
				check_id_in_database(current_db.id, current_db.sn);

				bin_search(min_id, (U8_T)(((U16_T)min_id + (U16_T)max_id) / 2));
				bin_search((U8_T)(((U16_T)min_id + (U16_T)max_id) / 2) + 1, max_id);
			}
			break;
		case NONE_ID:
			// none id means there is not id in the range
			break;
		case SCAN_BUSY:
		default:
			bin_search(min_id, max_id);
			break;
	}

	return;
}

void dealwith_conflict_id(void)
{
	U8_T idle_id;
	U8_T status;
	U8_T occupy_id = 1;
	U8_T try_times = 3;

	while(1)
	{
		if(db_occupy[occupy_id / 8] & (1 << (occupy_id % 8)))
		{
			try_times--;
			if(try_times == 0)
			{
				occupy_id++;
				try_times = 0;
				if(occupy_id == 0xff)
					break;
				else
					continue;
			}

			idle_id = get_idle_id();
			if(idle_id == 0xff) break;
			else scan_idle_id = idle_id;

			status = send_scan_cmd(occupy_id, occupy_id); // get the seperate sn

			if((status == UNIQUE_ID) || (status == UNIQUE_ID_FROM_MULTIPLE))
			{
				if(occupy_id == current_db.id)
					db_occupy[occupy_id / 8] &= ~(1 << (occupy_id % 8));

				check_id_in_database(current_db.id, current_db.sn);
				// if still occupy in the database
				if(db_occupy[current_db.id / 8] & (1 << (current_db.id % 8)))
				{
					// assign idle id with sn to this occupy device.
					if(assignment_id_with_sn(current_db.id, idle_id, current_db.sn)== ASSIGN_ID)
					{
						db_online[idle_id / 8] |= 1 << (idle_id % 8);
						scan_db[db_ctr].id = idle_id;
						scan_db[db_ctr].sn = current_db.sn;
						db_ctr++;
						scan_db_changed = TRUE;
					}
				}

				if(status == UNIQUE_ID_FROM_MULTIPLE)
					continue;
			}
			else if(status == MULTIPLE_ID)
			{
				continue;
			}
			else // if(status == NONE_ID)
			{
				db_occupy[occupy_id / 8] &= ~(1 << (occupy_id % 8));
				// maybe the nodes are removed from the subnet, so skip it.
			}
		}
		
		occupy_id++;
		try_times = 0;
		if(occupy_id == 0xff) break;
	}

	return;
}

void scan_sub_nodes(void)
{
	bin_search(1, 254);
	if(external_nodes_plug_and_play == 1)
		dealwith_conflict_id();

	if(scan_db_changed == TRUE)
	{
// 		start_data_save_timer();
		scan_db_changed = FALSE;
	}
}

// read from flash to get the init db_online status
void init_scan_db(void)
{
//	U8_T i;

	U8_T local_id = modbus.address;
	U32_T local_sn = ((U32_T)modbus.serial_Num[3] << 24) | ((U32_T)modbus.serial_Num[2] << 16) | 
				((U32_T)modbus.serial_Num[1] << 8) | modbus.serial_Num[0];

	memset(db_online, 0, 32);
	memset(db_occupy, 0, 32);
	memset(get_para, 0, 32);
	memset(current_online, 0, 32);
	memset((uint8 *)(&scan_db[0].id), 0, SCAN_DB_SIZE * MAX_EXT_CO2);
	current_online_ctr = 0;

//	db_ctr = 0; //////////////////
//	if(/*!flash_read_char(FLASH_SCAN_DB_CTR, &db_ctr) || */(db_ctr == 0x00) || (db_ctr == 0xff))
	{
		db_ctr = 1; 
		scan_db[0].id = local_id;
		scan_db[0].sn = local_sn;  
		db_online[scan_db[0].id / 8] |= (1 << (scan_db[0].id % 8));
	}
//	else
//	{
//		uint8 sn_temp[4];
//		for(i = 0; i < db_ctr; i++)
//		{ 
//			scan_db[i].sn = ((U32_T)sn_temp[3] << 24) | ((U32_T)sn_temp[2] << 16) | 
//							((U32_T)sn_temp[1] << 8) | (U32_T)sn_temp[0];

//			if((i == 0)	&& ((scan_db[i].id != local_id) || (scan_db[i].sn != local_sn)))
//			{
//				scan_db[0].id = local_id;
//				scan_db[0].sn = local_sn; 
//			} 

//			db_online[scan_db[i].id / 8] |= (1 << (scan_db[i].id % 8));
//		}		
//	}
}

void clear_scan_db(void)
{
	db_ctr = 0;
//	flash_write_int(FLASH_SCAN_DB_CTR, db_ctr);

	init_scan_db();
// 	start_data_save_timer();
}

void remove_id_from_db(U8_T index)
{
	U8_T i;
	if((db_ctr > 1) && (index < db_ctr) && (index > 0)) // can not delete the internal sensor
	{
		i = scan_db[index].id;
		db_online[i / 8] &= ~(1 << (i % 8));
		db_occupy[i / 8] &= ~(1 << (i % 8));
		if(current_online[i / 8] & (1 << (i % 8)))
		{
			current_online[i / 8] &= ~(1 << (i % 8));
			current_online_ctr--;
		}

		for(i = index; i < db_ctr - 1; i++)
		{
			scan_db[i].id = scan_db[i+1].id;
			scan_db[i].sn = scan_db[i+1].sn;
		}
		db_ctr--;
		scan_db_changed = TRUE;
	}
}

U8_T check_master_id_in_database(U8_T set_id, U8_T increase) 
{
	U8_T i;

	if((set_id == 0) && (increase == 0))
		set_id = 254;
	
	if((set_id == 255) && (increase == 1))
		set_id = 1;

	for(i = 1; i < db_ctr; i++)
	{
		if(scan_db[i].id == set_id)
			break;
	}

	if(i >= db_ctr)
		return set_id;
	else
	{
		if(increase == 1)
			return check_master_id_in_database(set_id+1, 1);
		else
			return check_master_id_in_database(set_id-1, 0);
	}
}

void modify_master_id_in_database(U8_T old_id, U8_T set_id)
{
	modbus.address = set_id;
	Station_NUM = modbus.address;
	scan_db[0].id = set_id;
//	Inital_Bacnet_Server();
	dlmstp_init(NULL);
	AT24CXX_WriteOneByte(EEP_ADDRESS, modbus.address);

	// modify scan datebase
	db_online[old_id / 8] &= ~(1 << (old_id % 8));
	db_online[set_id / 8] |= 1 << (set_id % 8);

// 	start_data_save_timer();
}

#define TOTAL_BITMAP_BYTES	((MAX_EXT_CO2 + 1 + 7) >> 3)
void get_parameters(uint8 index, uint8 *p)
{
//	uint16 crc_check = crc16(p, 9); // crc16
//	if((HIGH_BYTE(crc_check) == p[9]) && (LOW_BYTE(crc_check) == p[10]))
//	{
//		ext_co2_str[index - 1].co2_offset = (int16)((p[3] << 8) | p[4]);
//		ext_co2_str[index - 1].alarm_setpoint = (p[5] << 8) | p[6];
//		ext_co2_str[index - 1].pre_alarm_setpoint = (p[7] << 8) | p[8];
//		get_para[index / 8] &= ~(1 << (index % 8));
//	}
}

void get_parameters_from_nodes(void)
{
	U8_T i, j, length;
	U8_T buf[8];
	U16_T crc_check;
	U8_T uart3_response_buf[UART3_BUF_LEN];
	for(i = 0; i < TOTAL_BITMAP_BYTES; i++)
		if(get_para[i]) break;

	if(i >= TOTAL_BITMAP_BYTES) return;

	for(j = 0; j < 8; j++)
		if(get_para[i] & (1 << j)) break;

	i = (i << 3) + j;
	j = i;

	if(i < (MAX_EXT_CO2 + 1))
	{
//		if(cSemaphoreTake(sem_subnet_tx, 5) == pdFALSE)
//			return;
		if(wait_for_uart3_idle(5) == FALSE)
			return; 
		uart3_idle = FALSE;
		buf[0] = scan_db[i].id;
		buf[1] = READ_VARIABLES;
		buf[2] = HIGH_BYTE(SLAVE_MODBUS_CO2_OFFSET);
		buf[3] = LOW_BYTE(SLAVE_MODBUS_CO2_OFFSET); // start address
	
		buf[4] = 0;
		buf[5] = 3;
	
		crc_check = crc16(buf, 6); // crc16
		buf[6] = HIGH_BYTE(crc_check);
		buf[7] = LOW_BYTE(crc_check);
	
 		uart3_send_string(buf, 8);
		set_uart3_parameters(RECEIVE, 11);

		if((length = wait_uart3_response(10)) != 0)
		{
			for(i = 0; i < length; i++)
				xQueueReceive(qSubSerial3, uart3_response_buf+i, 0);
			
			get_parameters(j, uart3_response_buf);
		}
	
		set_uart3_parameters(SEND, 0);
//		cSemaphoreGive(sem_subnet_tx); 
		uart3_idle = TRUE;
		return;
	}
}

void write_parameters_to_nodes(uint8 index, uint16 reg, uint16 value)
{
	node_operate_index = index;
	node_operate_reg = reg;
	node_operate_value = value;
	node_operate_flag = 1;
}

void check_write_to_nodes(void)
{
	U8_T buf[8], length;
	U16_T crc_check;

	if(node_operate_flag == 0)
		return;

//	if(cSemaphoreTake(sem_subnet_tx, 5) == pdFALSE)
//		return;
	if(wait_for_uart3_idle(5) == FALSE)
			return; 
	uart3_idle = FALSE;
	buf[0] = scan_db[node_operate_index].id;
	buf[1] = WRITE_VARIABLES;
	buf[2] = HIGH_BYTE(node_operate_reg);
	buf[3] = LOW_BYTE(node_operate_reg); // start address

	buf[4] = HIGH_BYTE(node_operate_value);
	buf[5] = LOW_BYTE(node_operate_value);

	crc_check = crc16(buf, 6); // crc16
	buf[6] = HIGH_BYTE(crc_check);
	buf[7] = LOW_BYTE(crc_check);

 	uart3_send_string(buf, 8);
	set_uart3_parameters(RECEIVE, 8);

	// send successful if receive the reply
	if((length = wait_uart3_response(10)) != 0)
		node_operate_flag = 0; // without doing checksum

	set_uart3_parameters(SEND, 0);
//	cSemaphoreGive(sem_subnet_tx); 
	uart3_idle = TRUE;
}

void ScanTask(void *pvParameters )
{
	portTickType xDelayPeriod = (portTickType)2000 / portTICK_RATE_MS;
	uart3_modbus_init();
	init_scan_db(); // move to the routine when pull all the parameters from flash in data.c
	print("Scan Task\r\n");
	delay_ms(100);
	
	while(1)
	{
		 
 		scan_sub_nodes();
//		stack_detect(&test[12]);
		vTaskDelay(xDelayPeriod); 
//		taskYIELD();
	}
}

void ParameterOperationTask(void *pvParameters )
{
	portTickType xDelayPeriod = (portTickType)1000 / portTICK_RATE_MS;
	print("Parameter operation Task\r\n");
	delay_ms(100);
	
	while(1)
	{ 
		get_parameters_from_nodes();
		check_write_to_nodes();
		if(reset_scan_db_flag)
		{
			clear_scan_db();
			reset_scan_db_flag = 0;
		} 
//	stack_detect(&test[13]);		
		vTaskDelay(xDelayPeriod);
	}
}

void vStartScanTask(unsigned char uxPriority)
{
     xTaskCreate(ScanTask, 				(signed portCHAR *)"ScanTask", 				 configMINIMAL_STACK_SIZE, NULL, uxPriority+128, NULL);
 	xTaskCreate(ParameterOperationTask, (signed portCHAR *)"ParameterOperationTask", configMINIMAL_STACK_SIZE, NULL, uxPriority, NULL);
 
}


