/*
 * Messtechnik Semesterarbeit
 * Thema: Proove-of-concept Oszilloskop mit einem TFT-Display, TI Tiva Launchpad ARM CortexM4 Microcontroller
 *
 *
 * Pin configuration:
 * Display:
 *
 *  GND -> GND
 *  VCC -> 3V3
 *  SCK -> PA2
 *  SDA -> PA5
 *  RES -> PA7
 *  RS  -> PA6
 *  CS  -> PA3
 *  LEDA-> 3V3
 *
 *
 *  TODO: - Timer Interrupt to start conversion
 *  TODO: - solid FIFO buffer for pre-buffering and post-buffering
 *  TODO: - Interrupt driven menu
 *  TODO: - Graph plotting with interaction
 *  TODO: -
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "inc/tm4c123gh6pm.h"

#define TARGET_IS_BLIZZARD_RB1

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"


#include "ST7735.h"


#include "fifo.h"
#include "adc.h"
#include "timer.h"
#include "main.h"
#include "display.h"


//global variables -> in bss
    volatile struct Buffer preBuffer = {{0}, 0, 0};
    volatile struct Buffer postBuffer = {{0}, 0, 0};


    volatile enum status triggerstatus= IDLE;
    volatile uint8_t trigger_voltage = 112;
    volatile enum edge trigger_edge = RISING;
    volatile uint16_t prebuffer_filling = 0;

    volatile uint16_t trigger_frequency = 1000;
    volatile uint16_t samples_per_division = 100;

    volatile enum display_variant display_method = AVG;
    //currently has no effect
    volatile enum interpolation_variant interpolationmethod = LINEAR;


int main(void){

    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);   //run at 80MHz
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();

    //aber kann hier vom Interrupt aus zugegriffen werden? :O
    //bei global definiert kann man mitm Debugger nicht beobachten...


    //initialize uart;
    uart_init(115200);

    uart_put_s("init Display\n\r");


    //init display
    ST7735_InitR(INITR_BLACKTAB);

    ST7735_SetRotation(1);


    uart_put_s("init ADC\n\r");


   //init adc
    adc_init();
    SysCtlDelay(300000);


    uart_put_s("init Timer\n\r");


    //initialize timer
    timer_init();


    //configure led for debug information on ISR runtime
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);



    uart_put_s("SETUP FINISHED\n\n\rsetting up for first conversion\n\r");

    SysCtlDelay(3000000);



    ST7735_PlotClear(0 , 4096);
    IntMasterEnable();

    //start a single conversion from signal generator

    preBuffer.read = 0;
    preBuffer.write = 0;

    postBuffer.read = 0;
    postBuffer.write = 0;

    triggerstatus = IDLE;

    timer_set_frequency(1000);
    timer_activate();

    display_frame();

    while(triggerstatus != IDLE){
        SysCtlDelay(300000);
        display_update_frame();
    }




    display_chart();


    while(1){
        //to start trigger:
        //prebuffer_filling to zero
        //triggerstatus to PREBUFFERING
        //activate timer

        SysCtlDelay(3000000);
    }
    return 0;
}


