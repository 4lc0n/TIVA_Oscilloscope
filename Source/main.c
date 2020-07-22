/*
 * Messtechnik Semesterarbeit
 * Thema: Proove-of-concept Oszilloskop mit einem TFT-Display, TI Tiva Launchpad ARM CortexM4 Microcontroller
 *
 *
 * Pin configuration:
 * Display:
 *
 *  --SS0: Communication with TFT Display
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
 *  --ADC0, SS0
 *  ADC_CHANNEL_10: ADCin -> PB4
 *      Interrupt Priority: 0
 *  ADC_Trigger_Detected -> PB1
 *      Interrupt Priority: 1
 *  ADC_Trigger_Voltage ->
 *
 *  --Rotary encoder
 *  A -> PD1
 *  B -> PD2
 *  Button -> PD3
 *      Interrupt Priority: 3
 *
 *  --ADC1 SS0
 *  ADC_CHANNEL_7: Potentiometer for horizontal position -> D0
 *
 *  On-Board Button for manual starting record
 *  Right Button -> PF0
 *
 *  TIMER0A: Trigger for ADC
 *
 *  Systick on 10ms Timebase (sys_time)
 *
 *
 *===================================================
 *  For Compiling:
 *  --opt_level: 2
 *  --opt_for_speed: 4
 *  (auf opt_level 3 und opt_for_speed 5 funktioniert adc Trigger-Funktion nicht mehr!)
 *
 *
 *
 *==================================================
 *  Done: - Semi-Interrupt driven menu: works if conversion completed
 *  TODO: - Graph plotting with interaction
 *  Done: - copy data from volatile buffer in processing buffer, use it for drawing, calculations etc: processing_buffer
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
#include "driverlib/systick.h"

#include "ST7735.h"


#include "fifo.h"
#include "adc.h"
#include "timer.h"
#include "main.h"
#include "display.h"
#include "uart.h"
#include "ui.h"
#include "dsp.h"


//global variables -> in bss

    volatile struct Buffer preBuffer = {{0}, 0, 0};     //prebuffer für Speicherung bevor triggerspannung erreicht wurde //aktuell 512 Byte

    volatile struct Buffer postBuffer = {{0}, 0, 0};    //postbuffer für nach triggerung  //aktuell 512 Byte

    uint8_t processing_buffer[1024] = {0};              //puffer für darstellung, signalverarbeitung, nachbereitung, fft etc...

    volatile enum status triggerstatus= IDLE;           //state variable für aktuellen status (wird in SIR aktualisiert)
    volatile uint8_t trigger_voltage = 112;             //triggerspannung (als 8bit adc wert)
    volatile enum edge trigger_edge = RISING;           //trigger-edge (RISING, FALLING, ANY)
    volatile uint32_t prebuffer_filling = 0;            //für ISR, um zuerst prebuffer zu füllen, dann erst triggerung erlauben

    volatile uint32_t trigger_frequency = 20000;        //trigger / timer frequenz, sollte 100.000 nicht überschreiten (isr-zeiten)
    uint32_t available_frequencies[11] = {100, 500, 1000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000};
    bool show_measurements = true;
    uint8_t divider = 0;
    bool ac_coupling = false;

    volatile uint32_t samples_per_division = 100;       //rechnierischer wert für display-fkt
    volatile uint16_t samples_horizontal_offset = 0;

    volatile enum display_variant display_method = PP;  //für darstellung (avg wird immer dargestellt, bei pp schwach dargestellte maximawerte
    volatile enum interpolation_variant interpolationmethod = DOT; //hat aktuell keinen effekt (LINEAR, SINC, DOT)

    volatile uint32_t sys_time = 0;                     //für systick, inc alle 10ms


int main(void){

    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);   //run at 80MHz
    ROM_FPUEnable();                                                                          //enable Floating Point Unit
    ROM_FPULazyStackingEnable();

    SysCtlDelay(300000);

    //initialize Systick Timer for software debouncing of rotary encoder
    uint32_t ui32SysClock = SysCtlClockGet();
    SysTickPeriodSet(4e6 / 100);        //interrupt every 10 ms


    SysTickIntEnable();
    SysTickEnable();


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
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, 0x00);

    ui_init();

    uart_put_s("SETUP FINISHED\n\n\rsetting up for first conversion\n\r");

    SysCtlDelay(3000000);
    display_frame();



    IntMasterEnable();

    //start a single conversion from signal generator



    while(1){

       adc_prepare();

       timer_set_frequency(trigger_frequency);
       timer_activate();



       while(triggerstatus != IDLE){
           if(trigger_frequency <= 100000){
               display_update_frame();
               SysCtlDelay(300000);

           }
       }

       get_data();

       display_frame();
       display_chart();
       while(ui_update() || GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)){
           SysCtlDelay(3000000);

       }

    }
    return 0;
}


void SysTickIntHandler(void){
    sys_time++;
}
