/*
 * main.h
 *
 *  Created on: May 27, 2020
 *      Author: h2l
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <math.h>
#include <stdio.h>
#include <stdint.h>

enum status{IDLE, PREBUFFERING, POSTBUFFERING};
enum edge{RISING = 0, FALLING = 1, BOTH = 2};
enum display_variant{AVG, PP};
enum interpolation_variant{LINEAR, SINC, DOT};




#endif /* SOURCE_MAIN_H_ */
