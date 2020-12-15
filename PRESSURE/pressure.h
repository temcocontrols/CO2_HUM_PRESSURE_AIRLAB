#ifndef __PRESSURE_H__
#define __PRESSURE_H__ 


#include "types.h"
#include "define.h"
#include "bitmap.h"   

 
/****************TYPE**********VALUE***R  A  N  G  E********/
		#define	MS4515			30
		#define DLVR_L01D		20  // -1WC -  +1WC
		#define MPXV7002DP		10	// -8WC -  +8WC
		#define MPXV7007DP		11	//-27WC - +27WC
		#define PRS_26PCDFA		00	// 0-100PSI
		#define PRS_26PCGFA     01	// 0-250PSI

		#define FACTORY_TABLE	0
		#define USER_TABLE		1

		typedef struct
		{ 
			
			u8 SNR_Model; 
//			s16 range_press[2]; 
			u8 filter;
 			u8 auto_manu;  
			u8 unit;
			u8 default_unit;
			u16 ad;
			s16 org_val;
			float pre_val;
			s16 org_val_offset;
// 			s16 manu_val;
			float val_temp;
			u16 index;			// 10^index	
			s32 base; 			//最多9位有效数字 
			u8  cal_point;
			u8 user_cal_point; 
			u16 cal_pr[10];
			u16 cal_ad[10];
			u16 user_cal_pr[10];
			u16 user_cal_ad[10]; 
			s16 pr_range[2];
			float b_line;
			float k_line; 
			u8 cal_table_enable	;
			u8 table_sel;		  // 0 = factory table, 1 = user table 
			u8 unit_change;
			u8 out_rng_flag;
			u8 sensor_status;     //0= no error, 2 = the value no update, 3 = invalid
		
		}_STR_PRESSURE_;

		enum
		{
			inWC = 0,
			KPa1 = 1,
			Psi = 2,
			mmHg = 3,
			inHg = 4,
			Kg_cm2 = 5,
			atmosphere = 6,
			bar = 7,
      pa = 8,			
			Unit_End,
		};  
		
		extern _STR_PRESSURE_  Pressure;
		extern uint8 const code Prs_Unit[][6];
		extern u8 const code  decimal_num[2][9]; 
		void Pressure_Task(void); 
		u8 get_default_unit(u8 pro_model);
 		void Pressure_initial(void);
		s16 Get_DLVR_Pressure(void); 
		void vUpdate_Pressure_Task( void *pvParameters );
		extern uint16 pm25_org_value;
		extern uint16 pm10_org_value;
 
#endif
