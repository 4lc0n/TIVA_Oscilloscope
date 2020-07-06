/*
 * adc.h
 *
 *  Created on: May 27, 2020
 *      Author: h2l
 */

#ifndef ADC_H_
#define ADC_H_


//initialize of pins and Interrupts as well as µDMA in Ping Pong Mode
void adc_init();

void adc_prepare();
//void adc_change_averaging(unsigned int nuber_of_averaging);


void ADC0IntHandler(void);
void GPIOBIntHandler(void);

void uDMA_config_primary();
void uDMA_config_secondary();

#endif /* ADC_H_ */
