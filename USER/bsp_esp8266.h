#ifndef  __BSP_ESP8266_H
#define	 __BSP_ESP8266_H



#include "stm32f10x.h"
//#include "product.h"
#include <stdio.h>
#include <stdbool.h>



#if defined ( __CC_ARM   )
#pragma anon_unions
#endif



/******************************* ESP8266 数据类型定义 ***************************/
//typedef struct
//{
//	unsigned char sta_ip[4];
//	unsigned char net_mask[4];
//	unsigned char wifi_mode;
//	unsigned short int tcp_port;
//	unsigned short int udp_port[2]; //modbus scan port 1234, bacnet 47808
//	
//}WIFI_STR;

#pragma pack(1) 
typedef struct 
{
	unsigned short int cmd;   // low byte first
	unsigned short int len;   // low byte first
	unsigned short int own_sn[4]; // low byte first
	unsigned short int product;   // low byte first
	unsigned short int address;   // low byte first
	unsigned short int ipaddr[4]; // low byte first
	unsigned short int modbus_port; // low byte first
	unsigned short int firmwarerev; // low byte first
	unsigned short int hardwarerev;  // 28 29	// low byte first
	
	unsigned char master_sn[4];  // master's SN 30 31 32 33
	unsigned short int instance_low; // 34 35 hight byte first
	unsigned char panel_number; //  36	
  char panelname[20]; // 37 - 56
	unsigned short int instance_hi; // 57 58 hight byte first
	
	unsigned char bootloader;  // 0 - app, 1 - bootloader, 2 - wrong bootloader , 3 - mstp device
	unsigned short int BAC_port;  //  hight byte first
	unsigned char zigbee_exist; // 0 - inexsit, 1 - exist
															// BIT1: 0 - NO WIFI, 1 - WIFI
	unsigned char subnet_protocal; // 0 - modbus, 12 - bip to mstp

}STR_SCAN_CMD;



typedef struct
{
	uint8_t MANUEL_EN;	
	uint8_t IP_Auto_Manual; //  0 Auto DHCP   1 static IP	  
	uint16_t modbus_port;
	uint16_t bacnet_port;	
	uint8_t IP_Wifi_Status;  // 0 no-Wifi
	uint8_t rev;
	uint8_t rssi;
	uint8_t	reserved;
	
	char name[64];
	char password[32];
	uint8_t ip_addr[4];
	uint8_t net_mask[4];
	uint8_t getway[4];
	uint8_t mac_addr[6];  // read-only
}STR_SSID;

extern STR_SSID	SSID_Info;
extern uint8_t flag_connect_AP;

typedef enum
{
	WIFI_NONE,
	WIFI_NO_WIFI,
	WIFI_NORMAL,
	WIFI_CONNECTED,
	WIFI_DISCONNECTED,
	WIFI_NO_CONNECT,
	WIFI_SSID_FAIL,
};


//#define Modbus  modbus  

//#define WIFI_CONNECTED 			1
//#define WIFI_DISCONNECTED   0

#define UCID_BACNET 0
#define UCID_SCAN 1
#define WIFI  5
#define UIP_HEAD 6

#define HTONS(n) (uint16_t)((((uint16_t) (n)) << 8) | (((uint16_t) (n)) >> 8))

//extern WIFI_STR AQ;
//extern STR_MODBUS Modbus;
extern uint8_t wifi_buf[500];
extern uint16_t wifi_len;
extern uint8_t modbus_wifi_buf[500];
extern uint16_t modbus_wifi_len;
extern uint8_t packet[1024];
extern uint8_t cStr [ 1024 ];
extern uint8_t state;
extern u32 Instance;
extern STR_SCAN_CMD Infor[20];
extern STR_SCAN_CMD Scan_Infor;
extern uint8_t transactionID[6];


uint16_t check_packet(uint8_t * str,uint8_t * dat);
void UdpData(unsigned char type);
void responseCmd(u8 type, u8* pData);
void Set_transaction_ID(unsigned char *str, unsigned short int id, unsigned short int num);

void Start_Smart_Config(void);
void Stop_Smart_Config(void);
void Restore_WIFI(void);
bool ESP8266_CWDHCP_DEF (void);

#define SENDBUFF_SIZE   600   

void dma_init_uart();
void dma_send_uart_data(uint16_t nlength);
unsigned char Send_Uart_Data(char * m_send_data, uint16_t nlength);


typedef enum{
	STA,
  AP,
  STA_AP  
} ENUM_Net_ModeTypeDef;


typedef enum{
	 enumTCP,
	 enumUDP,
} ENUM_NetPro_TypeDef;
	

typedef enum{
	Multiple_ID_0 = 0,
	Multiple_ID_1 = 1,
	Multiple_ID_2 = 2,
	Multiple_ID_3 = 3,
	Multiple_ID_4 = 4,
	Single_ID_0 = 5,
} ENUM_ID_NO_TypeDef;
	

typedef enum{
	OPEN = 0,
	WEP = 1,
	WPA_PSK = 2,
	WPA2_PSK = 3,
	WPA_WPA2_PSK = 4,
} ENUM_AP_PsdMode_TypeDef;



/******************************* ESP8266 外部全局变量声明 ***************************/
#define RX_BUF_MAX_LEN     1024                                     //最大接收缓存字节数

extern struct  STRUCT_USARTx_Fram                                  //串口数据帧的处理结构体
{
	uint8_t  Data_RX_BUF [ RX_BUF_MAX_LEN ];
	
  union {
    __IO u16 InfAll;
    struct {
		  __IO u16 FramLength       :15;                               // 14:0 
		  __IO u16 FramFinishFlag   :1;                                // 15 
	  } InfBit;
  }; 
	
} strEsp8266_Fram_Record;



/******************************** ESP8266 连接引脚定义 ***********************************/

#define      macESP8266_RST_APBxClock_FUN                     RCC_APB2PeriphClockCmd
#define      macESP8266_RST_CLK                               RCC_APB2Periph_GPIOD
#define      macESP8266_RST_PORT                              GPIOD
#define      macESP8266_RST_PIN                               GPIO_Pin_10

#define      macESP8266_USART_BAUD_RATE                       115200

#define      macESP8266_USARTx                                USART3
#define      macESP8266_USART_APBxClock_FUN                   RCC_APB1PeriphClockCmd
#define      macESP8266_USART_CLK                             RCC_APB1Periph_USART3
#define      macESP8266_USART_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define      macESP8266_USART_GPIO_CLK                        RCC_APB2Periph_GPIOD
#define      macESP8266_USART_TX_PORT                         GPIOD
#define      macESP8266_USART_TX_PIN                          GPIO_Pin_8
#define      macESP8266_USART_RX_PORT                         GPIOD
#define      macESP8266_USART_RX_PIN                          GPIO_Pin_9
#define      macESP8266_USART_IRQ                             USART3_IRQn
#define      macESP8266_USART_INT_FUN                         USART3_IRQHandler


/*********************************************** ESP8266 函数宏定义 *******************************************/
#define     macESP8266_Usart( fmt, ... )           USART_printf ( macESP8266_USARTx, fmt, ##__VA_ARGS__ ) 
#define     macPC_Usart( fmt, ... )                printf ( fmt, ##__VA_ARGS__ )
//#define     macPC_Usart( fmt, ... )                

//#define     macESP8266_CH_ENABLE()                 GPIO_SetBits ( macESP8266_CH_PD_PORT, macESP8266_CH_PD_PIN )
//#define     macESP8266_CH_DISABLE()                GPIO_ResetBits ( macESP8266_CH_PD_PORT, macESP8266_CH_PD_PIN )

#define     macESP8266_RST_HIGH_LEVEL()            GPIO_SetBits ( macESP8266_RST_PORT, macESP8266_RST_PIN )
#define     macESP8266_RST_LOW_LEVEL()             GPIO_ResetBits ( macESP8266_RST_PORT, macESP8266_RST_PIN )



/****************************************** ESP8266 函数声明 ***********************************************/
void                     ESP8266_Init                        ( void );
void                     ESP8266_Rst                         ( void );
bool                     ESP8266_Cmd                         ( char * cmd, char * reply1, char * reply2, u32 waittime );
char                     ESP8266_AT_Test                     ( void );
bool                     ESP8266_Net_Mode_Choose             ( ENUM_Net_ModeTypeDef enumMode );
bool                     ESP8266_JoinAP                      ( char * pSSID, char * pPassWord );
bool                     ESP8266_BuildAP                     ( char * pSSID, char * pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode );
bool                     ESP8266_Enable_MultipleId           ( FunctionalState enumEnUnvarnishTx );
bool                     ESP8266_Link_Server                 ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id);
bool                     ESP8266_StartOrShutServer           ( FunctionalState enumMode, char * pPortNum, char * pTimeOver );
uint8_t                  ESP8266_Get_LinkStatus              ( void );
uint8_t                  ESP8266_Get_IdLinkStatus            ( void );
//uint8_t                  ESP8266_Inquire_ApIp                ( uint8_t * pApIp, uint8_t ucArrayLength );
uint8_t ESP8266_Inquire_ApIp ( uint8_t * pApIp, uint8_t * pStaIp,uint8_t ucArrayLength );
bool                     ESP8266_UnvarnishSend               ( void );
void                     ESP8266_ExitUnvarnishSend           ( void );
bool                     ESP8266_SendString                  ( FunctionalState enumEnUnvarnishTx, uint8_t * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId );
uint8_t *                ESP8266_ReceiveString               ( FunctionalState enumEnUnvarnishTx );

bool ESP8266_Link_UDP( char * ip, uint16_t remoteport, uint16_t localport, uint8_t udpmode,ENUM_ID_NO_TypeDef id);
bool ESP8266_JoinAP_DEF( char * pSSID, char * pPassWord );
bool ESP8266_CIPSTA_DEF (void);

uint8_t ESP8266_CIPSTA_CUR(char type);
uint8_t ESP8266_Set_MAC(uint8_t * pStamac);
uint8_t ESP8266_Get_MAC(uint8_t * pStamac);

void USART_printf ( USART_TypeDef * USARTx, char * Data, ... );
#endif


