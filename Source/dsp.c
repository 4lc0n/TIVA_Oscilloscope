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

float get_mean(){
    float mean = 0;
    uint16_t i;
    for(i = 2*BUFFER_SIZE-1; i >= 0; i--)
    {
        mean += processing_buffer[i];
    }

    mean /= 2*BUFFER_SIZE;
    return mean;


}
