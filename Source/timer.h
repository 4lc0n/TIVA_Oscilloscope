/*
 * timer.h
 *
 *  Created on: May 27, 2020
 *      Author: h2l
 */

#ifndef SOURCE_TIMER_H_
#define SOURCE_TIMER_H_

void timer_init();

void timer_set_frequency(uint32_t frequency);
void timer_activate();
void timer_deactivate();

#endif /* SOURCE_TIMER_H_ */
