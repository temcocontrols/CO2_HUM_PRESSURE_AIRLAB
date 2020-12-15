
#include "config.h"
#include "myiic.h"
#include "sht3x.h"

uint8 point_num = 0; 
bit serial_int_flag = 0;
uint8 humidity_version;
STR_HUMIDITY HumSensor; 
uint8 hum_heat_status = 0;

int16 external_operation_value = 0;
uint8 external_operation_flag = 0;


#define SDA			PAout(2)	//SDA	
#define SCL			PAout(3)	//SCL
//#define IIC_SCL		PAout(3)	//SCL
//#define IIC_SDA		PAout(2)	//SDA	 
//#define READ_SDA	PAin(2)		//输入SDA 
//IIC所有操作函数 
void i2c_pic_start(void)
{
	SDA_OUT(); 
	SDA = 0;
	delay_us(30);
	SCL = 0;
	delay_us(30);
	
	// reset bus
	SDA = 1;
	SCL = 1;
	delay_us(30);
	
	// 2nd start condition
	SDA = 0;
	delay_us(30);
	SCL = 0;
	
	 
   
//	SDA = 0; 
//	delay_ms(1);
//	SDA = 1; 
//	delay_ms(5);
//	SDA = 0; 
//	delay_ms(10); 
//	SDA = 1; 
		
}

void i2c_pic_stop()
{	
	SDA_OUT();		//sda线输出
	delay_us(30);
	SCL = 0;
	SDA = 0;
	delay_us(30);
	SCL = 1;
	SDA = 1; 
}

void i2c_pic_write(u8 ch)
{
	uint8 i = 8;
 	SDA_OUT();		//sda线输出 
	do
	{    
		if((ch & 0x80))
			SDA = 1;
		else
			SDA = 0;
		ch <<= 1;
		
		// Pulse the clock
		delay_us(30); // settling time ,when use perry's code, change this to 10, not work
		SCL = 1;
		delay_us(30);
		SCL = 0; 
	}while(--i != 0);
	
 // 	SDA = 1;
}

#define READ_BITS	16
u16 i2c_pic_read(void)
{
	uint8 i;
	uint16 data1 = 0;
	SDA_IN();			//SDA设置为输入 
	 
	for(i = 0; i < READ_BITS; i++)
	{
		delay_us(30);
		SCL = 1;
		delay_us(30);
		
		if(READ_SDA)
			data1 = (data1 << 1) | 0x01;
		else
			data1 = (data1 << 1) ;
		SCL = 0;
	}
	return data1;
}

u8 GET_ACK(void)
{
	bit c = 0;
	uint8 i = 0;
	SDA_IN();			//SDA设置为输入
	delay_us(10);
	for (i = 0; i < 10; i++)
	{
		c = READ_SDA;
		if(c == 0)
		{
			// if data line is low, pulse the clock.
			SCL = 1;
			delay_us(50);
			SCL = 0;
			return 0;
		}
		delay_us(2);
	}
	SCL = 0;
	return 1;
}

void GIVE_PIC_ACK()
{
	uint8 j = 0;
	SDA_IN();			//SDA设置为输入 
	// Wait until the data signal goes high
	while(!READ_SDA)
	{
		j++;
		// If no clock, exit i2c_read routine
		if (j == 100)
		{
			// CCCounters[cccPICREAD_ERR3]++; 
			return;
		}
 		delay_us(1);
	}
	
	SDA_OUT();		//sda线输出
	delay_us(30);
	// Bring the data line low
	SDA = 0;
	delay_us(30); // settling time 
	
	// Pulse the clock
	SCL = 1;
	delay_us(30);
	SCL = 0;
	
	// Bring the data line back high
	SDA = 1;
} 

void GIVE_PIC_NACK(void)
{
	SDA_OUT();		//sda线输出
	SCL_OUT();
  SCL = 0;
  SDA = 1;
  delay_us(30);
  SCL = 1;
  delay_us(30);
  SDA = 0;
}

void start_light_sensor_mearsure(void)
{
	uint16 temp_version;
 
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_W);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
	}
	delay_us(100);
	i2c_pic_write(0x80);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
	}
	delay_us(100);
	i2c_pic_write(0x03);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
	}
	i2c_pic_stop();
}

uint8 read_light_sensors_time(void)
{
	uint8 time;
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_W);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	delay_us(100);
	i2c_pic_write(0x81);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	i2c_pic_stop();
	delay_us(100);
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_R);
	time = i2c_pic_read();
	GIVE_PIC_NACK();
	delay_us(30);
	i2c_pic_stop(); 
	return time;
}

uint8 read_light_sensors_gain(void)
{
	uint8 time;
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_W);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	delay_us(100);
	i2c_pic_write(0x87);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	i2c_pic_stop();
	delay_us(100);
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_R);
	time = i2c_pic_read();
	GIVE_PIC_NACK();
	delay_us(30);
	i2c_pic_stop(); 
	return time;
}

uint16 read_light_sensors_data0(void)
{
	uint16 data;
	uint8 time[2];
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_W);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	delay_us(100);
	i2c_pic_write(0x94);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	i2c_pic_stop();
	delay_us(100);
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_R);
	time[0] = i2c_pic_read();
	GIVE_PIC_ACK();
	delay_us(30);
	time[1] = i2c_pic_read();
	GIVE_PIC_NACK();
	delay_us(30);
	i2c_pic_stop(); 
	data = (uint16)time[1]<<8 | time[0];
	return data;
}

uint16 read_light_sensors_data1(void)
{
	uint16 data;
	uint8 time[2];
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_W);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	delay_us(100);
	i2c_pic_write(0x95);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	i2c_pic_stop();
	delay_us(100);
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_R);
	time[0] = i2c_pic_read();
	GIVE_PIC_ACK();
	delay_us(30);
	time[1] = i2c_pic_read();
	GIVE_PIC_NACK();
	delay_us(30);
	i2c_pic_stop(); 
	data = (uint16)time[1]<<8 | time[0];
	return data;
}

bit read_light_sensor_version(void)
{
	uint16 temp_version;
 
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_W);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	delay_us(100);
	i2c_pic_write(0x92);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	//delay_us(30);
	i2c_pic_stop();
	delay_us(100);
	i2c_pic_start();
	i2c_pic_write(LIGHT_SENSOR_ADDR_R);
	temp_version = i2c_pic_read();
	GIVE_PIC_NACK();
	delay_us(30);
	i2c_pic_stop(); 
	
	if((temp_version & 0x70)!= 0)
		return 1;
	else 
		return 0;
}

bit read_humidity_sensor_version(void)
{
	uint16 temp_version, temp_check;

	serial_int_flag = 0;
 
	i2c_pic_start();
	i2c_pic_write(CMD_READ_VERSION);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop(); 
		if(serial_int_flag == 1)
			return 1;
		else
			return 0;
	}
	
	delay_us(30);
	temp_version = i2c_pic_read();
	GIVE_PIC_ACK();
	delay_us(30);
	temp_check = i2c_pic_read();
	i2c_pic_stop();  
	if((temp_check == 0x69) && (serial_int_flag == 0))
	{
		humidity_version = temp_version & 0x00ff;
		if(humidity_version > 100)  return 0;
		else
			return 1;
	}
	else
	{ 
	 		return 0; 
	}
}

void humidity_check_online(void)
{
//	if(read_humidity_sensor_version())
//		temperature_sensor_select = EXTERNAL_TEMPERATURE_SENSOR;
//	else
//		temperature_sensor_select = INTERNAL_TEMPERATURE_SENSOR;
}

 
/****************************************************************************\
*       for the version number less than 24
\****************************************************************************/
bit read_humidity_sensor(void)
{ 
	uint16 data_buf[5] = {0};
	uint16 checksum;

    serial_int_flag = 0; //set a flag, detect if get a serial port interrupt during I2C rountine.if this flag be set to 1, do not accept the received data

	i2c_pic_start();
	i2c_pic_write(CMD_READ_PARAMS);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	delay_us(30);
	data_buf[0] = i2c_pic_read();	// humidity
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[1] = i2c_pic_read();	// temperature
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[2] = i2c_pic_read();	// humidity frequency
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[3] = i2c_pic_read();	// humidity table counter
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[4] = i2c_pic_read();	// checksum
	i2c_pic_stop();

	checksum = data_buf[0] + data_buf[1] + data_buf[2] + data_buf[3];
	if((data_buf[4] == checksum) && (serial_int_flag == 0))
	{
		HumSensor.ad[0] = data_buf[0];	//humidity
		HumSensor.ad[1] = data_buf[1];	//temperature
		HumSensor.frequency = data_buf[2];
		HumSensor.counter = data_buf[3];

//		HumSensor.temperature_f = HumSensor.temperature_c * 9 / 5 + 320;
//		if(HumSensor.humidity > 1000)
//			HumSensor.humidity = 1000;
//		
//		if(output_auto_manual & 0x02)
//			HumSensor.humidity = output_manual_value_humidity;

		return 1;    
	}
	else
		return 0;  
}


bit pic_calibrate_temperature(int16 temp)
{
	uint16 checksum = 0;

	serial_int_flag = 0;

	i2c_pic_start();
	i2c_pic_write(CMD_TEMP_CAL);
	checksum += CMD_TEMP_CAL;
	delay_us(10);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(temp >> 8);
	checksum += (temp >> 8);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(temp & 0x00ff);
	checksum += (temp & 0x00ff);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	delay_us(30);
	if(i2c_pic_read() == checksum)
	{
		i2c_pic_stop();
		if(serial_int_flag == 1)
			return 0;
		else
			return 1;
	}
	else
	{
		i2c_pic_stop();
		return 0;
	}
}

bit pic_calibrate_humidity(uint16 hum)
{
	uint16 checksum = 0;

	serial_int_flag = 0;

	i2c_pic_start();

	i2c_pic_write(CMD_HUM_CAL);
	checksum += CMD_HUM_CAL;
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(hum >> 8);
	checksum += (hum >> 8);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(hum & 0x00ff);
	checksum += (hum & 0x00ff);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	delay_us(30);
	if(i2c_pic_read() == checksum)
	{
		i2c_pic_stop();
		if(serial_int_flag == 1)
			return 0;
		else
			return 1;
	}
	else
	{
		i2c_pic_stop();
		return 0;
	}
}

bit pic_write_table(uint16 hum, uint16 freq)
{
	uint16 checksum = 0;

	serial_int_flag = 0;

	i2c_pic_start();

	i2c_pic_write(CMD_OVERRIDE_CAL);
	checksum += CMD_OVERRIDE_CAL;
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(hum >> 8);
	checksum += (hum >> 8);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(hum & 0x00ff);
	checksum += (hum & 0x00ff);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(freq >> 8);
	checksum += (freq >> 8);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(freq & 0x00ff);
	checksum += (freq & 0x00ff);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	delay_us(30);
	if(i2c_pic_read() == checksum)
	{
		i2c_pic_stop();
		if(serial_int_flag == 1)
			return 0;
		else
			return 1;
	}
	else
	{
		i2c_pic_stop();
		return 0;
	}
}

bit clear_hum_table(void)
{
	serial_int_flag = 0;

	i2c_pic_start();
	i2c_pic_write(CMD_CLEAR_TABLE);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	delay_us(30);
	if(i2c_pic_read() == CMD_CLEAR_TABLE)
	{
		i2c_pic_stop();
		if(serial_int_flag == 1)
			return 0;
		else
			return 1;
	}
	else
	{
		i2c_pic_stop();
		return 0;
	}
}



bit read_calibration_point(uint8 pt, uint16 *hum, uint16 *freq)
{ 
	uint16 data_buf[3] = {0};
	uint16 checksum;

    serial_int_flag = 0; //set a flag, detect if get a serial port interrupt during I2C rountine.if this flag be set to 1, do not accept the received data

	i2c_pic_start();
	i2c_pic_write(CMD_READ_CAL_PT);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}

	i2c_pic_write(pt);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}

	delay_us(30);
	data_buf[0] = i2c_pic_read();	// humidity
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[1] = i2c_pic_read();	// frequency
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[2] = i2c_pic_read();	// checksum
	GIVE_PIC_ACK();
	i2c_pic_stop();

	checksum = data_buf[0] + data_buf[1];
	if((data_buf[2] == checksum) && (serial_int_flag == 0))
	{
		*hum = data_buf[0];
		*freq = data_buf[1];
		return 1;    
	}
	else
		return 0;  
}

// bit pic_write_sn18(uint16 sn)
// {
//    uint16 checksum = 0;
//     
//    i2c_pic_start();

//    i2c_pic_write(0xee);
//    checksum += 0xee;
//    delay_us(10);
//    if(GET_ACK())
//    {
//       i2c_pic_stop();
//       return 0;   
//    }

//    i2c_pic_write(sn >> 8);
//    checksum += (sn >> 8);
//    delay_us(10);
//    if(GET_ACK())
//    {
//       i2c_pic_stop();
//       return 0;   
//    }

//    i2c_pic_write(sn & 0x00ff);
//    checksum += (sn & 0x00ff);
//    delay_us(10);
//    if(GET_ACK())
//    {
//       i2c_pic_stop();
//       return 0;   
//    }
//    delay_us(10);

//    if(i2c_pic_read() == checksum)
//    {
//       i2c_pic_stop();
//       return 1;
//    }
//    else
//    {
//       i2c_pic_stop();
//       return 0;
//    }
// }

 bit pic_read_sn18(uint16 *sn)
 {
 	uint16 data_buf[3] ;
 	uint16 checksum;
 	 
     i2c_pic_start();
 	
 	i2c_pic_write(0xef);
 	delay_us(30);

 	if(GET_ACK())
 	{
 		i2c_pic_stop(); 
 		return 0;
 	}
 	delay_us(30);

 	data_buf[0] = i2c_pic_read();   //the byte_high
 	GIVE_PIC_ACK();
 	delay_us(30);

 	data_buf[1] = i2c_pic_read();   //the byte_low
 	GIVE_PIC_ACK();
 	delay_us(30);

 	data_buf[2] = i2c_pic_read();   //the check data
 	GIVE_PIC_ACK();
 	delay_us(30);
 									 
 	i2c_pic_stop(); 

 	checksum = (data_buf[0] + data_buf[1]) & 0xFFFF;

 	if(data_buf[2] == checksum)
 	{
 		*sn = ((uint16)data_buf[0] << 8) + data_buf[1];
 		return 1;
 	}
 	else
 		return 0; 
 }  


/****************************************************************************\
*       for the version 24
\****************************************************************************/
bit read_humidity_sensor_Rev24(uint8 Addr, uint8 CMD)
{ 
	uint16 data_buf[5] = {0};
	uint16 checksum;

    serial_int_flag = 0; //set a flag, detect if get a serial port interrupt during I2C rountine.if this flag be set to 1, do not accept the received data

	i2c_pic_start();
	i2c_pic_write(Addr);
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	
	i2c_pic_write(CMD);     		//COMMAND
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}  
	
	delay_us(30);
	data_buf[0] = i2c_pic_read();	// humidity
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[1] = i2c_pic_read();	// temperature
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[2] = i2c_pic_read();	// humidity frequency
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[3] = i2c_pic_read();	// humidity table counter
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[4] = i2c_pic_read();	// checksum
	i2c_pic_stop();

	checksum = data_buf[0] + data_buf[1] + data_buf[2] + data_buf[3];
	if((data_buf[4] == checksum) && (serial_int_flag == 0))
	{
//		HumSensor.humidity = data_buf[0];
//		HumSensor.temperature_c = data_buf[1];
		HumSensor.ad[0] = data_buf[0];	//humidity
		HumSensor.ad[1] = data_buf[1];	//temperature
		HumSensor.frequency = data_buf[2];
		HumSensor.counter = data_buf[3];

//		HumSensor.temperature_f = HumSensor.temperature_c * 9 / 5 + 320;
//		if(HumSensor.humidity > 1000)
//			HumSensor.humidity = 1000;
//		
//		if(output_auto_manual & 0x02)
//			HumSensor.humidity = output_manual_value_humidity;

		return 1;    
	}
	else
		return 0;  
}
bit read_humtab_Rev24(uint8 Addr, uint8 CMD, uint8 pt, uint16 *hum, uint16 *freq)
{ 
	uint16 data_buf[3] = {0};
	uint16 checksum;

    serial_int_flag = 0; //set a flag, detect if get a serial port interrupt during I2C rountine.if this flag be set to 1, do not accept the received data

	i2c_pic_start();
	i2c_pic_write(Addr);		//IIC ADDRESS
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}

	i2c_pic_write(CMD);			//COMMAND
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}
	
	i2c_pic_write(pt);			//INDEX
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;
	}

	delay_us(30);
	data_buf[0] = i2c_pic_read();	// humidity
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[1] = i2c_pic_read();	// frequency
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[2] = i2c_pic_read();	// checksum
	GIVE_PIC_ACK();
	i2c_pic_stop();

	checksum = data_buf[0] + data_buf[1];
	if((data_buf[2] == checksum) && (serial_int_flag == 0))
	{
		*hum = data_buf[0];
		*freq = data_buf[1];
		return 1;    
	}
	else
		return 0;  
}

/***********************************************************\
* MDF 15/06/04
* Read parameters from pic
* CMD_CONFIG_READ,CMD_SN_READ
\***********************************************************/
bit read_params_Rev24(uint8 Addr, uint8 CMD, uint16 *params) 
{
  	uint16 data_buf[3] = {0};
  	uint16 checksum;
 	serial_int_flag = 0;	 
    i2c_pic_start();
	
	i2c_pic_write(Addr);			//IIC ADDRESS 
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		*params = 10;
		return 0;	
	}
	
	i2c_pic_write(CMD);     		//COMMAND
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		*params  = 11;
		return 0;	
	} 

	data_buf[0] = i2c_pic_read();   //the data
	GIVE_PIC_ACK();
	delay_us(30);

	data_buf[1] = i2c_pic_read();   //none
	GIVE_PIC_ACK();
	delay_us(30);

	data_buf[2] = i2c_pic_read();   //the check data
	GIVE_PIC_ACK();
	delay_us(30);
									 
	i2c_pic_stop(); 

	checksum = (data_buf[0] + data_buf[1]) & 0xFFFF;
	if((data_buf[2] == checksum) && (serial_int_flag == 0))
	{
		*params = data_buf[0];
		return 1;    
	}
	else
	{
		*params = 12; 
		return 0; 
	}
}  
/***********************************************************\
* MDF 15/06/04
* Set parameters to pic
* CMD_HEATING_CONTROL; CMD_CLEAR_TABLE;CMD_TEMP_CAL; CMD_HUM_CAL;CMD_SN_WRITE
* CMD_TAB_SEL; CMD_USER_TAB_PT_SET; CMD_OVERRIDE_USER_CAL
\***********************************************************/
bit write_sensor_Rev24(uint8 Addr, uint8 CMD, int16 params)   
{
	uint16 checksum = 0;

	serial_int_flag = 0;

	i2c_pic_start();
	
	i2c_pic_write(Addr);			//IIC ADDRESS 
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}
	
	i2c_pic_write(CMD);     		//COMMAND
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}
	
	i2c_pic_write(Addr + CMD);		//CHECK SUM
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(params >> 8);   // high byte
	checksum = params >> 8;
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(params);		  // low byte
	checksum += (uint8)params;
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	delay_us(30);
	
	if(i2c_pic_read() == checksum)	//CHECK SUM
	{
		i2c_pic_stop();
		if(serial_int_flag == 1)
			return 0;
		else
			return 1;
	}
	else
	{ 
		i2c_pic_stop();
		return 0;
	}
}

/***********************************************************\
* MDF 15/06/04
* Set calibrate point to pic
* CMD_OVERRIDE_CAL
\***********************************************************/
bit write_humtable_Rev24(uint8 Addr, uint8 CMD, uint16 hum, uint16 freq)
{
	uint16 checksum = 0;

	serial_int_flag = 0;
 
	i2c_pic_start();
	
	i2c_pic_write(Addr);			//IIC ADDRESS 
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}
	
	i2c_pic_write(CMD);     		//COMMAND
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}
	
	i2c_pic_write(Addr + CMD);		//CHECK SUM
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(hum >> 8);		//humidity high byte
	checksum = (hum >> 8)&0x00ff ;
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(hum);
	checksum += hum & 0x00ff;       // humidity low byte
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	}

	i2c_pic_write(freq >> 8);      // frequency high byte
	checksum += (freq >> 8)&0x00ff;
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	} 
	i2c_pic_write(freq);		   // frequency high byte
	checksum += freq & 0x00ff;
	delay_us(30);
	if(GET_ACK())
	{
		i2c_pic_stop();
		return 0;	
	} 
	delay_us(30);
	if(i2c_pic_read() == checksum)
	{
		i2c_pic_stop();
		if(serial_int_flag == 1)
			return 0;
		else
			return 1;
	}
	else
	{
		i2c_pic_stop();
		return 0;
	}
}

/****************************************************************************\
*       for the version number start from 25, add dew point
\****************************************************************************/
/***********************************************************\
* read humidity,temperature,frequency and points from sensor
* CMD_READ_CAL_PT, number = 0~9
\***********************************************************/
bit read_sensor_Rev25(uint8 Addr, uint8 CMD)	 
{	    
	uint16 data_buf[6] = {0};
	uint16 checksum; 
    serial_int_flag = 0; //set a flag, detect if get a serial port interrupt during I2C rountine.if this flag be set to 1, do not accept the received data

	i2c_pic_start();
	i2c_pic_write(Addr);			//IIC ADDRESS 
	delay_us(30);
	if(GET_ACK())
	{ 
		i2c_pic_stop();
		return 0;	
	}
	
	i2c_pic_write(CMD);     		//COMMAND
	delay_us(30);
	if(GET_ACK())
	{ 
		i2c_pic_stop();
		return 0;	
	}  
	data_buf[0] = i2c_pic_read();	// humidity
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[1] = i2c_pic_read();	// temperature
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[2] = i2c_pic_read();	// humidity frequency
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[3] = i2c_pic_read();	// humidity table counter
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[4] = i2c_pic_read();	// dew point
	GIVE_PIC_ACK();
	delay_us(30);
	data_buf[5] = i2c_pic_read();	// checksum
	i2c_pic_stop();
	 
	checksum = data_buf[0] + data_buf[1] + data_buf[2] + data_buf[3] + data_buf[4];
	if((data_buf[5] == checksum) && (serial_int_flag == 0))
	{
//		HumSensor.humidity = data_buf[0];
//		HumSensor.temperature_c = data_buf[1];
		HumSensor.ad[0] = data_buf[0];	//humidity
		HumSensor.ad[1] = data_buf[1];	//temperature
		HumSensor.frequency = data_buf[2];
		HumSensor.counter = data_buf[3]; 
//		HumSensor.temperature_f = HumSensor.temperature_c * 9 / 5 + 320;
//		if(HumSensor.humidity > 1000)
//			HumSensor.humidity = 1000;
//		
//		if(output_auto_manual & 0x02)
//			HumSensor.humidity = output_manual_value_humidity;

		return 1;    
	}
	else
		return 0;
}



//////// APP FUNCTIONS
void update_humidity_display(uint8 ForceUpdate)
{
	uint16 pre_disp_humidity = 0;

	if(isColorScreen== false)
	{
		if((humidity_version == 111) || (humidity_version == 10) || (humidity_version == 222))
			return;

		if(ForceUpdate || (pre_disp_humidity != HumSensor.humidity))
		{
			Lcd_Show_Data(0, 12, HumSensor.humidity, 1, ALIGN_RIGHT, DISP_NOR);
			pre_disp_humidity = HumSensor.humidity;
		}

		if(ForceUpdate)
			Lcd_Write_Char(0, 13, '%', DISP_NOR);
	}
}

bit read_sensor(void)
{
	if(humidity_version < 24)
		return read_humidity_sensor();
	else if(humidity_version == 24)
		return read_humidity_sensor_Rev24(IIC_ADDR, CMD_READ_PARAMS);
	else //if(humidity_version == 25)
		return read_sensor_Rev25(IIC_ADDR, CMD_READ_PARAMS);
//	else
//		return 0;
}

bit write_sensor_temp(int16 cal_temp)
{
	
	if(humidity_version < 24)
		return pic_calibrate_temperature(cal_temp);
	else //if(humidity_version <= 25)
		return write_sensor_Rev24(IIC_ADDR, CMD_TEMP_CAL, cal_temp);
//	else
//		return 0;
}

bit write_sensor_humtable(uint16 hum, uint16 freq)
{
	if(humidity_version < 24)
		return pic_write_table(hum, freq);
	else //if(humidity_version <= 25)
		return write_humtable_Rev24(IIC_ADDR, CMD_OVERRIDE_CAL, hum, freq);
//	else
//		return 0;
}

bit read_sensor_humtab(uint8 pt, uint16 *hum, uint16 *freq)
{
	if(humidity_version < 24)
		return read_calibration_point(pt, hum, freq);
	else //if(humidity_version <= 25)
		return read_humtab_Rev24(IIC_ADDR, CMD_READ_CAL_PT, pt, hum, freq);
//	else
//		return 0;
}

bit clear_sensor_humtab(void)
{
	if(humidity_version < 24)
		return clear_hum_table();
	else //if(humidity_version <= 25)
		return write_sensor_Rev24(IIC_ADDR, CMD_CLEAR_TABLE, 1);
//	else
//		return 0;
}

bit write_sensor_humidity(uint16 hum)
{
	if(humidity_version < 24)
		return pic_calibrate_humidity(hum);
	else //if(humidity_version <= 25)
		return write_sensor_Rev24(IIC_ADDR, CMD_HUM_CAL, hum);
//	else
//		return 0;
}

 bit pic_read_sn(uint16 *sn)
 {
 	if(humidity_version < 24)
 		return pic_read_sn18(sn);
 	else //if(humidity_version <= 25)
 		return read_params_Rev24(IIC_ADDR, CMD_SN_READ, sn);
// 	else
// 		return 0;
 }

// bit pic_write_sn(uint16 sn)
// {
// 	if(humidity_version < 24)
// 		return pic_write_sn18(sn);
// 	else if(humidity_version <= 25)
// 		return write_sensor_Rev24(IIC_ADDR, CMD_SN_WRITE, sn);
// 	else
// 		return 0;
// }
bit pic_heating_control(uint8 cmd) 
{
	if(humidity_version > 24)
		return write_sensor_Rev24(IIC_ADDR,CMD_HEATING_CONTROL,cmd);
	else
		return 0;
}
bit pic_read_light_val(uint16 *val)
{
	return read_params_Rev24(IIC_ADDR, CMD_LIGHT_READ, val);
}
void external_operation(void)
{  
	switch(external_operation_flag)
	{
		case TEMP_CALIBRATION:  
				HumSensor.offset_t += external_operation_value - HumSensor.temperature_c ;
				 
				external_operation_flag = 0;
				new_write_eeprom(EEP_TEMP_OFFSET,HumSensor.offset_t); 
				new_write_eeprom(EEP_TEMP_OFFSET+1,HumSensor.offset_t>>8);  
			break;
		case HUM_CALIBRATION: 
				if(hum_exists == 1)
				{
					if(table_sel== USER)
					{
						HumSensor.offset_h = external_operation_value- humidity_back;
						new_write_eeprom(EEP_HUM_OFFSET,HumSensor.offset_h); 
						new_write_eeprom(EEP_HUM_OFFSET+1,HumSensor.offset_h>>8);
					}
					else
					{
						HumSensor.offset_h_default = external_operation_value - humidity_back;
						new_write_eeprom(EEP_CAL_DEFAULT_HUM,HumSensor.offset_h_default); 
						new_write_eeprom(EEP_CAL_DEFAULT_HUM+1,HumSensor.offset_h_default>>8); 
					}
				}
				else if(hum_exists == 2)
				{
					HumSensor.offset_h = external_operation_value - hum_org;
					new_write_eeprom(EEP_HUM_OFFSET,HumSensor.offset_h); 
					new_write_eeprom(EEP_HUM_OFFSET+1,HumSensor.offset_h>>8);
				}
				external_operation_flag = 0; 
			break;
				
			case HUM_HEATER:
				if(hum_exists == 2)
				{
					if(external_operation_value == 1)
						SHT3X_EnableHeater();
					else if(external_operation_value == 0)
						SHT3X_DisableHeater();
					external_operation_flag = 0;
					hum_heat_status = external_operation_value;
			
				}
				else
				{
					if(pic_heating_control((uint8)external_operation_value))
					{
						external_operation_flag = 0;
						hum_heat_status = external_operation_value;
					}
				}
			break;
		default:
			break;
	}
 
}

void auto_heating(void)
{ 
	static int8 read_count;
	if(display_state == PIC_NORMAL)
	{
		if(hum_heat_status == 0)
		{
			if(HumSensor.humidity > 950) 	//95%
			{
				if(read_count < 120)
					read_count++;  			
				else  						//every ten minutes
				{
					if(hum_exists == 2)
					{
						if(SHT3X_EnableHeater() == 0) 
						{
							read_count = 0;
							hum_heat_status = 1;
						}					
					}
					else
					{
						if(pic_heating_control(1)) 
						{
							read_count = 0;
							hum_heat_status = 1;
						}
					}
				}
			}
			else
				read_count = 0; 
		}
		else
		{
			if(read_count < 24) //1 minutes
				read_count++;
			else
			{
				if(hum_exists == 2)
				{
					if(SHT3X_DisableHeater() == 0) 
					{
						read_count = 0;
						hum_heat_status = 0;
					}					
				}
				else
				{
					if(pic_heating_control(0)) 
					{
						read_count = 0;
						hum_heat_status = 0;
					}
				}
			} 
		} 	
	}
	else
	{
		read_count = 0;
	} 
	
//	test[2] = read_count;

}




