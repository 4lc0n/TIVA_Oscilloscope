/*
 * dsp.h
 *
 *  Created on: 5 Jul 2020
 *      Author: k10l
 */

#ifndef SOURCE_DSP_H_
#define SOURCE_DSP_H_


//collects data from FIFO buffer and stores it in a 1024 uint8 Array for further investigation / processing etc.
void get_data();


//returns the mean value of the processing buffer (as a value between 0 and 255)
float get_mean();

#endif /* SOURCE_DSP_H_ */
