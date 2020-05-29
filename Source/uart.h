/*
 * uart.h
 *
 *  Created on: May 29, 2020
 *      Author: h2l
 */

#ifndef SOURCE_UART_H_
#define SOURCE_UART_H_

void uart_init(uint32_t baud);
void uart_set_baud(uint32_t baud);

uint8_t uart_put_c(char c);
uint8_t uart_put_s(char *p);

uint8_t uart_put_int(int i);
uint8_t uart_put_uint(uint32_t ui);

uint8_t uart_put_float(float f, uint8_t decimal);


void UART0RXIntHandler();

#endif /* SOURCE_UART_H_ */
