/*
 * ui.h
 *
 *  Created on: Jun 2, 2020
 *      Author: h2l
 */

#ifndef SOURCE_UI_H_
#define SOURCE_UI_H_

void ui_init();

//returns menu level (zero if closed)
uint8_t ui_update();


void GPIODIntHandler(void);


#endif /* SOURCE_UI_H_ */
