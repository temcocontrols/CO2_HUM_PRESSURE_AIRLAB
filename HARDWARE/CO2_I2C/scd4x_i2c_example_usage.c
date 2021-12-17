/*
 * Copyright (c) 2021, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>  // printf

#include "scd4x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"

/**
 * TO USE CONSOLE OUTPUT (PRINTF) IF NOT PRESENT ON YOUR PLATFORM
 */
//#define printf(...)


extern uint16_t  Test[50];
extern uint8_t internal_co2_module_type;
void co2_reset(void);
void IO_Init(void);
void SCD40_get_value(uint16_t co2,uint16_t temperaute, uint16_t humidity);
uint8_t scd4x_perform_forced;
extern uint16_t co2_frc;
extern uint16_t co2_asc;

extern uint8_t output_auto_manual;
extern uint16_t output_manual_value_co2;

void SCD40_Initial(void)
{
	int16_t error = 0;
//    uint16_t serial_0;
//    uint16_t serial_1;
//    uint16_t serial_2;
 //   sensirion_i2c_hal_init();
		sensirion_i2c_init();
		IO_Init();		
		co2_reset();
		sensirion_i2c_init();
    // Clean up potential SCD40 states
    scd4x_wake_up();
    scd4x_stop_periodic_measurement();
    scd4x_reinit();
		scd4x_perform_forced = 0;
//    error = scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
//    if (error) {;//Test[4] = 2;
//       // printf("Error executing scd4x_get_serial_number(): %i\n", error);
//    } else {;//Test[4] = 3;
//       // printf("serial: 0x%04x%04x%04x\n", serial_0, serial_1, serial_2);
//			internal_co2_module_type = 7;//SCD40;
//    }

    // Start Measurement
    error = scd4x_start_periodic_measurement();
    if (error) {
        //printf("Error executing scd4x_start_periodic_measurement(): %i\n", error);
    }
		else{
			internal_co2_module_type = 7;//SCD40;
		}
    //printf("Waiting for first measurement... (5 sec)\n");

}
extern float tem_org;
extern float hum_org;

void Refresh_SCD40(void)
{
	int16_t error = 0;
	
	if(internal_co2_module_type == 7/*SCD40*/)
	{ 
		uint16_t co2;
		int32_t temperature;
		int32_t humidity;		
		if(scd4x_perform_forced == 1)
		{
			scd4x_stop_periodic_measurement();delay_ms(1000);
			scd4x_perform_forced_recalibration(co2_frc,&co2_asc);delay_ms(1000);
			scd4x_start_periodic_measurement();delay_ms(1000);
			scd4x_perform_forced = 0;
		}
		else
		{
			if((output_auto_manual & 0x04) == 0x04)
			{
				SCD40_get_value(output_manual_value_co2,tem_org,hum_org);
			}
			else
			{					
				error = scd4x_read_measurement(&co2, &temperature, &humidity);
				if (error) {
						//printf("Error executing scd4x_read_measurement(): %i\n", error);
				} else if (co2 == 0) {
					 // printf("Invalid sample detected, skipping.\n");
				} else {
					SCD40_get_value(co2,temperature / 100,humidity / 100);
					
				}

			}

		}
	}
}
#if 0
void SCD40_Task( void *pvParameters )
{
    int16_t error = 0;
//    uint16_t serial_0;
//    uint16_t serial_1;
//    uint16_t serial_2;
 //   sensirion_i2c_hal_init();
		sensirion_i2c_init();
		IO_Init();		
		co2_reset();
		sensirion_i2c_init();
    // Clean up potential SCD40 states
    scd4x_wake_up();
    scd4x_stop_periodic_measurement();
    scd4x_reinit();
		//Test[4] = 1;		
//    error = scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
//    if (error) {;//Test[4] = 2;
//       // printf("Error executing scd4x_get_serial_number(): %i\n", error);
//    } else {;//Test[4] = 3;
//       // printf("serial: 0x%04x%04x%04x\n", serial_0, serial_1, serial_2);
//			internal_co2_module_type = 7;//SCD40;
//    }

    // Start Measurement
    error = scd4x_start_periodic_measurement();
    if (error) {;//Test[5] = 4;
        //printf("Error executing scd4x_start_periodic_measurement(): %i\n", error);
    }
		else
			internal_co2_module_type = 7;//SCD40;

    //printf("Waiting for first measurement... (5 sec)\n");

    for (;;) {
        // Read Measurement 
       
				
				sensirion_sleep_usec(5000000);
				if(internal_co2_module_type == 7/*SCD40*/)
				{ 
					uint16_t co2;
					int32_t temperature;
					int32_t humidity;
					error = scd4x_read_measurement(&co2, &temperature, &humidity);
					if (error) {
							//printf("Error executing scd4x_read_measurement(): %i\n", error);
					} else if (co2 == 0) {
						 // printf("Invalid sample detected, skipping.\n");
					} else {
						SCD40_get_value(co2);
	//					Test[2] = co2;
	//					Test[3] = temperature / 1000;
	//					Test[4] = humidity / 1000;
							//printf("CO2: %u\n", co2);
							//printf("Temperature: %d m°C\n", temperature);
							//printf("Humidity: %d mRH\n", humidity);
					}
			}
    }

}
#endif