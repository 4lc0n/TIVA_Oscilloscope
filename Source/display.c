/*
 * display.c
 *
 *  Created on: May 26, 2020
 *      Author: h2l
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>




#include "display.h"


#include "main.h"
#include "helper_fkt.h"
#include "fifo.h"

#include "uart.h"
#include "ST7735.h"

extern volatile enum status triggerstatus;
extern volatile enum edge trigger_edge;
extern volatile uint8_t trigger_voltage;
extern volatile uint32_t trigger_frequency ;
extern volatile uint32_t samples_per_division ;
extern volatile struct Buffer preBuffer;
extern volatile struct Buffer postBuffer;
extern uint8_t processing_buffer[1024];

extern volatile enum display_variant display_method;
//currently has no effect
extern volatile enum interpolation_variant interpolationmethod;

//local variables:
//we are in horizontal mode:
const uint16_t display_width = 160, display_height = 128;

const uint8_t _offset_x = 8, _offset_y = 9;
uint8_t plot_width = display_width - 2*_offset_x - 2;
uint8_t plot_height = display_height - 2*_offset_y -2;


void display_frame(){

    uint8_t temp = 0, temp2 = 0;
    char buf[15] = {0};

    //clear display
    ST7735_FillScreen(ST7735_BLACK);


    //draw line for x and y axis
    ST7735_DrawFastHLine(_offset_x, display_height - _offset_y, display_width - _offset_x - _offset_x, ST7735_Color565(255, 0, 0));
    ST7735_DrawFastVLine(_offset_x, _offset_y, display_height - _offset_y - _offset_y, ST7735_Color565(255, 0, 0));


    display_draw_grid();


    //write horizontal and vertical dimensions
    //ms / div
    temp = 0;
    float ftemp = (samples_per_division*1.0) / (trigger_frequency * 1.0) * 1000.0;
    ftoa(ftemp, buf, 1);
    while(buf[temp] != 0){
        temp++;
    }
    strcpy(buf + temp, " ms/Div");
    ST7735_DrawString(3, 12, buf, ST7735_WHITE);

    //samples/s
    temp = 0;
    for(temp = 0; temp < 15; temp++){
        buf[temp] = 0;
    }
    temp = 0;
    ltoa(trigger_frequency, buf, 10);
    while(buf[temp] != 0){
        temp++;
    }
    strcpy(buf + temp, " S/sec");
    ST7735_DrawString(3, 0, buf, ST7735_WHITE);

    //current status
    switch(triggerstatus){
    case IDLE:
        ST7735_DrawString(15, 0, "IDLE", ST7735_WHITE);
        break;
    case PREBUFFERING:
        ST7735_DrawString(15, 0, "PRE ", ST7735_WHITE);
        break;
    case POSTBUFFERING:
        ST7735_DrawString(15, 0, "POST", ST7735_WHITE);
        break;
    default:
        break;
    }
}


void display_chart()
{
    uint8_t current_pix= 0;
    uint8_t samples_to_use = 0;
    uint16_t used_samples = 0;
    uint8_t raw[30] = {0};
    uint32_t avg = 0;
    uint8_t tpeak, lpeak;
    //clear chart area
    ST7735_FillRect(_offset_x +1, _offset_y-2, 140, 112, ST7735_BLACK);
    display_draw_grid();
    //calculate datapoints per pixel
    float data_per_pixel = 1024 / 140.0;

    //loop over every pixel
    for(current_pix = 0; current_pix < plot_width; current_pix ++) {
        samples_to_use = lround((current_pix+1.0) * data_per_pixel) - used_samples;
        tpeak = lpeak = 0;
        avg = 0;
        //get samples for this pixel
        uint8_t temp, temp2;
        for(temp = 0; temp <samples_to_use; temp++){


            raw[temp] = processing_buffer[used_samples];


//            uart_put_uint(raw[temp]);
//            uart_put_s("\r\n");

            used_samples++;
        }
        for(temp = 0; temp < samples_to_use; temp++){
            avg += raw[temp];
            if(temp == 0){
                tpeak = lpeak = raw[temp];
            }
            if(raw[temp] >= tpeak)tpeak = raw[temp];
            if(raw[temp] <= lpeak)lpeak = raw[temp];
        }
        avg = lround((avg / (samples_to_use * 1.0)))%256;

        //display a fainth trace of the max and min around it
        if(display_method == PP){
            ST7735_DrawFastVLine(current_pix + _offset_x + 1, 112 - (uint8_t)(tpeak * 112 /256) + _offset_y, (tpeak - lpeak)*112/256, ST7735_Color565(0, 100, 100));
        }
        ST7735_DrawPixel(current_pix + _offset_x + 1, 112 - (uint8_t)(avg * 112.0 / 256) + _offset_y, ST7735_Color565(0, 255, 255));

    }

}


void display_update_frame()
{
    //write horizontal and vertical dimensions
    //ms / div
    uint8_t temp = 0;
    char buf[15] = {0};
    ftoa((samples_per_division*1.0) / trigger_frequency * 1000, buf, 1);
    while(buf[temp] != 0){
        temp++;
    }
    strcpy(buf + temp, " ms/Div  ");
    ST7735_DrawString(3, 12, buf, ST7735_WHITE);

    //samples/s
    temp = 0;
    for(temp = 0; temp < 15; temp++){
        buf[temp] = 0;
    }
    temp = 0;
    ltoa(trigger_frequency, buf, 10);
    while(buf[temp] != 0){
        temp++;
    }
    strcpy(buf + temp, " S/sec  ");
    ST7735_DrawString(3, 0, buf, ST7735_WHITE);

    //current status
    switch(triggerstatus){
    case IDLE:
        ST7735_DrawString(15, 0, "IDLE", ST7735_WHITE);
        break;
    case PREBUFFERING:
        ST7735_DrawString(15, 0, "PRE", ST7735_WHITE);
        break;
    case POSTBUFFERING:
        ST7735_DrawString(15, 0, "POST", ST7735_WHITE);
        break;
    default:
        break;
    }



}

void display_draw_grid(){
    uint8_t temp = 0, temp2 = 0;

    //draw dotted lines for divisions
    for(temp = 14 ; temp <= plot_width; temp+= 14){
        for(temp2 = 0; temp2 < plot_height; temp2+=14){
            ST7735_DrawPixel(temp + _offset_x, temp2 + _offset_y, ST7735_Color565(100, 50, 0));
        }
    }
}
