 
#include "Hum_Para.h"
#include "math.h" 					    
 
/*
Equation 1: Calculate Dew point from Temperature, humidity and atmospheric pressure
Temperature = Temperature of the gas
Humidity = Humidity of the gas, in percent
Td = Dewpoint temperature
Td= 243.04*(LN(humidity/100)+((17.625*temperature)/(243.04+temperature)))/(17.625-LN(humidity/100)-((17.625*temperature)/(243.04+temperature)))
*/ 
 
 
//signed int get_dew_pt(signed int para_t,signed int para_h)
//{
//   signed int temp;
//   float ph,pt,ps;
//   
//   ph = log((float)para_h/1000);
//   pt = 17.625*para_t;
//   pt = pt/(2430.4+para_t);
//   ps =ph+pt;
//// temp =2430.4*(log((float)para_h/1000)+((17.625*para_t)/(2430.4+para_t)))/(17.625-log((float)para_h/1000)-((17.625*para_t)/(2430.4+para_t)));
//   temp = ps*2430.4/(17.625-ps);
//       
//   return temp;
//} 

/*
Equation2: Calculate Water Vapor Saturation Pressure from the dew point temperature
Calculate Water Vapor Saturation Pressure from the dew point temperature
Pws = Partial Pressure of water at saturation at given temperature, [hPa]
Td = Dewpoint temperature
A, m, Tn =  curve fitting constants
'Pws=A*POWER(10,(m*TDewpoint)/(TDewpoint+Tn))
*/

#define 	A		6.116441
#define 	m		7.591386
#define		Tn		240.7263

//										A		m		Tn
//float const code Parameter[][] ={{6.116441,7.591386,240.7263}, // -20C ~ +50 C
//								   {6.004918,7.337936,229.3975}, // +50C ~ +100C
//								   {5.856548,7.277310,225.1033}, //+100C ~ +150C
//								   {6.002859,7.290361,227.1704}, //+150C ~ +200C
//								   {9.980622,7.388931,263.1239}};//+200C ~ +350C
 
//unsigned int Get_Pws(signed int TDewpoint)
//{
//	float ftemp1,ftemp2;
//	unsigned int Pws;
//	ftemp1 = m*TDewpoint;
//	ftemp2 = (float)TDewpoint/10+Tn;
//	ftemp1 = pow(10,ftemp1/ftemp2);
//	Pws=ftemp1 * A * 10;
//	return Pws;	
//}
 
/*
Equation3: Calculate the mixing ratio, mass of water to mass of air
X=Mixing Ratio, the mass of water over the mass of dry gas, [g/kg]
'B = Molecular mass of water over the molecular weight of gas x 1000 , 621.9907 for air [k/kg]
Pws = Partial pressure of water at saturation
Patm = Atmospheric pressure, [hPa]
'X=B*Pws/(Patm-Pws) 
*/

#define B		621.9907
#define Patm	1013
//unsigned int Get_Mix_Ratio(unsigned int Pws)
//{
//	unsigned int Mix_Ratio;
//	float ftemp;
//	ftemp = (float)B * Pws;
//	Mix_Ratio = ftemp*10/(Patm-Pws/10);
//	return Mix_Ratio;
//}


/*
Equation4: Calculate the enthalpy
h = Enthalpy of the air, [kJ/kg]
T = temperautre of the air
X = mixing ratio of the air
'h=temperature*(1.01+(0.00189*X))+2.5*X
*/ 

//unsigned int Get_Enthalpy(signed int Temperature,unsigned int Mix_Ratio)
//{
//	unsigned int Enthalpy;
//	float ftemp;
//	ftemp = 1.01+(0.000189*Mix_Ratio); 
//	Enthalpy=Temperature*ftemp+2.5*Mix_Ratio;		
//	return Enthalpy;
//}
// 

 
void Get_Hum_Para(signed short int Temperature,signed short int Humidity,signed short int *Dew_pt,\
					unsigned short int *Pws,unsigned short int *Mix_Ratio,unsigned short int *Enthalpy)
{
	float   ftemp1,ftemp2,ftemp;
//get dew point 	
	ftemp1 = log((float)Humidity/1000);
	ftemp2 = 17.625*Temperature;
	ftemp2 = ftemp2/(2430.4+Temperature);
	ftemp1 =ftemp1+ftemp2;
	ftemp = ftemp1*243.04/(17.625-ftemp1); 
	*Dew_pt = ftemp * 10;
//get pws	
	ftemp1 = m*ftemp;
	ftemp2 = (float)ftemp+Tn;
	ftemp1 = pow(10,ftemp1/ftemp2);
	ftemp=ftemp1 * A;  
	*Pws = ftemp * 10;
//get Mix Ratio	
	ftemp1 = ftemp * B;
	ftemp = ftemp1/(Patm-ftemp); 
	*Mix_Ratio = ftemp * 10;
//get Enthalpy  
	ftemp1 = 1.01+(0.00189*ftemp); 
	ftemp=ftemp1*Temperature + ftemp * 25;	
	*Enthalpy = ftemp;  
}	

float Sys_Filter(float new_value,float pre_value,unsigned char input_filter)
{         
	float temp;
//	static float pre_value = 0;
//	if(Pressure.unit_change == 1)
//	{
//		Pressure.unit_change = 0;
//		pre_value = new_value;
//	}
	temp = pre_value * input_filter;
	pre_value = (temp + new_value) / (input_filter + 1);            
	return pre_value;
} 
 


