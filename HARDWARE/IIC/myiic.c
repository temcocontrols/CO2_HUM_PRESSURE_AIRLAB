#include "myiic.h"
#include "delay.h"
#include "temperature.h"

//IO��������
//#define SDA_IN()	{GPIOB->CRL &= 0XFFFFFFF0; GPIOB->CRL |= ((u32)8 << 0);}
//#define SDA_OUT()	{GPIOB->CRL &= 0XFFFFFFF0; GPIOB->CRL |= ((u32)3 << 0);} 
  void SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
}
  void SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOA, GPIO_Pin_2);
}

  void SCL_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
}
  void SCL_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

}
//��ʼ��IIC
void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//PA2/PA3

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_3);
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//ʹ��GPIOBʱ��
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOC, GPIO_Pin_5);
}

//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();		//sda�����
	IIC_SDA = 1;	  	  
	IIC_SCL = 1;
	delay_us(4*hum_read_delay);
 	IIC_SDA = 0;	//START:when CLK is high,DATA change form high to low 
	delay_us(4*hum_read_delay);
	IIC_SCL = 0;	//ǯסI2C���ߣ�׼�����ͻ�������� 
}
	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();		//sda�����
	IIC_SCL = 0;
	IIC_SDA = 0;	//STOP:when CLK is high DATA change form low to high
 	delay_us(4*hum_read_delay);
	IIC_SCL = 1; 
	IIC_SDA = 1;	//����I2C���߽����ź�
	delay_us(4*hum_read_delay);							   	
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime = 0;
	SDA_IN();		//SDA����Ϊ����  
	IIC_SDA = 1;
	delay_us(1*hum_read_delay);	   
	IIC_SCL = 1;
	delay_us(1*hum_read_delay);	 
	while(READ_SDA)
	{
		ucErrTime++;		
		if(ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL = 0;	//ʱ�����0 	   
	return 0;  
}
 
//����ACKӦ��
void IIC_Ack(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 0;
	delay_us(2*hum_read_delay);
	IIC_SCL = 1;
	delay_us(2*hum_read_delay);
	IIC_SCL = 0;
}

//������ACKӦ��		    
void IIC_NAck(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 1;
	delay_us(2*hum_read_delay);
	IIC_SCL = 1;
	delay_us(2*hum_read_delay);
	IIC_SCL = 0;
}
					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL = 0;		//����ʱ�ӿ�ʼ���ݴ���
    for(t = 0; t < 8; t++)
    {              
        IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1; 	  
		delay_us(2*hum_read_delay);   //��TEA5767��������ʱ���Ǳ����
		IIC_SCL = 1;
		delay_us(2*hum_read_delay); 
		IIC_SCL = 0;	
		delay_us(2*hum_read_delay);
    }	 
}
extern   u16  Test[50];  
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	
	SDA_IN();			//SDA����Ϊ����
	
    for(i = 0; i < 8; i++)
	{
        IIC_SCL = 0; 
        delay_us(2*hum_read_delay);
		IIC_SCL = 1;
        receive <<= 1;
        if(READ_SDA)receive++;   
		delay_us(1*hum_read_delay); 
    }
					 
    if(!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK
		
    return receive;
}
