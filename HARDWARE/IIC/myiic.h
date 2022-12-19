#ifndef __MYIIC_H
#define __MYIIC_H

#include "bitmap.h"
   	   		   
////IO��������
//#define SDA_IN()	{GPIOB->CRL &= 0XFFFFFFF0; GPIOB->CRL |= ((u32)8 << 0);}
//#define SDA_OUT()	{GPIOB->CRL &= 0XFFFFFFF0; GPIOB->CRL |= ((u32)3 << 0);}

//IO��������	 
/*#define IIC_SCL		PAout(3)	//SCL
#define IIC_SDA		PAout(2)	//SDA	 
#define READ_SDA	PAin(2)		//����SDA
#define READ_SCL  PAin(3)*/
//#define IIC_WP		PCout(5)

extern uint8_t i2c_index;
extern uint8_t i2c_sensor_type[3];//i2c_sht_exist[3];
extern uint8_t i2c_sht_num;
typedef struct 
{
	float tem_org;
	float hum_org;
	uint16_t co2_org;
}Str_Mul_I2C;

extern Str_Mul_I2C I2C_Sensor[3];


//IIC���в�������
void IIC_Init(void);				//��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);				//����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void);				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

//void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
//u8 IIC_Read_One_Byte(u8 daddr,u8 addr);
void SDA_IN(void);
void SDA_OUT(void);
void SCL_IN(void);
void SCL_OUT(void);

void IIC_SDA_HI(void);
void IIC_SDA_LO(void);
void IIC_SCL_HI(void);
void IIC_SCL_LO(void);
uint8_t READ_SDA(void);
uint8_t READ_SCL(void);

#endif
