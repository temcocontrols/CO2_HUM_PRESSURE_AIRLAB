//#include "define.h"
//#include "LibTstatIO.h"
#include "check_AQ.h"
#include "math.h"
#include "registerlist.h"
#include "stm32f10x.h"
#include "24cxx.h"
unsigned short int  aq_calibration = 0;
unsigned short int aq_value_temp = 0;
unsigned char alarm_delay;
extern short int temperature;
unsigned short int  aq_value=99;
unsigned short int  show_aq_sen=99;
unsigned char air_cal_point[4];
unsigned short int aq_level_value[4];
unsigned char  output_manu_enable ;
unsigned char GetByteBit(unsigned char *c,unsigned char pos);
unsigned short int  output_manu_value[3] ;
signed short int AQfilter(signed short int new_temperature,signed short int old_temperature,unsigned char channel);
unsigned short int  analog_calibration[3];
float look_up_hummity_temp_table(float hummity,float tmper);
unsigned short int  hummity ; 
unsigned short int  base_air_q,now_aq;
unsigned short int  aq_buffer;//ervery time read aq
unsigned short int  eight_min_ch=0;
unsigned char aq_level = AQ_CLEAN_STATE;
unsigned char air_up=0;
float const temp_hummity_for_aq[][3]={
					/*40%  65%   85%*/	
					{1.61,1.42,1.25},//10
					{1.30,1.00,0.93},//20
					{0.99,0.80,	0.70},//30
					{0.78,0.61,0.54},//40
					{0.64,0.45,0.43}//4                      	  //50
												};

extern u16  Test[50];

float const  R0[]={0.65,0.53,0.43,0.38};
unsigned short int const  hummity_talbe[]={400,650,850};

unsigned short int const  temperture_talbe[]={100,200,300,400,500};
unsigned char  run_check_aq=0;

// *************************************************************************************
//	initialise air calibration variables
// *************************************************************************************
// ron debug
//	temporary here until i start coding the calibration procedure

//unsigned short int  analog_input[2] ; 

//test
//extern  unsigned short int  test1_f[10]; 

unsigned char GetByteBit(unsigned char* c,unsigned char pos)   
{
	unsigned char  temp;
	//
	temp = 0x01;
	temp <<= pos;
	if((*c & temp) == 1)
		return 1;
	else 
		return 0;
}

signed short int AQfilter(signed short int new_temperature,signed short int old_temperature,unsigned char channel)
{
//   static signed int xdata old_temperature = 0;
//   unsigned int xdata  sum;
   unsigned char temperature_filter; 
   unsigned short int  delta = abs(old_temperature - new_temperature);
   temperature_filter = 5;//b.eeprom[ EEP_FILTER + channel - MAXEEPCONSTRANGE];

   if((delta>100)||(delta<=temperature_filter))
   		old_temperature = new_temperature;	
   else
   {  
		old_temperature = old_temperature + (new_temperature - old_temperature )/(temperature_filter+1);
		
   }   
   return old_temperature;
}


void delay_ms(u16 nms);
void init_AQ_calibration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_11);
	delay_ms(500);
	GPIO_SetBits(GPIOC, GPIO_Pin_11); 	
	
	
	air_cal_point[0]= AT24CXX_ReadOneByte(EEP_CELBRA_AIR1);
	air_cal_point[1]= AT24CXX_ReadOneByte(EEP_CELBRA_AIR2);
	air_cal_point[2]= AT24CXX_ReadOneByte(EEP_CELBRA_AIR3);
	air_cal_point[3]= AT24CXX_ReadOneByte(EEP_CELBRA_AIR4);
	if(air_cal_point[0] ==0xff)	   // set default value
	{
		air_cal_point[0] = 20;
		AT24CXX_WriteOneByte(EEP_CELBRA_AIR1, 20);	
	} 
	if(air_cal_point[1] ==0xff)
	{
		air_cal_point[1] = 30;
		AT24CXX_WriteOneByte(EEP_CELBRA_AIR2, 30);	
	}
	if(air_cal_point[2] ==0xff)
	{
		air_cal_point[2] = 55;
		AT24CXX_WriteOneByte(EEP_CELBRA_AIR3, 55);	
	}

	if(air_cal_point[3] ==0xff)
	{
		air_cal_point[3] = 100;
		AT24CXX_WriteOneByte(EEP_CELBRA_AIR4, 100);	
	}
	alarm_delay=20;//read_eeprom(EEP_ALARM_DELAY);
	if(alarm_delay>40)
		alarm_delay=40;
}




// *************************************************************************************
//	check the air quality of things
//		must call this routine each time there is a new sensor reading
// *************************************************************************************
void check_Air_Quality()
{
	
	static unsigned short int  aq_bu[40],temp,temp1,aq_low_point;
	static unsigned char i=0;
	static unsigned short int  RA;
	unsigned char j;
	static unsigned char k=0;
	float  RS1,RS2;
	static float  rs_cel=1;
	static float  temper_back[2],hummity_back[2];
//	static unsigned int xdata hummity[2],temper[2];

//	aq_value  =  analog_input[1];
	

	/*
		if the model open after 6~7min 
	*/
	if(run_check_aq==INIT_AQ)
	{
		base_air_q = aq_value_temp;//analog_input[1];
		aq_buffer=base_air_q;
		now_aq=base_air_q;//put this to first aq base;
		aq_bu[0]=now_aq;
	//	hummity[0]=avg_humidity_RH;
	//	temper[0]=temperature;
	}
	else 
	{
		if(k&&(aq_level>AQ_CLEAN_STATE)&&(hummity>100))
		{
			rs_cel=now_aq/20;
			RA=(50-rs_cel*10)/rs_cel;
			
			RS1=RA/30.0;
			RS1=look_up_hummity_temp_table(hummity_back[0],temper_back[0])*RS1;
			RS2=RA/30.0;
			RS2=look_up_hummity_temp_table(hummity_back[1],temper_back[1])*RS2;
			if(RS2<RS1*0.83)
			{
				temp=(50/(RA*RS2+10)-50/(RA*RS1+10))*20+temp;//celbration
				if((temp1-temp<air_cal_point[0]-AIR_POLLUTE_PASS)||temp1<temp)
				{
					now_aq=temp1;
					aq_level=AQ_CLEAN_STATE;
				}
			}
		}
	
		
		if(base_air_q > aq_value_temp)
			base_air_q = aq_value_temp;
		
		i++;
	
		if(i>=alarm_delay)
		{
			k=1;
			i=0;
			temper_back[0]=temperature;	
			hummity_back[0]=hummity;
		} 
		
		aq_buffer = aq_value_temp;
		aq_bu[i]=aq_buffer;
		
		if(i==(alarm_delay-1))
		{
			temper_back[1]=temperature;	
			hummity_back[1]=hummity;
		}

		
			/*
				every 8 mins change the air base 
			*/
		eight_min_ch++;
		if((eight_min_ch>AQ_UP_TIME)&&(air_up==1))
		{
			air_up=0;
			eight_min_ch=0;
			now_aq=base_air_q;
			base_air_q=0x3FF;
		}

			
		if((eight_min_ch>AQ_TIME)&&(aq_level==0))//if polluted ,we cannot change the base air
		{
			eight_min_ch=0;
			
		
			now_aq=base_air_q;//put the low in the before time as aq base 
				
			base_air_q=0x3FF;
		}
		
		if(k&&(aq_buffer>now_aq))//after first collection
		{
			////////////////////
			temp1=aq_bu[0];
			
			for(j=0;j<(alarm_delay);j++)          //max        //?    
      	   		 { 
								 if(temp1<aq_bu[j])  
								 {
									temp1=aq_bu[j];  
								 }  
            	 } 
			
			temp=aq_bu[19];
			for(j=0;j<alarm_delay;j++)             //min          
      	   	     	{ 
           	            if(temp>aq_bu[j])  
                  	     {
                  		   temp=aq_bu[j];  
           		     }  
            	    	} 
			
			 if(temp<=now_aq)
			 	temp=now_aq; 
			 /////////////////////////////////
			if(((aq_buffer-now_aq)>air_cal_point[2])&&(aq_level!=AQ_POL_LEV3))
			{


			 if((temp1-temp>air_cal_point[2]))
			 {
			 		aq_low_point=temp;//save the old low point
			 		aq_level=AQ_POL_LEV3;
			 }
			 else if((temp1-temp>air_cal_point[1])&&(aq_level!=AQ_POL_LEV2))
			 {
			 	if(aq_level==AQ_POL_LEV3)
			 	{
			 		if(aq_buffer-now_aq>(air_cal_point[1]-AIR_POLLUTE_PASS))//?
			 		{
			 			//aq_low_point=temp;//save the old low point
			 			aq_level=AQ_POL_LEV2;
			 		}
					else
					{
						//aq_low_point=temp;//save the old low point
						aq_level=AQ_POL_LEV3;
					}
			 	}
				else
					aq_level=AQ_POL_LEV2;
			 }
			 else if((temp1-temp>air_cal_point[0])&&(aq_level!=AQ_POL_LEV1))
			 {
			 	if(aq_level>AQ_POL_LEV1)
			 	{
			 		if(aq_buffer-now_aq<(air_cal_point[0]-AIR_POLLUTE_PASS))
			 			aq_level=AQ_POL_LEV1;
					else
						aq_level=AQ_POL_LEV2;
			 	}
				else
					aq_level=AQ_POL_LEV1;
			 }	
			 
			 
		}
		else if(((aq_buffer-now_aq)>air_cal_point[1])&&((aq_buffer-now_aq)<air_cal_point[2])&&(aq_level!=AQ_POL_LEV2))
		{			
			 if(aq_level<AQ_POL_LEV2)
			 {
				 if(temp1-temp>air_cal_point[1])
				 {
				 	if(aq_level==AQ_POL_LEV3)
				 	{
				 		if(aq_buffer-now_aq<(air_cal_point[1]-AIR_POLLUTE_PASS))
				 			aq_level=AQ_POL_LEV2;
						else
							aq_level=AQ_POL_LEV3;
				 	}
					else 
						aq_level=AQ_POL_LEV2;
				 }
				 else if((temp1-temp>air_cal_point[0])&&(aq_level!=AQ_POL_LEV1))	
				 {
			 		if(aq_level>AQ_POL_LEV1)
			 		{
			 			if(aq_buffer-now_aq>(air_cal_point[0]-AIR_POLLUTE_PASS))
			 				aq_level=AQ_POL_LEV1;
						else
							aq_level=AQ_POL_LEV2;
			 		}
					else
						aq_level=AQ_POL_LEV1;
				 }
			 }
			 else if(aq_level>AQ_POL_LEV2)
			 {
			 	aq_level=AQ_POL_LEV2;
			 }
		}	
		else if(((aq_buffer-now_aq)>air_cal_point[0])&&((aq_buffer-now_aq)<air_cal_point[1])&&(aq_level!=AQ_POL_LEV1))	
		{
			 if(aq_level<AQ_POL_LEV1)
			 {
				  if(temp1-temp>air_cal_point[0])
				 {
				 	if(aq_level>AQ_POL_LEV1)
				 	{
				 		if(aq_buffer-now_aq>(air_cal_point[0]-AIR_POLLUTE_PASS))
				 			aq_level=AQ_POL_LEV1;
						else
							aq_level=AQ_POL_LEV2;
				 	}
					else 
						aq_level=AQ_POL_LEV1;
			 	 }
			 }
			 else if(aq_level>AQ_POL_LEV1)
			 {
			 	aq_level=AQ_POL_LEV1;
			 }
		}	
		else if((aq_value_temp-now_aq)<(air_cal_point[0]-AIR_POLLUTE_PASS)||(aq_value_temp<now_aq))
		{	
		#if 0
			if(aq_level==AQ_POL_LEV1)
			{
			 	if(aq_level>AQ_CLEAN_STATE)
			 	{
			 		if(temp1-temp<(air_cal_point[0]-AIR_POLLUTE_PASS))
			 			aq_level=AQ_CLEAN_STATE;
			 	}
				else
					aq_level=AQ_POL_LEV1;
			 }	
			else
		#endif
				aq_level=AQ_CLEAN_STATE;
			
		}			


	  }
	  else if(k)
	  {
	  	aq_level=AQ_CLEAN_STATE;
		now_aq=aq_buffer;
	  }
		

	
		
		if(aq_level<AQ_POL_LEV3)
		{

			air_up=0;
			
		}
		else /*if(display_char!=6)*/
		{
		
			air_up=1;
			
		}  

		{	

		if(aq_level==AQ_CLEAN_STATE)
		{
		//	show_aq_sen=100; 
			//if((analog_input[1]-now_aq>air_cal_point[0])&&(air_cal_point[0]>5)&&(analog_input[1]>now_aq))
			if(aq_value_temp>now_aq)
			show_aq_sen=100+(((aq_value_temp-now_aq)*100)/(air_cal_point[0]*1.0));
			show_aq_sen += aq_calibration - CALIBRATION_OFFSET;
			if(show_aq_sen>=aq_level_value[0])
				show_aq_sen=aq_level_value[0] - 1;
			else if(show_aq_sen<=99)
					show_aq_sen=99;
		}
		else if(aq_level==AQ_POL_LEV1)
		{
			//show_aq_sen=200; 
			if(aq_value_temp>now_aq)//>air_cal_point[1])//&&(air_cal_point[1]>15)&&(analog_input[1]>now_aq))
			show_aq_sen=aq_level_value[0]+((((aq_value_temp-now_aq-air_cal_point[0])*200)/((air_cal_point[1]-air_cal_point[0])*1.0)));
			show_aq_sen += aq_calibration - CALIBRATION_OFFSET;
			if(show_aq_sen>=aq_level_value[1])
				show_aq_sen=aq_level_value[1] - 1;
			else if(show_aq_sen<=aq_level_value[0])
				show_aq_sen=aq_level_value[0];
		}
		else if(aq_level==AQ_POL_LEV2)
		{
			//show_aq_sen=400; 
			if((aq_value_temp-now_aq>air_cal_point[2])&&(air_cal_point[2]>25)&&(aq_value_temp>now_aq))
			show_aq_sen=aq_level_value[1]+(((aq_value_temp-now_aq-air_cal_point[1])/((air_cal_point[2]-air_cal_point[1])*1.0))*200);
			show_aq_sen += aq_calibration - CALIBRATION_OFFSET;
			if(show_aq_sen>=aq_level_value[2])
				show_aq_sen=aq_level_value[2] - 1;
			else if(show_aq_sen<=aq_level_value[1])
				show_aq_sen=aq_level_value[1];
		}
		else if(aq_level==AQ_POL_LEV3)
		{
			  
//			if((analog_input[1]-now_aq>air_cal_point[3])&&(air_cal_point[3]>35)&&(analog_input[1]>now_aq))
//			show_aq_sen=600+(((analog_input[1]-now_aq-air_cal_point[2])/((air_cal_point[3]-air_cal_point[2])*1.0))*400);
//			show_aq_sen += analog_calibration[1] - CALIBRATION_OFFSET;
//			if(show_aq_sen>=5000)
//				show_aq_sen=5000;
//			else if(show_aq_sen<=600)
//				show_aq_sen=600;
			if((aq_value_temp-now_aq>air_cal_point[2])&&(air_cal_point[2]>25)&&(aq_value_temp>now_aq))
			show_aq_sen=aq_level_value[2]+(((aq_value_temp-now_aq-air_cal_point[1])/((air_cal_point[2]-air_cal_point[1])*1.0))*200);
			show_aq_sen += aq_calibration - CALIBRATION_OFFSET;
			if(show_aq_sen>=aq_level_value[3])
				show_aq_sen=aq_level_value[3] - 1;
			else if(show_aq_sen<=aq_level_value[2])
				show_aq_sen=aq_level_value[0];
		}

		
			aq_value = AQfilter(show_aq_sen,aq_value,CHANNEL_AQ);
	
		/*
		else if(aq_level==AQ_POL_LEV4)
		{
			show_aq_sen=800; 
			if((analog_input[1]-now_aq>air_cal_point[4])&&(air_cal_point[4]>45)&&(analog_input[1]>now_aq))
			show_aq_sen=show_aq_sen+(((analog_input[1]-now_aq-air_cal_point[3])/((air_cal_point[4]-air_cal_point[3])*1.0))*200);
		}
		*/
		//	temp_int = analog_input[1];//GW::
	
//		else
//			temp_int = 0;
	}

		#if 0
		if(k&&(aq_buffer<now_aq)&&(air_up==0))
		{
			aq_level=AQ_CLEAN_STATE;
			now_aq=aq_buffer;
		}
		#endif
	}

	
}

float look_up_hummity_temp_table(float hummity,float tmper) 
{
	float RS=1;
	float vect[2][2];
	float per_x=1,per_y=1;
	float x1,x2;



	//%40 hummity ------65% hummity
	if((hummity>=hummity_talbe[0])&&(hummity<hummity_talbe[1]))	
	{
		//10c~20c temper
		if((tmper>=temperture_talbe[0])&&(tmper<=temperture_talbe[1]))
		{
			vect[0][0]=temp_hummity_for_aq[0][0];
			vect[0][1]=temp_hummity_for_aq[0][1];
			vect[1][0]=temp_hummity_for_aq[1][0];
			vect[1][1]=temp_hummity_for_aq[1][1];
			per_x=(hummity-hummity_talbe[0])/(hummity_talbe[1]-hummity_talbe[0]);
			per_y=(tmper-temperture_talbe[0])/(temperture_talbe[1]-temperture_talbe[0]);
			//return per_x;
		}
		//20c~30c temper
		else if((tmper>temperture_talbe[1])&&(tmper<=temperture_talbe[2]))
		{
			vect[0][0]=temp_hummity_for_aq[1][0];
			vect[0][1]=temp_hummity_for_aq[1][1];
			vect[1][0]=temp_hummity_for_aq[2][0];
			vect[1][1]=temp_hummity_for_aq[2][1];
			per_x=(hummity-hummity_talbe[0])/(hummity_talbe[1]-hummity_talbe[0]);
			per_y=(tmper-temperture_talbe[1])/(temperture_talbe[2]-temperture_talbe[1]);
		//	return per_x;
		}
		//30c~40c temper
		else if((tmper>temperture_talbe[2])&&(tmper<=temperture_talbe[3]))
		{
			vect[0][0]=temp_hummity_for_aq[2][0];
			vect[0][1]=temp_hummity_for_aq[2][1];
			vect[1][0]=temp_hummity_for_aq[3][0];
			vect[1][1]=temp_hummity_for_aq[3][1];
			per_x=(hummity-hummity_talbe[0])/(hummity_talbe[1]-hummity_talbe[0]);
			per_y=(tmper-temperture_talbe[2])/(temperture_talbe[3]-temperture_talbe[2]);
		}
		//40c~50c temper
		else if((tmper>temperture_talbe[3])&&(tmper<=temperture_talbe[4]))
		{
			vect[0][0]=temp_hummity_for_aq[3][0];
			vect[0][1]=temp_hummity_for_aq[3][1];
			vect[1][0]=temp_hummity_for_aq[4][0];
			vect[1][1]=temp_hummity_for_aq[4][1];
			per_x=(hummity-hummity_talbe[0])/(hummity_talbe[1]-hummity_talbe[0]);
			per_y=(tmper-temperture_talbe[3])/(temperture_talbe[4]-temperture_talbe[3]);
		}
	}

		//%65 hummity ------85% hummity
	else if((hummity>=hummity_talbe[1])&&(hummity<=hummity_talbe[2]))	
	{
		//10c~20c temper
		if((tmper>=temperture_talbe[0])&&(tmper<=temperture_talbe[1]))
		{
			vect[0][0]=temp_hummity_for_aq[0][1];
			vect[0][1]=temp_hummity_for_aq[0][2];
			vect[1][0]=temp_hummity_for_aq[1][1];
			vect[1][1]=temp_hummity_for_aq[1][2];
			per_x=(hummity-hummity_talbe[1])/(hummity_talbe[2]-hummity_talbe[1]);
			per_y=(tmper-temperture_talbe[0])/(temperture_talbe[1]-temperture_talbe[0]);
		}
		//20c~30c temper
		else if((tmper>temperture_talbe[1])&&(tmper<=temperture_talbe[2]))
		{
			vect[0][0]=temp_hummity_for_aq[1][1];
			vect[0][1]=temp_hummity_for_aq[1][2];
			vect[1][0]=temp_hummity_for_aq[2][1];
			vect[1][1]=temp_hummity_for_aq[2][2];
			per_x=(hummity-hummity_talbe[1])/(hummity_talbe[2]-hummity_talbe[1]);
			per_y=(tmper-temperture_talbe[1])/(temperture_talbe[2]-temperture_talbe[1]);
		}
		//30c~40c temper
		else if((tmper>temperture_talbe[2])&&(tmper<=temperture_talbe[3]))
		{
			vect[0][0]=temp_hummity_for_aq[2][1];
			vect[0][1]=temp_hummity_for_aq[2][2];
			vect[1][0]=temp_hummity_for_aq[3][1];
			vect[1][1]=temp_hummity_for_aq[3][2];
			per_x=(hummity-hummity_talbe[1])/(hummity_talbe[2]-hummity_talbe[1]);
			per_y=(tmper-temperture_talbe[2])/(temperture_talbe[3]-temperture_talbe[2]);
		}
		//40c~50c temper
		else if((tmper>temperture_talbe[3])&&(tmper<=temperture_talbe[4]))
		{
			vect[0][0]=temp_hummity_for_aq[3][1];
			vect[0][1]=temp_hummity_for_aq[3][2];
			vect[1][0]=temp_hummity_for_aq[4][1];
			vect[1][1]=temp_hummity_for_aq[4][2];
			per_x=(hummity-hummity_talbe[1])/(hummity_talbe[2]-hummity_talbe[1]);
			per_y=(tmper-temperture_talbe[3])/(temperture_talbe[4]-temperture_talbe[3]);
		}
	}




///////out of table
	
	if((hummity<=hummity_talbe[2])&&(hummity>=hummity_talbe[0]))
	if((tmper>=temperture_talbe[0])&&(tmper<=temperture_talbe[4]))
	{
		x1=vect[0][0]*(1-per_x)+vect[0][1]*per_x;
		x2=vect[1][0]*(1-per_x)+vect[1][1]*per_x;

		RS=x1*(1-per_y)+x2*per_y;
	}

	if(hummity>hummity_talbe[2])
	{
		if((tmper>=-100)&&(tmper<=0))
		{
			RS=2.35;
			
		}
		else if((tmper>0)&&(tmper<temperture_talbe[0]))
		{
			RS=1.60;
			
		}
		else if((tmper>temperture_talbe[0])&&(tmper<temperture_talbe[1]))
		{
			RS=temp_hummity_for_aq[0][2];
			
		}
		else if((tmper>temperture_talbe[1])&&(tmper<temperture_talbe[2]))
		{
			RS=temp_hummity_for_aq[1][2];
			
		}
		else if((tmper>temperture_talbe[2])&&(tmper<temperture_talbe[3]))
		{
			RS=temp_hummity_for_aq[2][2];
			
		}
		else if((tmper>temperture_talbe[3])&&(tmper<temperture_talbe[4]))
		{
			RS=temp_hummity_for_aq[3][2];
			
		}	
		else if(tmper>temperture_talbe[4])
		{
			RS=temp_hummity_for_aq[4][2];
			
		}
	}
	else if(hummity<hummity_talbe[0])
	{
		if((tmper>=-100)&&(tmper<=0))
		{
			RS=2.35;
			
		}
		else if((tmper>0)&&(tmper<temperture_talbe[0]))
		{
			RS=1.60;
			
		}
		else if((tmper>temperture_talbe[0])&&(tmper<temperture_talbe[1]))
		{
			RS=temp_hummity_for_aq[0][0];
			
		}
		else if((tmper>temperture_talbe[1])&&(tmper<temperture_talbe[2]))
		{
			RS=temp_hummity_for_aq[1][0];
			
		}
		else if((tmper>temperture_talbe[2])&&(tmper<temperture_talbe[3]))
		{
			RS=temp_hummity_for_aq[2][0];
			
		}
		else if((tmper>temperture_talbe[3])&&(tmper<temperture_talbe[4]))
		{
			RS=temp_hummity_for_aq[3][0];
			
		}	
		else if(tmper>temperture_talbe[4])
		{
			RS=temp_hummity_for_aq[4][0];			
		}
	}
	else if(((hummity<hummity_talbe[2])&&(hummity>hummity_talbe[0]))&&(tmper<temperture_talbe[0])&&(tmper>temperture_talbe[4]))
	{

			if(tmper<-100)
			{
				RS=2.35;
			
			}	
			else if(tmper>-100)
			{
				RS=1.60;
			}
	 		else if(tmper>temperture_talbe[4])
			{
				if((hummity<hummity_talbe[1])&&(hummity>=hummity_talbe[0]))
				{
					RS=0.63;

				}
				else if((hummity<hummity_talbe[2])&&(hummity>=hummity_talbe[1]))
				{
					RS=0.48;

				}				
				else if(hummity>=hummity_talbe[2])
				{
					RS=0.43;
				}					
	 		}
	}

	return RS;
	//look_up_hummity_tmep_table(
}
