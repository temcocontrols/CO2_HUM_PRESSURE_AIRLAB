#include "stdio.h"
#include "t3-pt12.h" 
#include "read_pt.h"
#ifdef T3PT12
_TEMP_VALUE_  linear_K , linear_B;	

_TEMP_VALUE_ _temp_value[12],_temp_offset_value[12];
_TEMP_VALUE_ _offset_rc[12] ;	  // _offset_rc for temperture adujust.

_TEMP_VALUE_   linear_K , linear_B;

signed int temperature[12];
unsigned char   int_float =0 ;

//unsigned char  sensor_flag[10];
//unsigned int   int_temp_offset[10];
 
//_TEMP_INT_ 	rs_data;

float Rs[RPS] = {RP1, RP2, RP3, RP4}; //4 calibration resistance value for calculate K and B


void min2method(float *K, float *B, u8 PointNum, float *Xbuf, float *Ybuf)
{
	double sumx, sumx2, sumy, sumxy;
	u8 i;
	
	sumx = 0;
	sumx2 = 0;
	for(i = 0; i < PointNum; i++)
	{
		sumx += Xbuf[i];
		sumx2 += Xbuf[i] * Xbuf[i];
//		printf("sumx= %lf,sumx2=%lf\n\r ",sumx,sumx2 );
		
	}
	
	sumy = 0;
	for(i = 0; i < PointNum; i++)
	{
		sumy += Ybuf[i];
	}

	sumxy = 0;
	for(i = 0; i < PointNum; i++)
	{
		sumxy += Xbuf[i] * Ybuf[i];
	}
//	printf("sumy=%lf,sumxy=%lf\n\r ",sumy,sumxy );
	*K = ((PointNum*sumxy - sumx*sumy) / (PointNum*sumx2 - sumx*sumx));
	*B = ((sumx2*sumy - sumx*sumxy) / (PointNum*sumx2 - sumx*sumx));
}


float get_rtd_temperature(long rtd_ad , unsigned char channel)
{
	float fT0,fTX;
	float rtd_res ,rtd_tc;
	
	float rtd_A = -0.000000577521439;
	float rtd_B = 0.003908319257;
	float rtd_C = -0.00000000000418347612;

//	float rtd_A = _rtd_ABC[3*channel].temp_C;
//	float rtd_B =_rtd_ABC[3*channel +1].temp_C;
//	float rtd_C = _rtd_ABC[3*channel +2].temp_C;
//	printf("rtd_res=%u\n\r", modbus.range[channel]);
//	rtd_ad = 816959 ;
	if( modbus.range[channel] == 0)
	{
		rtd_res = linear_K.temp_C * rtd_ad + linear_B.temp_C;	//get rtd resistor
		fT0 = (-1.0*rtd_B + sqrt(rtd_B*rtd_B - 4*rtd_A*(1 - rtd_res/RTD_R0))) / (2*rtd_A);
		if(rtd_res < RTD_R0)
		{
			fTX = (rtd_C*fT0*fT0*fT0*(fT0 - 100)) / (rtd_B + 2*rtd_A*fT0 + 3*rtd_C*fT0*fT0*(fT0 - 100) + rtd_C*fT0*fT0*fT0);
			rtd_tc = fT0 + fTX;
		}
		else
		{
			rtd_tc = fT0;
		}
	//	rtd_tc = rtd_tc +(_offset_rc.temp_C) - DEFAULT_TEMP_OFFSET ;
	//	rtd_tc = 49.0  ;
	//	printf("rtd_tc=%f\n\r", rtd_tc);
		return rtd_tc ;
	}
	else if( modbus.range[channel] == 1)
	{
	 	rtd_res = linear_K.temp_C * rtd_ad + linear_B.temp_C;	//get rtd resistor
		fT0 = (-1.0*rtd_B + sqrt(rtd_B*rtd_B - 4*rtd_A*(1 - rtd_res/RTD_R0))) / (2*rtd_A);
		if(rtd_res < RTD_R0)
		{
			fTX = (rtd_C*fT0*fT0*fT0*(fT0 - 100)) / (rtd_B + 2*rtd_A*fT0 + 3*rtd_C*fT0*fT0*(fT0 - 100) + rtd_C*fT0*fT0*fT0);
			rtd_tc = fT0 + fTX;
		}
		else
		{
			rtd_tc = fT0;
		}
	    rtd_tc = rtd_tc * 9	;
		rtd_tc = rtd_tc / 5 ;
		rtd_tc = rtd_tc + 32 ;
		return rtd_tc ;	
	}
	else 
	{
	 	rtd_res = linear_K.temp_C * rtd_ad + linear_B.temp_C;	//get rtd resistor
//		printf("rtd_res=%f\n\r", rtd_res);
		return rtd_res ;
	}
}
void update_temperature	(void)
{
//	unsigned char data i;
	static u8  channel_loop = 0;
//	modbus.range[channel_loop] = 0 ;
	for(channel_loop=0; channel_loop<12; channel_loop++)
	{
			
		if(modbus.range[channel_loop] == 0)
			{
				
				rs_data[channel_loop+4]=PT12_FITER(channel_loop+4,rs_data[channel_loop+4],modbus.filter_value[channel_loop]);
				if(rs_data[channel_loop+4] > 2000000)
				{
					_temp_value[channel_loop].temp_C = 1000;

					temperature[channel_loop] = 10000;
				}
				else if(rs_data[channel_loop+4] <500000) 
				{
					_temp_value[channel_loop].temp_C = -251;
					temperature[channel_loop] = -2510;
				}
				else
				{
//					sensor_flag[channel_loop] = 0 ;
					_temp_offset_value[channel_loop].temp_C  =get_rtd_temperature(rs_data[channel_loop+4] , channel_loop)  ;			
					
					
					
					if(int_float == 0)
					{
						_temp_value[channel_loop].temp_C = _temp_offset_value[channel_loop].temp_C /*+(_offset_rc[channel_loop].temp_C)-100.000*/;
//					   if(_temp_value[channel_loop].temp_C > 800) 	_temp_value[channel_loop].temp_C  = 850 ;
//					   else if(_temp_value[channel_loop].temp_C < -200)	_temp_value[channel_loop].temp_C  = -250 ;
					_temp_value[channel_loop].temp_C = data_convert(_temp_value[channel_loop].temp_C , modbus.resole_bit);
					//	printf(" value[%u]=%f\n\r", channel_loop, _temp_value[channel_loop].temp_C);
					}
					else
					{
					//	temperature[channel_loop] = (signed int)( _temp_offset_value[channel_loop].temp_C *10) +_offset_rc[channel_loop].half_temp_C[0] -100;				 
						temperature[channel_loop] = (signed int)( _temp_offset_value[channel_loop].temp_C *10) +(signed int)(_offset_rc[channel_loop].temp_C*10) -1000;
					}
				}
			}
			else if (modbus.range[channel_loop]== 1)
			{
				rs_data[channel_loop+4]=PT12_FITER(channel_loop,rs_data[channel_loop+4],modbus.filter_value[channel_loop]);
			   if(rs_data[channel_loop+4] > 2000000)
				{
					_temp_value[channel_loop].temp_C = 1000;
//					sensor_flag[channel_loop] = 1 ;
				}
				else if(rs_data[channel_loop+4] <500000) 
				{
					_temp_value[channel_loop].temp_C = -251;
//					sensor_flag[channel_loop] = 2 ;
				}
				else
				{
//						sensor_flag[channel_loop] = 0 ;
					_temp_offset_value[channel_loop].temp_C  =get_rtd_temperature(rs_data[channel_loop+4], channel_loop) ;
					
					if(int_float == 0)
					{
						_temp_value[channel_loop].temp_C = _temp_offset_value[channel_loop].temp_C +(_offset_rc[channel_loop].temp_C)-100.000;
						_temp_value[channel_loop].temp_C = data_convert(_temp_value[channel_loop].temp_C , modbus.resole_bit);
				   	 if(_temp_value[channel_loop].temp_C > 1000) 	_temp_value[channel_loop].temp_C  =1000 ;
					 else if(_temp_value[channel_loop].temp_C < -251)	_temp_value[channel_loop].temp_C  = -251 ;
					}
					else
					{
						temperature[channel_loop] = (signed int)( _temp_offset_value[channel_loop].temp_C *10)+_offset_rc[channel_loop].half_temp_C[0] -1000;
					}	
				}
			}
			else
			{
				rs_data[channel_loop+4]=PT12_FITER(channel_loop,rs_data[channel_loop+4],modbus.filter_value[channel_loop]);
				_temp_offset_value[channel_loop].temp_C  =get_rtd_temperature(rs_data[channel_loop+4] , channel_loop)  ;
				_temp_value[channel_loop].temp_C = _temp_offset_value[channel_loop].temp_C ;
				
				if(int_float == 0)
				{
				_temp_value[channel_loop].temp_C = data_convert(_temp_value[channel_loop].temp_C , modbus.resole_bit);
				   if(_temp_value[channel_loop].temp_C > 800) 	_temp_value[channel_loop].temp_C  = 850 ;
				   else if(_temp_value[channel_loop].temp_C < 0)	_temp_value[channel_loop].temp_C  = -250 ;
				}
				else
				{
					temperature[channel_loop] = (signed int)( _temp_offset_value[channel_loop].temp_C *10);
				}	
			}
		}
}

float data_convert(float input_data , u8 resol_bit )
{
	u32   convert_temp;
	float  convert_output;
	input_data = input_data *10000 ;
	if(resol_bit == 2)
	{
		convert_temp =(u32)input_data	/ 100 ;
		convert_output =   convert_temp / 100.0 ;
	}
	else if(resol_bit == 1)
	{
		convert_temp =(u32)input_data	/ 1000 ;
		convert_output =   convert_temp / 10.0 ;	
	}
	else if(resol_bit == 3)
	{
		convert_temp =(u32)input_data	/ 10 ;
		convert_output =   convert_temp / 1000.0 ;	
	}
	else if(resol_bit == 0)
	{
		convert_temp =(u32)input_data	/ 10000 ;
		convert_output =   convert_temp / 1.0 ;	
	}
	else
	{
		convert_output= input_data ;
	}



	return 	convert_output;		
}


s32 PT12_FITER(u8 channel,u32 input, u8 filter)
{
	// -------------FILTERING------------------
	s32  siDelta;
	s32  siResult;
	u8  I;
    s32  siTemp;
	s32  slTemp;  
	static signed long  adam_analog_filter[16];
	I = channel;
	siTemp = input;
 
 
 
	siDelta = siTemp - (s32)adam_analog_filter[I] ;    //compare new reading and old reading
    if (( siDelta >= 1000 ) || ( siDelta <= -1000 ) ) // deg f
	//	adam_analog_filter[I] = adam_analog_filter[I] + (siDelta >> 2);//1 
		adam_analog_filter[I] =  siTemp; 

	// If the difference in new reading and old reading is greater than 5 degrees, implement rough filtering.
    else if (( siDelta >= 100 ) || ( siDelta <= -100 ) ) // deg f
	//	adam_analog_filter[I] = adam_analog_filter[I] + (siDelta >> 2);//1 
		adam_analog_filter[I] =  adam_analog_filter[I]+ (siDelta >> 4); 
 			
	// Otherwise, implement fine filtering.
	else
	{		      
	 
		slTemp = (signed long)filter*adam_analog_filter[I];
		slTemp += (signed long)siTemp * (100 - filter);
	 	adam_analog_filter[I] = (signed long)(slTemp/100);			 
	 
	}
	siResult = adam_analog_filter[I];
 	
 
	return siResult;
	
}
#endif
