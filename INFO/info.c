#include "stm32f10x.h"
#include "ProductModel.h"
#include "define.h"

#define FW_VER_HIGH (SOFTREV>>8)&0xff
#define FW_VER_LOW (SOFTREV)&0xff


 
//#if (PRODUCT_ID == STM32_CO2_NET)
const u8 pro_info[20] __attribute__((at(0x08008200))) = {'T', 'e', 'm', 'c', 'o', 'C', 'O', '2',  'A', 'L', 'L',0, 0, 0, 0,  FW_VER_LOW, FW_VER_HIGH, 0, 0, 0,};
//const u8 pro_info[20] __attribute__((at(0x08008200))) = {'T', 'e', 'm', 'c', 'o', 'H', 'U', 'M',  'N', 'E', 'T',0, 0, 0, 0,  FW_VER_LOW, FW_VER_HIGH, 0, 0, 0,};
//#elif (PRODUCT_ID == STM32_CO2_RS485) 
//const u8 pro_info[20] __attribute__((at(0x08008200))) = {'T', 'e', 'm', 'c', 'o', 'C', 'O', '2',  'R', 'S', '4','8','5', 0, 0,  FW_VER_LOW, FW_VER_HIGH, 0, 0, 0,};

//#elif (PRODUCT_ID == STM32_HUM_NET)
//const u8 pro_info[20] __attribute__((at(0x08008200))) = {'T', 'e', 'm', 'c', 'o', 'H', 'U', 'M',  'N', 'E', 'T',0, 0, 0, 0,  FW_VER_LOW, FW_VER_HIGH, 0, 0, 0,};

//#elif (PRODUCT_ID == STM32_HUM_RS485) 
//const u8 pro_info[20] __attribute__((at(0x08008200))) = {'T', 'e', 'm', 'c', 'o', 'H', 'U', 'M',  'R', 'S', '4','8','5', 0, 0,  FW_VER_LOW, FW_VER_HIGH, 0, 0, 0,};

//#elif (PRODUCT_ID == STM32_PRESSURE_NET)
//const u8 pro_info[20] __attribute__((at(0x08008200))) = {'T', 'e', 'm', 'c', 'o', 'P', 'S','N', 'E', 'T',0, 0,0, 0, 0,  FW_VER_LOW, FW_VER_HIGH, 0, 0, 0,};

//#elif (PRODUCT_ID == STM32_PRESSURE_RS485) 
//const u8 pro_info[20] __attribute__((at(0x08008200))) = {'T', 'e', 'm', 'c', 'o', 'P', 'S', 'R',  'S', '4', '8','5', 0, 0, 0,  FW_VER_LOW, FW_VER_HIGH, 0, 0, 0,};

//#endif

