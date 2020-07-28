/*
 * timer.c
 *
 *  Created on: May 27, 2020
 *      Author: h2l
 */

#include <stdint.h>
#include <stdbool.h>
#define TARGET_IS_BLIZZARD_RB1

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"

#include "main.h"

extern volatile enum status triggerstatus;

//configures timer 0A for triggering adc conversion
void timer_init(){
    //enable timer
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);


    ROM_TimerControlStall(TIMER0_BASE, TIMER_A, true);
    uint32_t ui32Period = (SysCtlClockGet() / 10 ) ;
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period - 1);

    //enable interrupt
    //IntEnable(INT_TIMER0A);
    //TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //enables triggering the adc
    ROM_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);



}

//sets frequency for TIMER0A, independent from cpu-freq
void timer_set_frequency(uint32_t frequency){
    if(triggerstatus == IDLE){
        uint32_t ui32Period = (SysCtlClockGet() / frequency );
        ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period);
    }
}

//activates timer to trigger adc conversion, set triggerstatus to prebuffering
void timer_activate(){
    //start timer
    if(triggerstatus == IDLE){
        ROM_TimerEnable(TIMER0_BASE, TIMER_A);
        triggerstatus =PREBUFFERING;
    }
}

//deactivates timer 0A
void timer_deactivate(){
    ROM_TimerDisable(TIMER0_BASE, TIMER_A);

}
