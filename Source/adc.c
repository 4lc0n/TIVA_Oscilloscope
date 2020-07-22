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
#include "inc/hw_adc.h"
#include "inc/hw_udma.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/udma.h"

#include "fifo.h"
#include "main.h"
#include "timer.h"

#define DMA_BUFFER_SIZE 128


extern struct Buffer preBuffer;
extern struct Buffer postBuffer;
extern volatile enum status triggerstatus;
extern volatile enum edge trigger_edge;
extern volatile uint8_t trigger_voltage;
extern volatile uint16_t prebuffer_filling;
extern volatile uint16_t samples_horizontal_offset;


#pragma DATA_ALIGN(DMA_Control_Table,1024)
uint8_t DMA_Control_Table[1024] = {0};

//buffer for µDMA
uint16_t BufferA [DMA_BUFFER_SIZE], BufferB[DMA_BUFFER_SIZE];


enum BUFFERSTATUS
                  { EMPTY,
                    FILLING,
                    FULL
                  };
volatile enum BUFFERSTATUS Bufferstatus[2];

void adc_init(){
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);                                         //enable ADC clock
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);                                        //enable PORTB
    ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_UDMA);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    //set outputs for attenuator network

    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5|GPIO_PIN_6);
    ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5|GPIO_PIN_6, 0x00);


    ROM_GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_4);

    //ADC and µDMA configuration
    //disable hardware oversampling
    ADCHardwareOversampleConfigure(ADC0_BASE, 0);


    //disable SS for configuration
    ADCSequenceDisable(ADC0_BASE, 0);

    //configure SS
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_TIMER, 0);

    //Assign Pins to sequencer
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH10|ADC_CTL_IE|ADC_CTL_END);

    //enable sequencer
    ADCSequenceEnable(ADC0_BASE, 0);

    //activate uDMA and assign table to it
    uDMAEnable();
    uDMAControlBaseSet(DMA_Control_Table);


    ADCSequenceDMAEnable(ADC0_BASE, 0);

    uDMAChannelAttributeDisable(UDMA_CHANNEL_ADC0, UDMA_ATTR_ALTSELECT | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

    uDMAChannelAttributeEnable(UDMA_CHANNEL_ADC0, UDMA_ATTR_USEBURST);

    uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);

    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &BufferA, DMA_BUFFER_SIZE);
    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &BufferB, DMA_BUFFER_SIZE);

    IntEnable(INT_ADC0SS0);
    IntPrioritySet(INT_ADC0SS0, 0);
    ADCIntEnableEx(ADC0_BASE, ADC_INT_DMA_SS0); // Enables ADC interrupt source due to DMA on ADC sample sequence 0
    uDMAChannelEnable(UDMA_CHANNEL_ADC0); // Enables DMA channel so it can perform transfers


//    ROM_ADCSequenceDisable(ADC0_BASE, 0);
//                                                                                        //changed sequencer from 1 to 0 -> 8 Byte FIFO!
//    ROM_ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_TIMER, 0);                       //enable adc0 with sequence 1 and processor trigger, highest priority
//    ROM_ADCSequenceStepConfigure(ADC0_BASE,0,0,ADC_CTL_CH10|ADC_CTL_IE|ADC_CTL_END);     //     //configure to sample ADC Channel 10 and end conversion (interrupt when fifo is half full (4 Bytes)
//
//    ROM_ADCIntClear(ADC0_BASE, 0);
//    ROM_ADCIntEnable(ADC0_BASE, 0);
////    ADCSequenceDMAEnable(ADC0_BASE, 0);
//
//
////    ADCIntEnableEx(ADC0_BASE, ADC_INT_DMA_SS0);                                                         //enable Interrupt to be sent to NVIC
//
//    ADCSequenceEnable(ADC0_BASE, 0);
//
//    IntPrioritySet(INT_ADC0SS0, 0);
//    IntEnable(INT_ADC0SS0);

//    //configure µDMA for faster sampling rate
//    ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_UDMA);
//    while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA));
//    uDMAEnable();
//
//    uDMAControlBaseSet(DMA_Control_Table);
//
//    ROM_ADCSequenceEnable(ADC0_BASE, 0);                                                    //enable sequencer 1
//
//    // Put the attributes in a known state.  These should already be disabled by default.
//    uDMAChannelAttributeDisable( UDMA_CHANNEL_ADC0, UDMA_ATTR_ALTSELECT |UDMA_ATTR_USEBURST | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK );
//    // Set the USEBURST attribute. This is somewhat more efficient bus usage than the default which
//    // allows single or burst transfers.
//    uDMAChannelAttributeEnable( UDMA_CHANNEL_ADC0, UDMA_ATTR_USEBURST );
//
//    uDMAChannelControlSet( UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_ARB_1 | UDMA_DST_INC_16 );
//
//    uDMAChannelControlSet( UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_ARB_1 | UDMA_DST_INC_16 );
//
////      uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG, (void *) (ADC0_BASE + ADC_O_SSFIFO0), BufferA, 64);
//    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG, (void *) (ADC0_SSFIFO0_R), &BufferA, 64 ); //ADC0_SSFIFO0_R
//
//    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG, (void *) (ADC0_SSFIFO0_R), &BufferB, 64 );
//
//
//    ROM_uDMAChannelEnable(UDMA_CHANNEL_ADC0);




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
    uDMAChannelAttributeDisable(UDMA_CHANNEL_ADC0, UDMA_ATTR_ALTSELECT | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

    uDMAChannelAttributeEnable(UDMA_CHANNEL_ADC0, UDMA_ATTR_USEBURST);

    uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);

    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &BufferA, DMA_BUFFER_SIZE);
    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &BufferB, DMA_BUFFER_SIZE);
    uDMAChannelEnable(UDMA_CHANNEL_ADC0); // Enables DMA channel so it can perform transfers
    Bufferstatus[0] = FILLING;
    Bufferstatus[1] = EMPTY;
}


void ADC0IntHandler(void){
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);

    ADCIntClear(ADC0_BASE, 0);


    uint32_t ui32_mode;
    ui32_mode = ROM_uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT);
    if( ui32_mode == UDMA_MODE_STOP&& Bufferstatus[0] == FILLING){
        uDMA_config_primary();
        Bufferstatus[0] = FULL;
        Bufferstatus[1] = FILLING;
        if(triggerstatus == PREBUFFERING){
            for(ui32_mode = 0; ui32_mode < DMA_BUFFER_SIZE; ui32_mode++){

               prebuffer_filling++;
               BufferOverwriteIn(&preBuffer, (uint8_t)(BufferA[ui32_mode]>>4));
            }
        }
        else{
            for(ui32_mode = 0; ui32_mode < DMA_BUFFER_SIZE; ui32_mode++){
               if(!BufferIn(&postBuffer, (uint8_t)(BufferA[ui32_mode]>>4))){
                   triggerstatus = IDLE;
                   timer_deactivate();
                   ui32_mode = DMA_BUFFER_SIZE;
                   uDMAChannelDisable(UDMA_CHANNEL_ADC0);
               }
            }
        }
    }
    ui32_mode = ROM_uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT);
    if(ui32_mode == UDMA_MODE_STOP && Bufferstatus[1] == FILLING){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        uDMA_config_secondary();
        Bufferstatus[1] = FULL;
        Bufferstatus[0] = FILLING;
        if(triggerstatus == PREBUFFERING){
            for(ui32_mode = 0; ui32_mode < DMA_BUFFER_SIZE; ui32_mode++){

               prebuffer_filling++;
               BufferOverwriteIn(&preBuffer, (uint8_t)(BufferB[ui32_mode]>>4));
            }
        }
        else{
            for(ui32_mode = 0; ui32_mode < DMA_BUFFER_SIZE; ui32_mode++){
               if(!BufferIn(&postBuffer, (uint8_t)(BufferB[ui32_mode]>>4))){
                   triggerstatus = IDLE;
                   timer_deactivate();
                   ui32_mode = DMA_BUFFER_SIZE;
                   uDMAChannelDisable(UDMA_CHANNEL_ADC0);
                   IntDisable(INT_GPIOB);
               }
            }
        }
    }




//    uint32_t ui32ADC0Raw = 0;
//    //uint32_t ui32ADCAvg = 0;
//    uint8_t ui8data;
//    //get value from buffer:
//    ROM_ADCSequenceDataGet(ADC0_BASE, 0, &ui32ADC0Raw);
//
//
//    ui8data = (uint8_t)(ui32ADC0Raw >>4);
//
//
//    // if prebuffer not full: fill prebuffer
//    if(triggerstatus == PREBUFFERING){
//       prebuffer_filling++;
//
//
//       GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
//       BufferOverwriteIn(&preBuffer, ui8data);
//    }
//    else{
//       GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
//
//       if(!BufferIn(&postBuffer, ui8data)){
//           triggerstatus = IDLE;
//           timer_deactivate();
//       }
//    }
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, 0x00);

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


void uDMA_config_primary(){
    uint32_t temp;
    temp = ROM_uDMAChannelIsEnabled(UDMA_CHANNEL_ADC0);
    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &BufferA, DMA_BUFFER_SIZE);


}
void uDMA_config_secondary(){
    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &BufferB, DMA_BUFFER_SIZE);


}




bool sample_user_input()
{
    static uint32_t last_horizontal_offset = 0;
    uint32_t ui32_adc_data;
    //Prepare ADC1 for sampling potentiometer on Pin XXX
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);                                         //enable ADC clock
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    ROM_ADCSequenceDisable(ADC1_BASE, 0);
    ROM_GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0);

    //enable oversampling for noise free results
    ROM_ADCHardwareOversampleConfigure(ADC1_BASE, 64);


    ROM_ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);                       //enable adc0 with sequence 1 and processor trigger, highest priority
    ROM_ADCSequenceStepConfigure(ADC1_BASE,0,0,ADC_CTL_CH7|ADC_CTL_IE|ADC_CTL_END);     //     //configure to sample ADC Channel 10 and end conversion (interrupt when fifo is half full (4 Bytes)

    ROM_ADCSequenceEnable(ADC1_BASE, 0);

    ROM_ADCIntClear(ADC1_BASE, 0);

    ADCProcessorTrigger(ADC1_BASE,  0);
    while(!ADCIntStatus(ADC1_BASE, 0, false));

    ADCSequenceDataGet(ADC1_BASE, 0, &ui32_adc_data);

    //make it stepped so no deadband is needed to display values near edge
    samples_horizontal_offset = (ui32_adc_data>>2) & (0xFFC0);
    if(last_horizontal_offset != samples_horizontal_offset){
        last_horizontal_offset = samples_horizontal_offset;
        return true;
    }
    else{
        return false;
    }
}

void set_attenuator(uint8_t divider_setting)
{
    switch(divider_setting){
    case 0:
        ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_6);
        break;
    case 1:
        ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_5);
        break;
    case 2:
        ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_5 | GPIO_PIN_6);
        break;
    default:
        ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5|GPIO_PIN_6, 0);
        break;



    }


}
void set_ac_coupling(bool state)
{
    if(state){
        ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
    }
    else{
        ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
    }
}
