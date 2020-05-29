/*
 * adc.c
 *
 *  Created on: May 27, 2020
 *      Author: h2l
 */


#include "adc.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"

#define TARGET_IS_BLIZZARD_RB1

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"

#include "fifo.h"
#include "main.h"
#include "timer.h"


extern struct Buffer preBuffer;
extern struct Buffer postBuffer;
extern volatile enum status triggerstatus;
extern volatile enum edge trigger_edge;
extern volatile uint8_t trigger_voltage;
extern volatile uint16_t prebuffer_filling;


void adc_init(){
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);                                         //enable ADC clock
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);                                        //enable PORTB



    ROM_GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_4);


    ROM_ADCHardwareOversampleConfigure(ADC0_BASE, 64);

    ROM_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_TIMER, 0);                       //enable adc0 with sequence 1 and processor trigger, highest priority

    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH10);                              //configure sequencer for sampling the temperature sensor
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH10);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH10);
    ROM_ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_CH10|ADC_CTL_IE|ADC_CTL_END);          //configure to sample temperature sensor and end conversion + create interrupt

    ROM_ADCIntEnable(ADC0_BASE, 1);                                                         //enable Interrupt to be sent to NVIC

    IntEnable(INT_ADC0SS1);

    ROM_ADCSequenceEnable(ADC0_BASE, 1);                                                    //enable sequencer 1

}

void ADC0IntHandler(void){
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    static uint8_t previous = 0;
    ADCIntClear(ADC0_BASE, 1);

    uint32_t ui32ADC0Raw[4] = {0};
    uint32_t ui32ADCAvg = 0;
    uint8_t ui8data;
    //get value from buffer:
    ROM_ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Raw);

    ui32ADCAvg = (ui32ADC0Raw[0]+ui32ADC0Raw[1]+ui32ADC0Raw[2]+ui32ADC0Raw[3]) / 4;
    ui8data = (uint8_t)(ui32ADCAvg >>4);


    // if prebuffer not full: fill prebuffer
    if(prebuffer_filling < BUFFER_SIZE){

        BufferOverwriteIn(&preBuffer, ui8data);
        triggerstatus = PREBUFFERING;
        prebuffer_filling++;

    }
    else{
        //if buffer full, but still prebuffering: here comes the trigger!
        if(triggerstatus == PREBUFFERING){
            if(trigger_edge == RISING){

                //if previous data < triggervoltage and it now has risen above -> prevents to trigger if buffering starts with a higher voltage
                if(ui8data > trigger_voltage && previous < trigger_voltage){
                    triggerstatus = POSTBUFFERING;
                    BufferIn(&postBuffer, ui8data);

                }
                else{
                    BufferOverwriteIn(&preBuffer, ui8data);
                }
            }
            else if(trigger_edge == FALLING){
                if(ui8data < trigger_voltage && previous > trigger_voltage){
                    triggerstatus = POSTBUFFERING;
                    BufferIn(&postBuffer, ui8data);
                }
                else{
                    BufferOverwriteIn(&preBuffer, ui8data);
                }
            }
            else{
               if((ui8data < trigger_voltage && previous > trigger_voltage) ||(ui8data > trigger_voltage && previous < trigger_voltage) ){
                   triggerstatus = POSTBUFFERING;
                   BufferIn(&postBuffer, ui8data);
               }
               else{
                   BufferOverwriteIn(&preBuffer, ui8data);
              }
            }
        }

        // if post_buffering: simply insert into buffer
        else if(triggerstatus == POSTBUFFERING){
            if(!BufferIn(&postBuffer, ui8data)){       //buffer full: change trigger_status to idle
                triggerstatus = IDLE;
                timer_deactivate();
            }

        }
    }
    //safe previous measurement for triggering
    previous = ui8data;
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);

}

