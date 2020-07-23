/***************************************************
  This is a library for the Adafruit 1.8" SPI display.
  This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
  as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

// ST7735.h
// Runs on LM4F120/TM4C123
// Low level drivers for the ST7735 160x128 LCD based off of
// the file described above.
// Daniel Valvano
// September 12, 2013
// Augmented 3/3/2014 to have a simple graphics facility

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO)
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#ifndef _ST7735H_
#define _ST7735H_

// some flags for ST7735_InitR()
enum initRFlags{
  none,
  INITR_GREENTAB,
  INITR_REDTAB,
  INITR_BLACKTAB
};



// Color definitions
#define ST7735_BLACK   0x0000
#define ST7735_BLUE    0x001F
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF

// Initialization for ST7735B screens
void ST7735_InitB(void);




// Initialization for ST7735R screens (green or red tabs)
// ST7735_InitR(INITR_GREENTAB);
void ST7735_InitR(enum initRFlags option);

//void setAddrWindow(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1); // function is now private

//void pushColor(unsigned short color); // function is now private

void ST7735_DrawPixel(int16_t x, int16_t y, uint16_t color);

void ST7735_DrawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

void ST7735_DrawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

void ST7735_FillScreen(uint16_t color);

void ST7735_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

unsigned short ST7735_Color565(uint8_t r, uint8_t g, uint8_t b);

unsigned short ST7735_SwapColor(uint16_t x);

void ST7735_DrawBitmap(int16_t x, int16_t y, const uint16_t *image, int16_t w, int16_t h);

void ST7735_DrawCharS(int16_t x, int16_t y, char c, int16_t textColor, int16_t bgColor, uint8_t size);

void ST7735_DrawChar(int16_t x, int16_t y, char c, int16_t textColor, int16_t bgColor, uint8_t size);

uint32_t ST7735_DrawString(uint16_t x, uint16_t y, char *pt, int16_t textColor);

void ST7735_SetCursor(uint32_t newX, uint32_t newY);

void ST7735_OutUDec(uint32_t n);

void ST7735_Drawline(uint16_t xa, uint16_t ya, uint16_t xe, uint16_t ye, uint16_t color);


//********ST7735_OutString*****************
// Print a string of characters to the ST7735 LCD.
// Position determined by ST7735_SetCursor command
// Color set by ST7735_SetTextColor
// The string will not automatically wrap.
// inputs: ptr  pointer to NULL-terminated ASCII string
// outputs: none
void ST7735_OutString(char *ptr);

//------------ST7735_Message------------
// String draw and number output.  
// Input: device  0 is on top, 1 is on bottom
//        line    row from top, 0 to 7 for each device
//        pt      pointer to a null terminated string to be printed
//        value   signed integer to be printed
void ST7735_Message (unsigned long d, unsigned long l, unsigned char *pt, long value);

void ST7735_SetRotation(unsigned char m);

void ST7735_InvertDisplay(int i);

// *************** ST7735_PlotClear ********************
// Clear the graphics buffer, set X coordinate to 0
// This routine clears the display 
// Inputs: ymin and ymax are range of the plot
// Outputs: none
void ST7735_PlotClear(int32_t ymin, int32_t ymax);
// *************** ST7735_PlotPoint ********************
// Used in the voltage versus time plot, plot one point at y
// It does output to display 
// Inputs: y is the y coordinate of the point plotted
// Outputs: none
void ST7735_PlotPoint(int32_t y);
// *************** ST7735_PlotBar ********************
// Used in the voltage versus time bar, plot one bar at y
// It does not output to display until RIT128x96x4ShowPlot called
// Inputs: y is the y coordinate of the bar plotted
// Outputs: none
void ST7735_PlotBar(int32_t y);
// *************** ST7735_PlotdBfs ********************
// Used in the amplitude versus frequency plot, plot bar point at y
// 0 to 0.625V scaled on a log plot from min to max
// It does output to display 
// Inputs: y is the y ADC value of the bar plotted
// Outputs: none
void ST7735_PlotdBfs(int32_t y);
// *************** ST7735_PlotNext ********************
// Used in all the plots to step the X coordinate one pixel
// X steps from 0 to 127, then back to 0 again
// It does not output to display 
// Inputs: none
// Outputs: none
void ST7735_PlotNext(void);

#endif
