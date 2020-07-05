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


    ROM_ADCHardwareOversampleConfigure(ADC0_BASE, 4);
                                                                                        //changed sequencer from 1 to 0 -> 8 Byte FIFO!
    ROM_ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_TIMER, 0);                       //enable adc0 with sequence 1 and processor trigger, highest priority


    ROM_ADCSequenceStepConfigure(ADC0_BASE,0,0,ADC_CTL_CH10|ADC_CTL_IE|ADC_CTL_END);     //     //configure to sample ADC Channel 10 and end conversion (interrupt when fifo is half full (4 Bytes)

    ROM_ADCIntEnable(ADC0_BASE, 0);                                                         //enable Interrupt to be sent to NVIC

    IntEnable(INT_ADC0SS0);
    IntPrioritySet(INT_ADC0SS0, 0);

    ROM_ADCSequenceEnable(ADC0_BASE, 0);                                                    //enable sequencer 1


    //configure trigger input pin with pin change interrupt
    //enable interrupt for GPIOD Pin 1, 2, 3

    //enable Periphery GPIOD , even though it is enabled, rea
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
    {

    }

    //    GPIOIntRegister(GPIO_PORTB_BASE, GPIOBIntHandler);
    //    not used because set in startupfile

    //set pin as input
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_1);



    //set Interrupt to trigger on any edge
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_BOTH_EDGES);

    //enable interrupt for pin 1
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_1);

    //IntEnable(INT_GPIOB);     //start with prepare
    IntPrioritySet(INT_GPIOB, 1);

}

void adc_prepare(){
    preBuffer.read = 0;
    preBuffer.write = 0;

    postBuffer.read = 0;
    postBuffer.write = 0;

    triggerstatus = IDLE;
    prebuffer_filling = 0;
    IntEnable(INT_GPIOB);
}


void ADC0IntHandler(void){

//
//    static uint8_t previous = 0;
//    ADCIntClear(ADC0_BASE, 1);
//
//    uint32_t ui32ADC0Raw = 0;
//    //uint32_t ui32ADCAvg = 0;
//    uint8_t ui8data;
//    //get value from buffer:
//    ROM_ADCSequenceDataGet(ADC0_BASE, 1, &ui32ADC0Raw);
//
//
//    ui8data = (uint8_t)(ui32ADC0Raw >>4);
//
//
//    // if prebuffer not full: fill prebuffer
//    if(prebuffer_filling < BUFFER_SIZE){
//
//        BufferOverwriteIn(&preBuffer, ui8data);
//        triggerstatus = PREBUFFERING;
//        prebuffer_filling++;
//        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
//    }
//    else{
//        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
//        //if buffer full, but still prebuffering: here comes the trigger!
//        if(triggerstatus == PREBUFFERING){
//            if(trigger_edge == RISING){
//
//                //if previous data < triggervoltage and it now has risen above -> prevents to trigger if buffering starts with a higher voltage
//                if(ui8data > trigger_voltage && previous < trigger_voltage){
//                    triggerstatus = POSTBUFFERING;
//                    BufferIn(&postBuffer, ui8data);
//
//                    //for debug timing purpose:
//
//
//                }
//                else{
//                    BufferOverwriteIn(&preBuffer, ui8data);
//                }
//            }
//            //trigger_edgt: falling
//            else if(trigger_edge == FALLING){
//                if(ui8data < trigger_voltage && previous > trigger_voltage){
//                    triggerstatus = POSTBUFFERING;
//                    BufferIn(&postBuffer, ui8data);
//                }
//                else{
//                    BufferOverwriteIn(&preBuffer, ui8data);
//                }
//            }
//            //trigger_edge: any
//            else{
//               if((ui8data < trigger_voltage && previous > trigger_voltage) ||(ui8data > trigger_voltage && previous < trigger_voltage) ){
//                   triggerstatus = POSTBUFFERING;
//                   BufferIn(&postBuffer, ui8data);
//               }
//               else{
//                   BufferOverwriteIn(&preBuffer, ui8data);
//              }
//            }
//        }
//
//        // if post_buffering: simply insert into buffer
//        else if(triggerstatus == POSTBUFFERING){
//            if(!BufferIn(&postBuffer, ui8data)){       //buffer full: change trigger_status to idle
//                triggerstatus = IDLE;
//                timer_deactivate();
//            }
//
//        }
//    }
//    //safe previous measurement for triggering
//    previous = ui8data;
//
//
//
//
//
//
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, 0x0);



    /*hardware triggering: for faster ISR time (with potential to 1MSPS
     *
     */

    ADCIntClear(ADC0_BASE, 0);

    uint32_t ui32ADC0Raw = 0;
    //uint32_t ui32ADCAvg = 0;
    uint8_t ui8data;
    //get value from buffer:
    ROM_ADCSequenceDataGet(ADC0_BASE, 0, &ui32ADC0Raw);


    ui8data = (uint8_t)(ui32ADC0Raw >>4);


    // if prebuffer not full: fill prebuffer
    if(triggerstatus == PREBUFFERING){
       prebuffer_filling++;
       GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
       BufferOverwriteIn(&preBuffer, ui8data);
    }
    else{
       GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
       if(!BufferIn(&postBuffer, ui8data)){
           triggerstatus = IDLE;
           timer_deactivate();
       }
    }
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, 0x0);

}

//ISR for PB1, trigger_detect pin
void GPIOBIntHandler(void)
{
    uint32_t temp = GPIOIntStatus(GPIO_PORTB_BASE, true);
    GPIOIntClear(GPIO_PORTB_BASE, temp);

    temp = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1);
    if(prebuffer_filling >= BUFFER_SIZE && triggerstatus != IDLE){
        if(trigger_edge == RISING  && (temp & GPIO_PIN_1)){
            triggerstatus = POSTBUFFERING;
            IntDisable(INT_GPIOB);

        }
        else if(trigger_edge == FALLING && !(temp & GPIO_PIN_1)){
            triggerstatus = POSTBUFFERING;
            IntDisable(INT_GPIOB);
        }
        else if(trigger_edge == BOTH){
            triggerstatus = POSTBUFFERING;
            IntDisable(INT_GPIOB);

        }
    }


}
