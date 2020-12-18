#include "types.h"

#define USART_REC_LEN  			512  	//定义最大接收字节数 200
#define USART_SEND_LEN			512



typedef struct
{
	uint8 MANUEL_EN;	// 0 - smartconfig 		1 - t3000方式 		2 - disable wifi
	uint8 IP_Auto_Manual; //  0 Auto DHCP   1 static IP
	uint8 IP_Wifi_Status;  // 0 no-Wifi  
	uint16 modbus_port;
	uint16 bacnet_port;
	uint8 rev;
	uint8	reserved[2];
	
	char name[64];
	char password[32];
	uint8 ip_addr[4];
	uint8 net_mask[4];
	uint8 getway[4];
	uint8 mac_addr[6];  // read-only
}STR_SSID;

extern STR_SSID	SSID_Info;

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

void connect_AP(void);
void Restore_WIFI(void);
typedef enum 
{ 
	CMD_NULL,
	CMD_SWITCH_MODE,
	CMD_READ_STA_INFO,
	CMD_READ_AP_INFO,
	CMD_READ_LINK_INFO,
	CMD_READ_MAC,
	CMD_READ_WSLK,
	CMD_CFG_SOCKETA,
	CMD_CFG_SOCKETB,
	CMD_FAC_RESET,
	CMD_BACK_TO_NOMRAL,
	NORMAL_MODE,
	WIFI_TOTOAL_CMD
} WIFI_CMD;

extern uint8 ip_read_flag;
extern uint8 wifi_cmd_num;
extern uint8 const wifi_cmd[WIFI_TOTOAL_CMD][10];

extern uint8 wifi_send_buf[1000];
extern uint16 wifi_sendbyte_num;
extern uint16 wifi_send_count;

extern uint8 USART_RX_BUFC[512];   
//extern uint8 USART_RX_BUFD[50];
extern uint16 rece_countB;
extern uint8 dealwithTagB;
extern uint8 uart_sendB[USART_SEND_LEN];
extern uint16 sendbyte_numB;
//u8 SERIAL_RECEIVE_TIMEOUT ;
extern uint16 rece_sizeB;
extern uint8 update_flag;
extern uint8 serial_receive_timeout_countB;

void dma_init_uart4();
