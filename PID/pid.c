#include "config.h"
#define NOCOASTING
 
STR_PID PID[MAX_CONS]; 
uint8 mode_select;

Str_controller_point controllers[MAX_CONS];  

Con_aux	con_aux[MAX_CONS]; 
 
#define PID_SAMPLE_TIME 2

 


void pid_controller( S16_T p_number )   // 10s 
{
 /* The setpoint and input point can only be local to the panel even
		though the structure would allow for points from the network segment
		to which the local panel is connected */
	S32_T op, oi, od, err, erp, out_sum;
/*  err = percent of error = ( input_value - setpoint_value ) * 100 /
					proportional_band	*/
//	U8_T sample_time = 10L;       /* seconds */
 
	U16_T prop;
	S32_T l1;
	Str_controller_point *con;
	Con_aux *conx;
	static S32_T temp_input_value,temp_setpoint_value;
	
	controllers[p_number].sample_time = PID_SAMPLE_TIME;
	con = &controllers[p_number];
	

	if(con->auto_manual == 1)  // manual - 1 , 0 - auto
		return;
	conx = &con_aux[p_number];
//	get_point_value( (Point*)&con->input, (S32_T *)&con->input_value );
//	get_point_value( (Point*)&con->setpoint, (S32_T *)&con->setpoint_value );
	od = oi = op = 0;
//	con->proportional = 20;

	prop = con->prop_high;	
	prop <<= 8;
	prop += con->proportional;

	temp_input_value = swap_double(con->input_value);
	temp_setpoint_value = swap_double(con->setpoint_value); 

	//err = temp_input_value - temp_setpoint_value;  /* absolute error */
	err = temp_setpoint_value - temp_input_value;  /* absolute error */	
	
	erp = 0L;

//	con->reset = 20;

/* proportional term*/
	if( prop > 0 )
		erp = 100L * err / prop;
	if( erp > 100000L ) erp = 100000L;
	if( con->action > 0)
		op = erp; /* + */
	else
		op = -erp; /* - */

	erp = 0L;
	
/* integral term	*/
	/* sample_time = 10s */
	l1 = ( conx->old_err + err ) * (con->sample_time / 2); /* 5 = sample_time / 2 */
	l1 += conx->error_area;
	if( conx->error_area >= 0 )
	{
		 if( l1 > 8388607L )
				l1 = 8388607L;
	}
	else
	{
		 if( l1 < -8388607L )
				l1 = -8388607L;
	}
	conx->error_area = l1;
	if( con->reset > 0 )
	{
		if( con->action > 0)  // fix iterm 
			oi = con->reset * conx->error_area;
		else
			oi -= con->reset * conx->error_area;

		if(con->repeats_per_min > 0)
			oi /= 60L;
		else
			oi /= 3600L;
	}
/* differential term	*/
	if( con->rate > 0)
	{
		od = conx->old_err * 100;
		od /= prop;
		od = erp - od;
		if(con->action > 0)
		{
/*			od = ( erp - conx->old_err * 100 / prop ) * con->rate / 600L;*/
																						/* 600 = sample_time * 60  */
			od *= con->rate;
		}
		else
		{
/*			od = -con->rate * ( erp - conx->old_err
						* 100 / prop ) / 600L; *//* 600 = sample_time * 60  */
			od *= ( -con->rate );
		}
		od /= (con->sample_time * 60); 	/* 600 = sample_time * 60  */
	}

	out_sum = op + con->bias + od / 100; //  od / 100 , because con->rate is 100x

	if( out_sum > 100000L ) out_sum = 100000L;
	if( out_sum < 0 ) out_sum = 0;
	if( con->reset > 0)
	{
		 out_sum += oi;
		 if( out_sum > 100000L )
		 {
				out_sum = 100000L;
		 }
		 if( out_sum < 0 )
		 {
			out_sum = 0;
		 }
	}
	conx->old_err = err;
	con->value = swap_double(out_sum); 
}
 

const int16 default_value[3][6] ={
//  mode,   setpoint,  pterm, iterm
{COOL_MODE,  500,       60,    50},  // PID1 = Humdity
{HEAT_MODE,  200,       60,    50},  // PID2 = Temperature
{COOL_MODE,  600,       60,    50}   // PID3 = CO2
};
static void EEP_pid_intial(void)
{
	uint8 i,j;
	if(AT24CXX_ReadOneByte(EEP_PID_TEST) != 0x56)
	{ 
		write_eeprom(EEP_PID_TEST,0x56);
		for(i=0;i<3;i++) 
		{
			j = i*11;
			controllers[i].action = default_value[i][0];
			write_eeprom(EEP_PID1_MODE+j,controllers[i].action); 
			
			PID[i].EEP_SetPoint = default_value[i][1];
			controllers[i].setpoint_value =  (int32)PID[i].EEP_SetPoint  * 100; 
			write_eeprom(EEP_PID1_SETPOINT+1+j,PID[i].EEP_SetPoint>>8);
			write_eeprom(EEP_PID1_SETPOINT+j,PID[i].EEP_SetPoint);
		  
			controllers[i].proportional = default_value[i][2];
			write_eeprom(EEP_PID1_PTERM+j,controllers[i].proportional);
		  
			controllers[i].reset =  default_value[i][3];
			write_eeprom(EEP_PID1_ITERM+j,controllers[i].reset); 
		}
		write_eeprom(EEP_MODE_SELECT,TRANSMIT_MODE); 
	}
}
static void pid_intial(void)
{ 
 
	uint8 i,j;
	for(i=0;i<3;i++) 
	{
		j = i*11;
		controllers[i].action =  AT24CXX_ReadOneByte(EEP_PID1_MODE+j);
		 
		PID[i].EEP_SetPoint = ((uint16)AT24CXX_ReadOneByte(EEP_PID1_SETPOINT+1+j)<<8)| \
									   AT24CXX_ReadOneByte(EEP_PID1_SETPOINT+j);
	    controllers[i].setpoint_value =  (int32)PID[i].EEP_SetPoint  * 100;
		
		controllers[i].proportional = AT24CXX_ReadOneByte(EEP_PID1_PTERM+j);
	  
		controllers[i].reset =  AT24CXX_ReadOneByte(EEP_PID1_ITERM+j);
		
		controllers[i].auto_manual = 0;
		controllers[i].repeats_per_min = 0;
		controllers[i].prop_high = 0;
		controllers[i].units = 0;	
		controllers[i].bias = 0;	
		controllers[i].rate = 0;	 
		controllers[i].input.panel = modbus.address;
		controllers[i].input.point_type = 3;
		controllers[i].input.number = i;
		var[i].digital_analog  = 0; 
	}
//	memcpy(var[0].label,"Humidity",9); 
//	memcpy(var[1].label,"Temperature",9); 
//	memcpy(var[2].label,"Co2",9); 
	mode_select =  AT24CXX_ReadOneByte(EEP_MODE_SELECT);
}

void vStartPIDTask(void *pvParameters)
{
	uint8 count_10s =0;
	uint8 i;
	portTickType xDelayPeriod = (portTickType)1000 / portTICK_RATE_MS;
	EEP_pid_intial();
	pid_intial();
	print("PID Task\r\n");
	delay_ms(100);
	
	while(1)
	{
	
		if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) ) 
			PID[0].EEP_Input_Value = (int32)Pressure.org_val;	
		else
			PID[0].EEP_Input_Value =(int32)HumSensor.humidity; 
		if((output_auto_manual & 0x01) == 0x01)
		{
			PID[1].EEP_Input_Value = (int32)output_manual_value_temp;
		}
		else if(temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
		{
			PID[1].EEP_Input_Value =(int32)internal_temperature_c;
		}
		else
		{
			PID[1].EEP_Input_Value = (int32)HumSensor.temperature_c;
		}  
		PID[2].EEP_Input_Value =(int32)int_co2_str.co2_int;//output_manual_value_co2;
		
		for(i=0;i<MAX_CONS;i++)
		{
 			if(i == 2)//co2 _channel
				controllers[i].input_value = (int32)PID[i].EEP_Input_Value *1000;
			else
				controllers[i].input_value = (int32)PID[i].EEP_Input_Value *100;
//			controllers[i].action = PID[i].EEP_Mode;
//			controllers[i].setpoint_value = PID[i].EEP_SetPoint;
//			controllers[i].proportional = PID[i].EEP_PTERM;
//			controllers[i].reset = PID[i].EEP_ITERM;
 			PID[i].EEP_Pid = controllers[i].value/1000;
			if(PID[i].Set_Flag)
			{
				uint8 j; 
				
				PID[i].Set_Flag = 0;
				j = i*11; 
				
				write_eeprom(EEP_PID1_MODE+j,controllers[i].action); 
				 
				if( i == 2 ) // co2 channel
				    PID[i].EEP_SetPoint  = controllers[i].setpoint_value  / 1000;
				else
					PID[i].EEP_SetPoint  = controllers[i].setpoint_value  / 100; 
				write_eeprom(EEP_PID1_SETPOINT+1+j,PID[i].EEP_SetPoint>>8);
				write_eeprom(EEP_PID1_SETPOINT+j,PID[i].EEP_SetPoint);
			   
				write_eeprom(EEP_PID1_PTERM+j,controllers[i].proportional);
			   
				write_eeprom(EEP_PID1_ITERM+j,controllers[i].reset);  
			}
 		}
		 
	    controllers[0].units = RH;//humidity
		if(deg_c_or_f == DEGREE_C)
			controllers[1].units = degC; //temperature
		else
			controllers[1].units = degF; 
		controllers[2].units = ppm;//co2
		
		
		count_10s++;  // 1s
		if(count_10s >= PID_SAMPLE_TIME)
		{
			// dealwith controller roution per 1 sec	
			for(i = 0;i < MAX_CONS; i++)
			{					
				pid_controller( i );
 
			}
			count_10s = 0;		 
		} 
		 
		vTaskDelay(xDelayPeriod); 
	}
	
}
