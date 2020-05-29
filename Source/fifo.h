/*
 * fifo.h
 *
 *  Created on: May 26, 2020
 *      Quelle: https://www.mikrocontroller.net/articles/FIFO
 */

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stdlib.h>

#define BUFFER_FAIL     0
#define BUFFER_SUCCESS  1

#define BUFFER_SIZE 512 // muss 2^n betragen (8, 16, 32, 64 ...)
#define BUFFER_MASK (BUFFER_SIZE-1) // Klammern auf keinen Fall vergessen



struct Buffer {
  uint8_t data[BUFFER_SIZE];
  uint16_t read; // zeigt auf das Feld mit dem ältesten Inhalt
  uint16_t write; // zeigt immer auf leeres Feld

};// buffer = {{}, 0, 0};

//
// Stellt 1 Byte in den Ringbuffer
//
// Returns:
//     BUFFER_FAIL       der Ringbuffer ist voll. Es kann kein weiteres Byte gespeichert werden
//     BUFFER_SUCCESS    das Byte wurde gespeichert
//
uint8_t BufferIn(struct Buffer *b, uint8_t byte);


//
//Stellt 1 Byte in den Ringpuffer
//Ignoriert dabei ein Überschreiben von alten Daten
//
//Returns:
//    BUFFER_FAIL     nie
//    BUFFER_SUCCESS  das Byte wurde gespeichert
//
uint8_t BufferOverwriteIn(struct Buffer *b, uint8_t byte);

//
// Holt 1 Byte aus dem Ringbuffer, sofern mindestens eines abholbereit ist
//
// Returns:
//     BUFFER_FAIL       der Ringbuffer ist leer. Es kann kein Byte geliefert werden.
//     BUFFER_SUCCESS    1 Byte wurde geliefert
//
uint8_t BufferOut(struct Buffer *b, uint8_t *pByte);

#endif /* SOURCE_FIFO_C_ */
