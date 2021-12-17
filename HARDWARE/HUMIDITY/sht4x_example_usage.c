/*
 * Copyright (c) 2020, Sensirion AG
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

#include "sht4x.h"
#include <stdio.h>  // printf

// added by chelsea
#include "humidity.h"
extern STR_HUMIDITY HumSensor;
extern uint16_t  Test[50];
extern bit hum_exists;
extern float tem_org;
extern float hum_org;
/**
 * TO USE CONSOLE OUTPUT (PRINTF) AND WAIT (SLEEP) PLEASE ADAPT THEM TO YOUR
 * PLATFORM
 */

void SHT4x_Initial(void)
{
	uint8_t count = 0;
	sensirion_i2c_init();
	while ((sht4x_probe() != STATUS_OK) && (count++ < 3)) 
	{
		//printf("SHT sensor probing failed\n");
		sensirion_sleep_usec(50000); /* sleep 1s */		
	}	
	if(count <= 3)	
		hum_exists = 3;
}

void Refresh_SHT4x(void)
{
		int32_t temperature, humidity;
		/* Measure temperature and relative humidity and store into variables
		 * temperature, humidity (each output multiplied by 1000).
		 */
		int8_t ret;
		
		if(hum_exists == 3)
		{
			ret = sht4x_measure_blocking_read(&temperature, &humidity);
			if (ret == STATUS_OK) {
				tem_org = temperature / 100;
				hum_org = humidity / 100;
				
	//            printf("measured temperature: %0.2f degreeCelsius, "
	//                   "measured humidity: %0.2f percentRH\n",
	//                   temperature / 1000.0f, humidity / 1000.0f);
			} else {//Test[18]++;
				 // printf("error reading measurement\n");
			}		
		}		
}
#if 0
void SHT4x_Task( void *pvParameters )	{
    /* Initialize the i2c bus for the current platform */
    //sensirion_i2c_init();
		//Test[13]++;
    /* Busy loop for initialization, because the main loop does not work without
     * a sensor.
     */
	Test[14] = 1;
    while (sht4x_probe() != STATUS_OK) 
			{
        //printf("SHT sensor probing failed\n");
        sensirion_sleep_usec(1000000); /* sleep 1s */
				hum_exists = 3;
    }
    //printf("SHT sensor probing successful\n");
		
    while (1) {
        int32_t temperature, humidity;
        /* Measure temperature and relative humidity and store into variables
         * temperature, humidity (each output multiplied by 1000).
         */
        int8_t ret;
			Test[14]++;
				if(hum_exists == 3)
				{Test[15]++;
					ret = sht4x_measure_blocking_read(&temperature, &humidity);
					if (ret == STATUS_OK) {//
						HumSensor.temperature_c = temperature / 100;
						HumSensor.humidity = humidity / 100;
						hum_exists = 3; // new SHT3X
						//Test[16] = HumSensor.temperature_c;
						//Test[17] = HumSensor.humidity;
	//            printf("measured temperature: %0.2f degreeCelsius, "
	//                   "measured humidity: %0.2f percentRH\n",
	//                   temperature / 1000.0f, humidity / 1000.0f);
					} else {//Test[18]++;
						 // printf("error reading measurement\n");
					}		
				}		
        sensirion_sleep_usec(1000000); /* sleep 1s */
		
			}
    
    //return 0;
}
#endif