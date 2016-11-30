#ifndef  _HUM_PARA_H_	
#define  _HUM_PARA_H_  

//signed int get_dew_pt(signed int para_t,signed int para_h);
//unsigned int Get_Pws(signed int TDewpoint);
//unsigned int Get_Mix_Ratio(unsigned int Pws);
//unsigned int Get_Enthalpy(signed int Temperature,unsigned int Mix_Ratio);

	 
void Get_Hum_Para(signed short int Temperature,signed short int Humidity,signed short int *Dew_pt,\
					unsigned short int *Pws,unsigned short int *Mix_Ratio,unsigned short int *Enthalpy);

float Sys_Filter(float new_value,float pre_value,unsigned char input_filter);
	
#endif

