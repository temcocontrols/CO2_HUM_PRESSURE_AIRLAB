#include "config.h"

#define OutputSTACK_SIZE	portMINIMAL_STACK_SIZE//512
xTaskHandle Handle_Output;

extern void watchdog(void);
U16_T internal_temperature_ad_value = 0;
U8_T output_mode = OUT_FAULT;
U16_T theory_ad;
U8_T th_offset;

U8_T output_auto_manual = 0;
U16_T output_manual_value_co2 = 1500;
S16_T output_manual_value_temp = 500;
U16_T output_manual_value_humidity = 500;

uint16 analog_output[3];
uint16 analog_input[3];

//S8_T hum_amp_output_offset = 0;
//S8_T temp_amp_output_offset = 0;
//S8_T co2_amp_output_offset = 0;
//S8_T hum_vol_output_offset = 0;
//S8_T temp_vol_output_offset = 0;
//S8_T co2_vol_output_offset = 0;

U16_T pwm_th_settings[3] =
{
	0xff,	// hum
	0xff,	// temp
	0xff,	// co2
};

U16_T feedback[6] = 
{
	0,	// hum voltage feedback
	0,	// temp voltage feedback
	0,	// co2 voltage feedback
	0,	// hum current feedback
	0,	// temp current feedback
	0 	// co2 current feedback
};

_RANGE_ output_range_table[3] = 
{
	{0, 1000},	// hum, 0.1%
	{0, 1000},	// temp, 0.1c
	{0, 3000},	// co2, ppm
}; 

int16 output_offset[2][3];

static void Mode_Input_Initial(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_1  ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOD, &GPIO_InitStructure); 

}

static uint8 OUT_AMPSMODE()  //PE1
{
	return GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2); 
}
static uint8 OUT_VOLTMODE()  //PE0
{
	return GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_1); 
} 

void analog_outputs_init(void)
{ 
	Mode_Input_Initial();
   	input_initial();
	TIM_Config();
}
uint8 get_output_mode(void)
{
	if(OUT_AMPSMODE() == 0)
	{
		return _4_20MA;
	}
	else
	{
		if(OUT_VOLTMODE() == 0)
			return _0_5V;
		else
			return _0_10V;		
	}

//	return OUT_FAULT;
}
//for transmit mode
static uint16 get_theory_ad(uint8 mode, uint8 channel, int16 value)
{
	if(value >= output_range_table[channel].max)
	{
		if(mode == _4_20MA)	// ???????
			return 2730;  // 4096*Vout-max/Vref = 4096*Imax*Rsense/Vref = 4096*20mA*100R/3.3V = 2482 = 0x9B2
																	//  = 4096*20  *100 /3	  =	2730  				
		else if(mode == _0_5V)
			return 1732;
		else // _0_10V
			return 3464 ; //(10*1/4)/3.3*4096 = 3103 = 0xC1F
			              //(10*1/4)/3  *4096 = 3413 =  
						  //(10*51/201)/3  *4096 = 3464 = 
	}
	else if(value <= output_range_table[channel].min)
	{
		if(mode == _4_20MA)	// ???????
			return 546;     // 4096*Vout-min/Vref = 4096*Imin*Rsense/Vref = 4096*4mA*100R/3.3V = 496 = 0x1F0
							// 4096*Vout-min/Vref = 4096*Imin*Rsense/Vref = 4096*4mA*100R/3  V = 546    
		else // _0_5V || _0_10V
			return 0x00;
	}
	else
	{ 
		int32 delta = value - output_range_table[channel].min;
		if(mode == _4_20MA)	// ???????
		{
			delta *= (2730 - 546);
			delta /= (output_range_table[channel].max - output_range_table[channel].min);
			delta += 546;
		}
		else if(mode == _0_5V)
		{
			delta *=1732; // *0x60F
			delta /= (output_range_table[channel].max - output_range_table[channel].min);
		}
		else // _0_10V
		{
			delta *=3464; // *0xC1F
			delta /= (output_range_table[channel].max - output_range_table[channel].min);
		}

		return (uint16)delta;
	}
}
//for pid mode
static uint16 get_pid_theory_ad(uint8 mode, uint8 channel, uint16 pid_value)
{
	if(pid_value >= 100)
	{
		if(mode == _4_20MA)	// ???????
			return 2730;  // 4096*Vout-max/Vref = 4096*Imax*Rsense/Vref = 4096*20mA*100R/3.3V = 2482 = 0x9B2
																	//  = 4096*20  *100 /3	  =	2730  				
		else if(mode == _0_5V)
			return 1732;
		else // _0_10V
			return 3464 ; //(10*1/4)/3.3*4096 = 3103 = 0xC1F
			              //(10*1/4)/3  *4096 = 3413 =  
						  //(10*51/201)/3  *4096 = 3464 = 
	}
	else if(pid_value <= 0)
	{
		if(mode == _4_20MA)	// ???????
			return 546;     // 4096*Vout-min/Vref = 4096*Imin*Rsense/Vref = 4096*4mA*100R/3.3V = 496 = 0x1F0
							// 4096*Vout-min/Vref = 4096*Imin*Rsense/Vref = 4096*4mA*100R/3  V = 546    
		else // _0_5V || _0_10V
			return 0x00;
	}
	else
	{ 
		int32 delta = pid_value ;
		if(mode == _4_20MA)	// ???????
		{
			delta *= (2730 - 546);
			delta /= 100;
			delta += 546;
		}
		else if(mode == _0_5V)
		{
			delta *=1732; // *0x60F
			delta /= 100;
		}
		else // _0_10V
		{
			delta *=3464; // *0xC1F
			delta /= 100;
		}

		return (uint16)delta;
	}
}

static void Set_analog_output(uint8 mode, uint8 channel, int16 value)
{
	uint16 real_ad,target_ad, th_offset;
	int16 real_ad_temp,target_ad_temp;
	if(mode_select == TRANSMIT_MODE)
		target_ad_temp = get_theory_ad(mode,channel,value);
	else
	{
//		if(controllers[channel].action  == HEAT_MODE)
			target_ad_temp = get_pid_theory_ad(mode,channel,PID[channel].EEP_Pid);
//		else
//			target_ad_temp = get_pid_theory_ad(mode,channel,100 - PID[channel].EEP_Pid);
	}
	
	if(mode == _4_20MA)	//calibrate
	{
		target_ad_temp += output_offset[1][channel];
		if(target_ad_temp < 0 ) target_ad = 0;
		else if (target_ad_temp > 4095 ) target_ad = 4095;
		else
			target_ad = target_ad_temp;
	}
	else
	{
		target_ad_temp += output_offset[0][channel];
		if(target_ad_temp < 0 ) target_ad = 0;
		else if (target_ad_temp > 4095 ) target_ad = 4095;
		else
			target_ad = target_ad_temp;
	}
 	if(mode == _4_20MA)
	{	
		real_ad  = get_ad_val(channel + 3);
		analog_input[channel] = real_ad; 
		real_ad_temp = real_ad - output_offset[1][channel];
		if(real_ad_temp < 0) real_ad_temp = 0;
		else if(real_ad_temp > 4095) real_ad_temp = 4095;
		analog_output[channel]  =  (uint32)real_ad_temp*3000/4096; //0.01MA 
		 
	}
	else
	{
		real_ad = get_ad_val(channel);
		analog_input[channel] = real_ad;
		real_ad_temp = real_ad - output_offset[0][channel];
		if(real_ad_temp < 0) real_ad_temp = 0;
		else if(real_ad_temp > 4095) real_ad_temp = 4095; 
		analog_output[channel]  =  (uint32)real_ad_temp*60300/208896;//0.01mv//300*201/4096*51
		
	} 
//	outputs[channel].value  = analog_output[channel] ;
	if(target_ad > real_ad)	 // symbol = 1
	{
		if(mode == _4_20MA)
		{
			th_offset = (target_ad - real_ad) / 8;
//			if((target_ad - real_ad) % 8 >= 2)
//				th_offset++;
		}
		else
		{
			th_offset = (target_ad - real_ad) /2;
//			th_offset = (target_ad - real_ad) / 8;
//			if((target_ad - real_ad) % 8 >= 4)
//				th_offset++;
		} 
 		pwm_th_settings[channel] = (((pwm_th_settings[channel] + th_offset) < 4096) ? (pwm_th_settings[channel] + th_offset) : 4095);
	}
	else if(target_ad < real_ad)
	{
		if(mode == _4_20MA)
		{
			th_offset = (real_ad - target_ad) /8;
//			if((real_ad - target_ad) % 8 >= 2)
//				th_offset++;
		}
		else
		{
			th_offset = (real_ad - target_ad) /2;
//			th_offset = (real_ad - target_ad) / 8;
//			if((real_ad - target_ad) % 8 >= 4)
//				th_offset++;
		}

 		pwm_th_settings[channel] = ((pwm_th_settings[channel] > th_offset) ? (pwm_th_settings[channel] - th_offset) : 0);
	} 
	set_output(channel,pwm_th_settings[channel]);
}


 
void refresh_output(void)
{ 
	if ((PRODUCT_ID == STM32_CO2_NET)||(PRODUCT_ID == STM32_CO2_RS485) ) 
	{
		Set_analog_output(output_mode, CHANNEL_CO2, int_co2_str.co2_int); 
		Set_analog_output(output_mode, CHANNEL_HUM,	HumSensor.humidity); 
	}
//#elif defined HUM_SENSOR
//	Set_analog_output(output_mode, CHANNEL_CO2, 	 HumSensor.dew_pt   );
//#elif defined PRESSURE_SENSOR 
//		Set_analog_output(output_mode, CHANNEL_CO2, 	 Pressure.org_val);
 

	else if ((PRODUCT_ID == STM32_HUM_NET)||(PRODUCT_ID == STM32_HUM_RS485) )
	{		
		if(analog_output_sel)
			Set_analog_output(output_mode, CHANNEL_HUM, 	 HumSensor.dew_pt   );
		else		
			Set_analog_output(output_mode, CHANNEL_HUM,	HumSensor.humidity);
	}
	else if ((PRODUCT_ID == STM32_PRESSURE_NET)||(PRODUCT_ID == STM32_PRESSURE_RS485) )
	
		Set_analog_output(output_mode, CHANNEL_HUM, 	 Pressure.org_val);	
	else if(PRODUCT_ID == STM32_PM25) 
	{
		if(pm25_sensor.pm25_range == PM25_0_100)
			Set_analog_output(output_mode, CHANNEL_HUM,	pm25_sensor.pm25);
		else if(pm25_sensor.pm25_range == PM25_0_1000)
			Set_analog_output(output_mode, CHANNEL_HUM,	pm25_sensor.pm25/10);
		Set_analog_output(output_mode, CHANNEL_TEMP,pm25_sensor.pm10);
		return;
	}
		
	if((output_auto_manual & 0x01) == 0x01)
	{
		Set_analog_output(output_mode, CHANNEL_TEMP,output_manual_value_temp);
	}
	else if(temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
	{
		Set_analog_output(output_mode, CHANNEL_TEMP,internal_temperature_c);
	}
	else
	{
		Set_analog_output(output_mode, CHANNEL_TEMP,HumSensor.temperature_c);
	} 
}

//extern void watchdog(void);
void vOutPutTask(void *pvParameters)
{
	analog_outputs_init();
	
	print("Output Task\r\n");
	 
 	delay_ms(100);
	
	for( ;; )
	{    

	
		output_mode = get_output_mode();
		refresh_output(); 

		vTaskDelay(500 / portTICK_RATE_MS);
		
//		 stack_detect(&test[7]);
	}
}






