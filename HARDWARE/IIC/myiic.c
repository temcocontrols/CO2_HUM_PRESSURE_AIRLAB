#include "myiic.h"
#include "delay.h"
#include "temperature.h"

uint8_t i2c_index = 0;
//uint8_t i2c_sht_exist[3] = { 0 , 0, 0};
uint8_t i2c_sht_num = 0;


Str_Mul_I2C I2C_Sensor[3];


//IO方向设置
//#define SDA_IN()	{GPIOB->CRL &= 0XFFFFFFF0; GPIOB->CRL |= ((u32)8 << 0);}
//#define SDA_OUT()	{GPIOB->CRL &= 0XFFFFFFF0; GPIOB->CRL |= ((u32)3 << 0);} 
// SCL -> PA3    SDA -> PA2
void SDA_IN(void)
{
	if(i2c_index == 0)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_SetBits(GPIOA, GPIO_Pin_2);
	}
	else if(i2c_index == 1)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_SetBits(GPIOB, GPIO_Pin_11);
	}
	else if(i2c_index == 2)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_SetBits(GPIOC, GPIO_Pin_8);
	}
}

void SDA_OUT(void)
{
	if(i2c_index == 0)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	else if(i2c_index == 1)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}
	else if(i2c_index == 2)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
	}
}

void SCL_IN(void)
{
	if(i2c_index == 0)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_SetBits(GPIOA, GPIO_Pin_3);
	}
	else if(i2c_index == 1)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_SetBits(GPIOB, GPIO_Pin_10);
	}
	else if(i2c_index == 2)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_SetBits(GPIOC, GPIO_Pin_7);
	}
}
  
void SCL_OUT(void)
{
	if(i2c_index == 0)
	{
	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
		}
	else if(i2c_index == 1)
	{
		GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	}
	else if(i2c_index == 2)
	{
			GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	}

}
//初始化IIC
void IIC_Init(void)
{
	if(i2c_index == 0)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//PA2/PA3

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_3);
// RESET PD0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_0);		
	}
	else if(i2c_index == 1)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);//PB11 PB10

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);
	
// RESET PC9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_9);	
	}
	else if(i2c_index == 2)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//PC6 PC7 PC8

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);	
		// RESTE PC6
	}
}

void IIC_SDA_HI(void)
{
	if(i2c_index == 0)	PAout(2) = 1;
	else if(i2c_index == 1)	PBout(11) = 1;
	else if(i2c_index == 2)	PCout(8) = 1;
	else 
		PAout(2) = 1;
}

void IIC_SDA_LO(void)
{
	if(i2c_index == 0)	PAout(2) = 0;
	else if(i2c_index == 1)	PBout(11) = 0;
	else if(i2c_index == 2)	PCout(8) = 0;
	else 
		PAout(2) = 0;
}

void IIC_SCL_HI(void)
{
	if(i2c_index == 0)	PAout(3) = 1;
	else if(i2c_index == 1)	PBout(10) = 1;
	else if(i2c_index == 2)	PCout(7) = 1;
	else
		PAout(3) = 1;
}

void IIC_SCL_LO(void)
{
	if(i2c_index == 0)	PAout(3) = 0;
	else if(i2c_index == 1)	PBout(10) = 0;
	else if(i2c_index == 2)	PCout(7) = 0;
	else
		PAout(3) = 0;
}

uint8_t READ_SDA(void)
{
	if(i2c_index == 0)	return PAin(2);
	else if(i2c_index == 1)	return PBin(11);
	else if(i2c_index == 2)	return PCin(8);
	else
		return PAin(2);
}

uint8_t READ_SCL(void)
{
	if(i2c_index == 0)	return PAin(3);
	else if(i2c_index == 1)	return PBin(10);
	else if(i2c_index == 2)	return PCin(7);
	else 
		return PAin(3);
}

//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();		//sda线输出
	IIC_SDA_HI();	  	  
	IIC_SCL_HI();
	delay_us(4*hum_read_delay);
 	IIC_SDA_LO();	//START:when CLK is high,DATA change form high to low 
	delay_us(4*hum_read_delay);
	IIC_SCL_LO();	//钳住I2C总线，准备发送或接收数据 
}
	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();		//sda线输出
	IIC_SCL_LO();
	IIC_SDA_LO();	//STOP:when CLK is high DATA change form low to high
 	delay_us(4*hum_read_delay);
	IIC_SCL_HI(); 
	IIC_SDA_HI();	//发送I2C总线结束信号
	delay_us(4*hum_read_delay);							   	
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime = 0;
	SDA_IN();		//SDA设置为输入  
	IIC_SDA_HI();
	delay_us(1*hum_read_delay);	   
	IIC_SCL_HI();
	delay_us(1*hum_read_delay);	 
	while(READ_SDA())
	{
		ucErrTime++;		
		if(ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_LO();	//时钟输出0 	   
	return 0;  
}
 
//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL_LO();
	SDA_OUT();
	IIC_SDA_LO();
	delay_us(2*hum_read_delay);
	IIC_SCL_HI();
	delay_us(2*hum_read_delay);
	IIC_SCL_LO();
}

//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL_LO();
	SDA_OUT();
	IIC_SDA_HI(); 
	delay_us(2*hum_read_delay);
	IIC_SCL_HI(); 
	delay_us(2*hum_read_delay);
	IIC_SCL_LO();
}
					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL_LO();		//拉低时钟开始数据传输
    for(t = 0; t < 8; t++)
    {         
				if((txd & 0x80) >> 7)	IIC_SDA_HI(); 
			else 
				IIC_SDA_LO(); 
			
        txd <<= 1; 	  
		delay_us(2*hum_read_delay);   //对TEA5767这三个延时都是必须的
		IIC_SCL_HI(); 
		delay_us(2*hum_read_delay); 
		IIC_SCL_LO();	
		delay_us(2*hum_read_delay);
    }	 
}
extern   u16  Test[50];  
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	
	SDA_IN();			//SDA设置为输入
	
    for(i = 0; i < 8; i++)
	{
        IIC_SCL_LO(); 
        delay_us(2*hum_read_delay);
		IIC_SCL_HI(); 
        receive <<= 1;
        if(READ_SDA())receive++;   
		delay_us(1*hum_read_delay); 
    }
					 
    if(!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK
		
    return receive;
}



