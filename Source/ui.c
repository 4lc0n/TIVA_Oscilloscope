/*
 * ui.c
 *
 *  Created on: Jun 2, 2020
 *      Author: h2l
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "inc/tm4c123gh6pm.h"

#define TARGET_IS_BLIZZARD_RB1

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"



#include "main.h"
#include "ui.h"
#include "timer.h"
#include "ST7735.h"
#include "display.h"
#include "adc.h"


#define MENU_ROWS 6

//#define INVERT_ROTARY_ENCODER

volatile int8_t rotary_count = 0;
volatile int8_t button_count = 0;

extern uint32_t available_frequencies[7];
extern volatile uint32_t trigger_frequency;
extern uint32_t sys_time;
extern volatile enum edge trigger_edge;
extern uint32_t samples_per_division;
extern bool show_measurements;
extern uint8_t divider;
extern bool ac_coupling;

uint8_t menu_level = 0; //0: menu invisible, 1: settings, 2: subsettings (not implemented yet)
uint8_t menu_row = 0;   //see switch satement
uint8_t menu_column = 0;    //0: various settings, 1: edit setting
uint32_t menu_refresh = 0;

char trig_string[3][8] = {"Rising", "Falling", "Any"};
char bool_string[2][6] = {"false", " true"};
char atten_string[3][4] = {"2:1", "3:1", "9:1"};


void ui_init()
{
    //enable interrupt for GPIOD Pin 1, 2, 3

    //enable Periphery GPIOD , even though it is enabled, rea
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
    {

    }

//    GPIOIntRegister(GPIO_PORTD_BASE, GPIODIntHandler);
//    not used because set in startupfile

    //set pin as input
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    //set to weak pullup
    GPIOPadConfigSet(GPIO_PORTD_BASE,GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 , GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    //set Interrupt to falling edge, because of pullup
    GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_FALLING_EDGE);

    //enable interrupt for 1 of 3 pins
    GPIOIntEnable(GPIO_PORTD_BASE, (GPIO_PIN_1 | GPIO_PIN_3));

    IntEnable(INT_GPIOD);
    IntPrioritySet(INT_GPIOD, 3);

    HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK) = 0x4C4F434B;
    HWREG(GPIO_PORTF_BASE+GPIO_O_CR) = GPIO_PIN_0 | GPIO_PIN_4;
//    GPIOUnlockPin(GPIO_PORTF_BASE, GPIO_PIN_0);
    //set button PF0 as Input for manual starting conversion
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);

    //enable pullup
    GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

}
uint8_t ui_update()
{
    if(menu_level == 0 && button_count){
        menu_level = 1;
        button_count = 0;
        menu_row = 0;
        menu_column = 0;
        rotary_count = 0;
        menu_refresh = 1;
    }


    if(menu_level >= 1){

        switch(menu_row){

        case 0:
            //trigger frequency
            if(menu_column){    //second row: change frequency
                static uint8_t current_frequency = 0;
                if(rotary_count){
                    current_frequency = ((current_frequency +rotary_count))%11;
                    trigger_frequency = available_frequencies[current_frequency];
                }

                else if(button_count){  //exit edit row: save changes
                    timer_set_frequency(trigger_frequency);
                    menu_column= 0;
                }
            }
            else{
                //first row:
                //go to next row
                if(rotary_count){
                    menu_row = ((menu_row + rotary_count)) % MENU_ROWS;
                }
                //go to setting
                else if(button_count){
                    menu_column = 1;
                }

            }
            break;


        case 1:
            //trigger edge
            if(menu_column){    //second column: change edge
                static uint8_t current_edge = 0;
                if(rotary_count){
                    current_edge = ((current_edge + rotary_count))%3;
                    trigger_edge = current_edge;
                }

                else if(button_count){  //exit edit row: save changes

                    menu_column= 0;
                }
            }
            else{
                //first column:
                //go to next row
                if(rotary_count){
                    menu_row = ((menu_row + rotary_count)) % MENU_ROWS;
                }
                //go to setting
                else if(button_count){
                    menu_column = 1;
                }

            }
            break;
            // show measurements
        case 2:
            if(menu_column){    //second column: change status

                if(rotary_count){
                    show_measurements = !show_measurements;
                                    }

                else if(button_count){  //exit edit row: save changes

                    menu_column= 0;
                }
            }
            else{
                //first column:
                //go to next row
                if(rotary_count){
                    menu_row = ((menu_row + rotary_count)) % MENU_ROWS;
                }
                //go to setting
                else if(button_count){
                    menu_column = 1;
                }

            }
            break;

            //input attenuator
        case 3:
            if(menu_column){    //second column: change status

                if(rotary_count){
                    divider = ((divider + rotary_count))%3;
                }

                else if(button_count){  //exit edit row: save changes

                    menu_column= 0;
                    set_attenuator(divider);
                }
            }
            else{
                //first column:
                //go to next row
                if(rotary_count){
                    menu_row = ((menu_row + rotary_count)) % MENU_ROWS;
                }
                //go to setting
                else if(button_count){
                    menu_column = 1;
                }

            }
            break;
        case 4:
            if(menu_column){    //second column: change status

                if(rotary_count){
                    ac_coupling = !ac_coupling;
                }

                else if(button_count){  //exit edit row: save changes

                    menu_column= 0;
                    set_ac_coupling(ac_coupling);
                }
            }
            else{
                //first column:
                //go to next row
                if(rotary_count){
                    menu_row = ((menu_row + rotary_count)) % MENU_ROWS;
                }
                //go to setting
                else if(button_count){
                    menu_column = 1;
                }

            }
            break;
        case 5:
            //back button
            //go to next row
            if(rotary_count){
                menu_row = ((menu_row + rotary_count)) % MENU_ROWS;
            }
            //close menu
            else if(button_count){
                menu_column = 0;
                menu_level = 0;

                menu_row = 0;
                menu_column = 0;
                rotary_count = 0;
                button_count = 0;
                menu_refresh = 0;
            }
            break;

        }






        rotary_count = 0;
        button_count = 0;


        //now print the menu
        //create a black square with white outline
        if(menu_refresh++ == 1){
            ST7735_DrawFastHLine(15, 15, 130, ST7735_WHITE);
            ST7735_DrawFastHLine(15, 113, 130, ST7735_WHITE);
            ST7735_DrawFastVLine(15, 16, 97, ST7735_WHITE);
            ST7735_DrawFastVLine(145, 16, 97, ST7735_WHITE);
            ST7735_FillRect(16, 16, 129, 97, ST7735_BLACK);
        }

        //write configuratoin
        ST7735_SetCursor(6, 4);
        ST7735_OutString(" FREQ");
        ST7735_SetCursor(12, 4);
        ST7735_OutString(" ");
        ST7735_OutUDec(trigger_frequency);
        //to fill previous zeros
        ST7735_OutString("    ");

        ST7735_SetCursor(6, 5);
        ST7735_OutString(" TRIG");
        ST7735_SetCursor(12, 5);
        ST7735_OutString(" ");
        ST7735_OutString(trig_string[trigger_edge]);
        ST7735_OutString("   ");

        ST7735_SetCursor(6, 6);
        ST7735_OutString(" MEASU");
        ST7735_SetCursor(12, 6);
        ST7735_OutString(" ");
        ST7735_OutString(bool_string[show_measurements]);
        ST7735_OutString("    ");

        ST7735_SetCursor(6, 7);
        ST7735_OutString(" ATTEN");
        ST7735_SetCursor(12, 7);
        ST7735_OutString(" ");
        ST7735_OutString(atten_string[divider]);
        ST7735_OutString("    ");

        ST7735_SetCursor(6, 8);
        ST7735_OutString(" ACDC");
        ST7735_SetCursor(12, 8);
        ST7735_OutString(" ");
        ST7735_OutString(bool_string[ac_coupling]);
        ST7735_OutString("    ");

        ST7735_SetCursor(6, 9);
        ST7735_OutString(" BACK");

        //print cursor
        ST7735_SetCursor(menu_column ? 12 : 6, 4 + menu_row);
        ST7735_OutString(">");

    }

    else{

        //if rotary_count > 0: zoom the display!
        if(rotary_count != 0 || sample_user_input()){
            if(rotary_count > 0 && samples_per_division > 3){
                samples_per_division /= 2;
            }
            else if(rotary_count < 0){
                samples_per_division *= 2;
                if(samples_per_division > 100)
                {
                    samples_per_division = 100;
                }
            }
            display_update_frame();
            display_chart();
        }


        rotary_count = 0;
        button_count = 0;

    }

    return menu_level;



}

//interrupt service routine for UI interface by rotary encoder on Pins 1,2 and Button on Pin 3
void GPIODIntHandler(void)
{
    uint32_t temp = GPIOIntStatus(GPIO_PORTD_BASE, true);
    GPIOIntClear(GPIO_PORTD_BASE, temp);

    static uint32_t last_rotary_time = 0;
    static uint32_t last_button_time = 0;

    if(temp & (GPIO_PIN_1)){
        //rotary encoder triggered the interrupt and last interrupt was prior 50ms
        if(sys_time - last_rotary_time > 10){
            if(GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2))
            {
                //pin2 is high: rotate right
#ifdef INVERT_ROTARY_ENCODER
                rotary_count++;
#else
                rotary_count--;
#endif

            }
            else{
                //rotate left
#ifdef INVERT_ROTARY_ENCODER
                rotary_count--;
#else
                rotary_count++;
#endif
            }
            last_rotary_time = sys_time;
        }
    }
    else if(temp & (GPIO_PIN_3))
    {
        //Button triggered event:
        if(sys_time - last_button_time > 400)
        {
            button_count++;
            last_button_time = sys_time;
        }
    }


}
