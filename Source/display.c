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
#include "dsp.h"
#include "uart.h"
#include "ST7735.h"

extern volatile enum status triggerstatus;
extern volatile enum edge trigger_edge;
extern volatile uint8_t trigger_voltage;
extern volatile uint32_t trigger_frequency ;
extern volatile uint32_t samples_per_division ;
extern volatile uint16_t samples_horizontal_offset;
extern volatile struct Buffer preBuffer;
extern volatile struct Buffer postBuffer;
extern uint8_t processing_buffer[1024];

extern volatile int8_t rotary_count;
extern bool show_measurements;
extern uint8_t divider;

extern volatile enum display_variant display_method;
//currently has no effect
extern volatile enum interpolation_variant interpolationmethod;

//local variables:
//we are in horizontal mode:
const uint16_t display_width = 160, display_height = 128;

const uint8_t _offset_x = 8, _offset_y = 9;
uint8_t plot_width = display_width - 2*_offset_x - 2;
uint8_t plot_height = display_height - 2*_offset_y -2;
uint8_t last_dot_x, last_dot_y;
uint8_t current_dot_x, current_dot_y;

void display_frame(){

    uint8_t temp = 0;
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
    if(trigger_frequency < 100000){
        float ftemp = (samples_per_division*1.0) / (trigger_frequency * 1.0) * 1000.0;
        ftoa(ftemp, buf, 1);
        while(buf[temp] != 0){
            temp++;
        }
        strcpy(buf + temp, " ms/Div");
    }
    else{
        float ftemp = (samples_per_division*1.0) / (trigger_frequency * 1.0) * 1000000.0;
        ftoa(ftemp, buf, 1);
        while(buf[temp] != 0){
            temp++;
        }
        strcpy(buf + temp, " us/Div");
    }
    ST7735_DrawString(1, 12, buf, ST7735_WHITE);


    //voltage per division
    temp = 0;
    for(temp = 0; temp < 15; temp++){
        buf[temp] = 0;
    }
    switch(divider){
    case 0:
        ST7735_DrawString(15, 12, "0.825V/Div", ST7735_WHITE);
        break;
    case 1:
        ST7735_DrawString(15, 12, "1.2V/Div  ", ST7735_WHITE);
        break;
    case 2:
        ST7735_DrawString(15, 12, "3.75V/Div ", ST7735_WHITE);
        break;
    default:
        break;
    }

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
        ST7735_DrawString(18, 0, "IDLE", ST7735_WHITE);
        break;
    case PREBUFFERING:
        ST7735_DrawString(18, 0, "PRE ", ST7735_WHITE);
        break;
    case POSTBUFFERING:
        ST7735_DrawString(18, 0, "POST", ST7735_WHITE);
        break;
    default:
        break;
    }
}


void display_chart()
{
    last_dot_x = last_dot_y = 0;
    uint8_t current_pix= 0;
    uint8_t samples_to_use = 0;
    uint16_t used_samples = 0;
    uint8_t raw[30] = {0};
    uint32_t avg = 0;
    uint8_t tpeak, lpeak;
//    samples_horizontal_offset = 0;

    //if rotary encoder was truned: in/decrese samples per division
    if(rotary_count != 0){
        if(rotary_count > 0 && samples_per_division >= 3){
            samples_per_division /= 2;
        }
        else {
            samples_per_division *= 2;
            if(samples_per_division > 100)
            {
                samples_per_division = 100;
            }
        }
    }

    // calculate amounts of samples to display
    uint16_t samples_to_display = samples_per_division * 10;


    //clear chart area
    ST7735_FillRect(_offset_x +1, _offset_y-2, 142, 112, ST7735_BLACK);
    display_draw_grid();
    //calculate datapoints per pixel
    float data_per_pixel = samples_to_display / 140.0;

    //loop over every pixel
    for(current_pix = 0; current_pix < plot_width; current_pix ++) {
        //calculate amount of samples for this pixel
        samples_to_use = lround((current_pix+1.0) * data_per_pixel) - used_samples;
        if(samples_to_use > 0 && used_samples + samples_horizontal_offset < 1023) // zoomed in too far
        {


            tpeak = lpeak = 0;
            avg = 0;


            //get samples for this pixel
            uint8_t temp;
            for(temp = 0; temp <samples_to_use; temp++){

                raw[temp] = processing_buffer[used_samples + samples_horizontal_offset];
    //            uart_put_uint(raw[temp]);
    //            uart_put_s("\r\n");
                used_samples++;

                //by horizontal shiftng reached end

            }

            for(temp = 0; temp < samples_to_use; temp++){
                avg += raw[temp];
                if(temp == 0){
                    tpeak = lpeak = raw[temp];
                }
                if(raw[temp] >= tpeak)tpeak = raw[temp];
                if(raw[temp] <= lpeak)lpeak = raw[temp];

                if(used_samples + samples_horizontal_offset > 1023)continue;
            }

            avg = lround((avg / (samples_to_use * 1.0)))%256;

            //display a fainth trace of the max and min around it
            if(display_method == PP){
                ST7735_DrawFastVLine(current_pix + _offset_x + 1, 109 - (uint8_t)(tpeak * 112 /256) + _offset_y, (tpeak - lpeak)*112/256, ST7735_Color565(0, 100, 100));
            }
            current_dot_x = current_pix + _offset_x + 1;
            current_dot_y = 109 - (uint8_t)(avg * 112.0 / 256) + _offset_y;
            ST7735_DrawPixel(current_dot_x, current_dot_y, ST7735_Color565(0, 255, 255));


            if(interpolationmethod == LINEAR && current_pix > 0){
                ST7735_Drawline(last_dot_x, last_dot_y, current_dot_x, current_dot_y, ST7735_Color565(0, 255, 255));
            }
            last_dot_x = current_dot_x;
            last_dot_y = current_dot_y;
        }

    }

    if(show_measurements){

        uint8_t temp = 0;
        char buf[15] = {0};
        //print V peak-to-peak
        temp = 0;
        for(temp = 0; temp < 15; temp++){
            buf[temp] = 0;
        }
        temp = 0;
        float vpp = get_peak_to_peak() / 256.0;
        switch(divider){
        case 0:
            vpp *= 6.6;
            break;
        case 1:
            vpp *= 10.0;
            break;
        case 2:
            vpp *= 30.0;
            break;
        default:
            break;
        }
        ftoa(vpp, buf, 1);
        while(buf[temp] != 0){
            temp++;
        }
        strcpy(buf + temp, " Vpp  ");
        ST7735_DrawString(17, 3, buf, ST7735_WHITE);

        //print mean voltage
        temp = 0;
        for(temp = 0; temp < 15; temp++){
            buf[temp] = 0;
        }
        temp = 0;
        float v_mean = get_mean() / 128.0 - 1;
        switch(divider){
        case 0:
            v_mean *= 3.3;
            break;
        case 1:
            v_mean *= 5.0;
            break;
        case 2:
            v_mean *=15.0;
            break;
        default:
            break;
        }
        if(v_mean < 0){
            buf[0] = '-';
            v_mean *= -1;
            ftoa(v_mean, buf+1, 2);
        }
        else{
            ftoa(v_mean, buf, 2);
        }
        while(buf[temp] != 0){
            temp++;
        }
        strcpy(buf + temp, " V  ");
        ST7735_DrawString(17, 4, buf, ST7735_WHITE);
    }

}


void display_update_frame()
{
    //write horizontal and vertical dimensions
    //ms / div
    uint8_t temp = 0;
    char buf[15] = {0};
    if(trigger_frequency < 100000){
        float ftemp = (samples_per_division*1.0) / (trigger_frequency * 1.0) * 1000.0;
        ftoa(ftemp, buf, 1);
        while(buf[temp] != 0){
            temp++;
        }
        strcpy(buf + temp, " ms/Div");
    }
    else{
        float ftemp = (samples_per_division*1.0) / (trigger_frequency * 1.0) * 1000000.0;
        ftoa(ftemp, buf, 1);
        while(buf[temp] != 0){
            temp++;
        }
        strcpy(buf + temp, " us/Div");
    }
    ST7735_DrawString(1, 12, buf, ST7735_WHITE);


    //voltage per division
    temp = 0;
    for(temp = 0; temp < 15; temp++){
        buf[temp] = 0;
    }
    switch(divider){
    case 0:
        ST7735_DrawString(15, 12, "0.825V/Div", ST7735_WHITE);
        break;
    case 1:
        ST7735_DrawString(15, 12, "1.2V/Div  ", ST7735_WHITE);
        break;
    case 2:
        ST7735_DrawString(15, 12, "3.75V/Div ", ST7735_WHITE);
        break;
    default:
        break;


    }

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
        ST7735_DrawString(18, 0, "IDLE", ST7735_WHITE);
        break;
    case PREBUFFERING:
        ST7735_DrawString(18, 0, "PRE ", ST7735_WHITE);
        break;
    case POSTBUFFERING:
        ST7735_DrawString(18, 0, "POST", ST7735_WHITE);
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
