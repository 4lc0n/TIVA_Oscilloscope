/*
 * dsp.c
 *
 *  Created on: 5 Jul 2020
 *      Author: k10l
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#include "fifo.h"

extern volatile struct Buffer preBuffer;     //prebuffer für Speicherung bevor triggerspannung erreicht wurde //aktuell 512 Byte
extern volatile struct Buffer postBuffer;     //postbuffer für nach triggerung  //aktuell 512 Byte
extern uint8_t processing_buffer[1024];      //puffer für darstellung, signalverarbeitung, nachbereitung, fft etc...



//dump data from fifo buffers in processing buffer for easy access
void get_data(){

    uint16_t temp1, temp2 = 0;
    while(BufferOut(&preBuffer, &temp1))
    {
        processing_buffer[temp2++] = temp1;
    }
    while(BufferOut(&postBuffer, &temp1))
    {
        processing_buffer[temp2++] = temp1;
    }

}

// U_mean = 1 / T * integral from 0 to T u(t)dt
//return as float as a avg of raw value (no conversion)
float get_mean(){
    float mean = 0;
    uint16_t i;
    for(i = 1023; i != 0; i--)
    {
        if(processing_buffer[i] != 0)mean += processing_buffer[i];
    }

    mean /= 2*BUFFER_SIZE;
    return mean;


}


//loop over buffer and pick highest and lowest value
//return as raw value
uint8_t get_peak_to_peak()
{
    //assign to a value for first data reference
    uint8_t high = processing_buffer[0], low = processing_buffer[0];
    uint16_t i;
    for(i = 1023; i != 0; i--)
    {
       if(processing_buffer[i] > high)
       {
           if(processing_buffer[i] != 0)high = processing_buffer[i];
       }
       if(processing_buffer[i] < low)
       {
           if(processing_buffer[i] != 0)low = processing_buffer[i];
       }
    }
    return high - low;

}


//loop over buffer, return highest value
//return raw value
uint8_t get_highest()
{
    uint16_t i;
    uint8_t high = processing_buffer[0];
    for(i = 1023; i != 0; i--)
        {
           if(processing_buffer[i] > high)
           {
               high = processing_buffer[i];
           }
        }
    return high;
}

//loop over buffer, return lowest value
//return raw value
uint8_t get_lowest()
{
    uint16_t i;
    uint8_t low = processing_buffer[0];
    for(i = 1023; i != 0; i--)
        {
           if(processing_buffer[i] < low)
           {
               low = processing_buffer[i];
           }
        }
    return low;
}
