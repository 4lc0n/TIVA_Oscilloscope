/*
 * uart.c
 *
 *  Created on: May 29, 2020
 *      Author: h2l
 */


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "inc/tm4c123gh6pm.h"

#define TARGET_IS_BLIZZARD_RB1

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"

#include "driverlib/interrupt.h"
#include "uart.h"

#define UART_SUCCESS 1
#define UART_FAIL 0

void uart_init(uint32_t baud)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_UART0) || !ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //configure for 115200, 8N1
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), baud, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    ROM_UARTFIFOEnable(UART0_BASE);

    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}
void uart_set_baud(uint32_t baud){
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), baud, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

}

uint8_t uart_put_c(char c)
{
    while(ROM_UARTBusy(UART0_BASE));
    if(ROM_UARTCharPutNonBlocking(UART0_BASE, c)){
        return UART_SUCCESS;
    }
    else return UART_FAIL;
}
uint8_t uart_put_s(char *p)
{
    uint8_t count = 0;
    while(*p){
        if(uart_put_c(*p++)){
            count++;
        }
        else{
            return 0;
        }

    }
    return count;
}

uint8_t uart_put_int(int i)
{
    char buf[10] = {0};
    if(i < 0){
        uart_put_c('-');
        i *= -1;
    }
    ltoa(i, buf, 10);
    if(uart_put_s(buf)){
        return UART_SUCCESS;
    }
    else return UART_FAIL;

}
uint8_t uart_put_uint(uint32_t ui)
{
    char buf[10] = {0};
    ltoa(ui, buf, 10);
    if(uart_put_s(buf)){
        return UART_SUCCESS;
    }
    else return UART_FAIL;
}

uint8_t uart_put_float(float f, uint8_t decimal)
{
    char buf[10] = {0};
    ftoa(f, buf, decimal);
    if(uart_put_s(buf)){
        return UART_SUCCESS;
    }
    else return UART_FAIL;

}

void UART0RXIntHandler()
{
    uint32_t ui32Status;
    ui32Status = ROM_UARTIntStatus(UART0_BASE, true);
    ROM_UARTIntClear(UART0_BASE, ui32Status);

    //wait for interrupt to be cleared...
    SysCtlDelay(3);
}

