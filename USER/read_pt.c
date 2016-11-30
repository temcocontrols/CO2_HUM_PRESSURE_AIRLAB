#include "stdio.h"
#include "read_pt.h"
#include "pt12_i2c.h"
#include "delay.h"

#ifdef T3PT12



//union {
//		u8 qauter[64] ;
//		u16  half [32];
//		u32  word [16];

//	}	rs_data ;


//RS_DATA  rs_data ;

u32 rs_data[16] ;

//one time read 40 bytes for 10 channel ad value
void read_rtd_bytes(u8 addr)
{
	u8 i;
	u8 iic_buf[51]; 
	u8   checksum = 0;
 
	PT12_I2C_start();
	PT12_I2C_write_byte((addr << 1) | IIC_READ);
	if(PT12_I2C_wait_for_ack() == ACK)
	{
		for(i = 0; i < 50; i++)
		{
			iic_buf[i] = PT12_I2C_read_byte(ACK);
			checksum += iic_buf[i];
			
			//delay_ms(1);
		}
		iic_buf[50] = PT12_I2C_read_byte(NACK); //no need give ACK
		
	}
	PT12_I2C_stop();

if(checksum == iic_buf[50])
	{
		rs_data[4] = (iic_buf[2]<<24)|(iic_buf[3]<<16)|(iic_buf[4]<<8)|(iic_buf[5]) ;
		rs_data[5] = (iic_buf[6]<<24)|(iic_buf[7]<<16)|(iic_buf[8]<<8)|(iic_buf[9]) ;
		rs_data[6] = (iic_buf[10]<<24)|(iic_buf[11]<<16)|(iic_buf[12]<<8)|(iic_buf[13]) ;
		rs_data[7] = (iic_buf[14]<<24)|(iic_buf[15]<<16)|(iic_buf[16]<<8)|(iic_buf[17]) ;
		rs_data[8] = (iic_buf[18]<<24)|(iic_buf[19]<<16)|(iic_buf[20]<<8)|(iic_buf[21]) ;
		rs_data[9] = (iic_buf[22]<<24)|(iic_buf[23]<<16)|(iic_buf[24]<<8)|(iic_buf[25]) ;
		rs_data[10] = (iic_buf[26]<<24)|(iic_buf[27]<<16)|(iic_buf[28]<<8)|(iic_buf[29]) ;
		rs_data[11] = (iic_buf[30]<<24)|(iic_buf[31]<<16)|(iic_buf[32]<<8)|(iic_buf[33]) ;
		rs_data[12] = (iic_buf[34]<<24)|(iic_buf[35]<<16)|(iic_buf[36]<<8)|(iic_buf[37]) ;
		rs_data[13] = (iic_buf[38]<<24)|(iic_buf[39]<<16)|(iic_buf[40]<<8)|(iic_buf[41]) ;
		rs_data[14] = (iic_buf[42]<<24)|(iic_buf[43]<<16)|(iic_buf[44]<<8)|(iic_buf[45]) ;
		rs_data[15] = (iic_buf[46]<<24)|(iic_buf[47]<<16)|(iic_buf[48]<<8)|(iic_buf[49]) ;
	
	}
}
//read 4 point calibration data when startup the board
u8 read_celpoint_bytes(unsigned char addr)
{
	u8 i;
	u8 iic_buf[51];
	u8 checksum = 0;
	PT12_I2C_start();
	PT12_I2C_write_byte((addr << 1) | IIC_READ);
	if(PT12_I2C_wait_for_ack() == ACK)
	{
		for(i = 0; i < 50; i++)
		{
			iic_buf[i] = PT12_I2C_read_byte(ACK);
			checksum += iic_buf[i];
			delay_ms(1);
		}
		iic_buf[50] = PT12_I2C_read_byte(NACK); //no need give ACK
	} 
	PT12_I2C_stop();	
	if(checksum == iic_buf[50])
	{	
			
		rs_data[0] = (iic_buf[2]<<24)|(iic_buf[3]<<16)|(iic_buf[4]<<8)|(iic_buf[5]) ;
		rs_data[1] = (iic_buf[6]<<24)|(iic_buf[7]<<16)|(iic_buf[8]<<8)|(iic_buf[9]) ;
		rs_data[2] = (iic_buf[10]<<24)|(iic_buf[11]<<16)|(iic_buf[12]<<8)|(iic_buf[13]) ;
		rs_data[3] = (iic_buf[14]<<24)|(iic_buf[15]<<16)|(iic_buf[16]<<8)|(iic_buf[17]) ;
		return 1 ;
	}
	return 0 ;
 
}
void write_rtd_bytes(unsigned char addr, unsigned char cmd)
{
	unsigned char  i ,i2c_buff[3];
	unsigned char  checksum;
	i2c_buff[0] = SYNC_HEAD;
	i2c_buff[1] = cmd;
	checksum = SYNC_HEAD + cmd;
	i2c_buff[2] = checksum;	
	PT12_I2C_start();
	PT12_I2C_write_byte((addr << 1) | IIC_WRITE);
	for(i = 0; i < 3; i++)
	{	
		delay_us(100);
		if(PT12_I2C_wait_for_ack() == NACK)
		{
			break;
			 	
		}
		PT12_I2C_write_byte(i2c_buff[i]);
		
	}
	PT12_I2C_stop();	
}
//when startup the board ,we read the four point calibration data to caculate the linear
u8 init_celpoint(void)
{
	write_rtd_bytes(IIC_RTD_ADDR , CALI_AD_VALUE);
	delay_ms(3);
	return read_celpoint_bytes(IIC_RTD_ADDR);

}

//one time we read 40 bytes for ten channel RTD data. 
void read_rtd_data (void)
{
	static uint8_t read_status = 0 ;
//	switch(read_status)
//	{
//		case 0: 
//		write_rtd_bytes(IIC_RTD_ADDR , TEN_CHANNAL_AD_VALUE);
//		read_status = 1 ;
//		
//		case 1:
//			read_rtd_bytes(IIC_RTD_ADDR);
//			read_status = 0 ;
//		break ;
//	}
	write_rtd_bytes(IIC_RTD_ADDR , TEN_CHANNAL_AD_VALUE);
	delay_ms(3);
	read_rtd_bytes(IIC_RTD_ADDR);
		
}


#endif

