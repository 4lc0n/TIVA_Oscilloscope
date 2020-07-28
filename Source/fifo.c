/*
 * fifo.c
 *
 *  Created on: May 26, 2020
 *      Author: h2l
 */

#include "fifo.h"
#include <stdint.h>
#include <stdlib.h>


//writes data into buffer
//returns success state (if buffer is not full)
uint8_t BufferIn(struct Buffer *b, uint8_t byte)
{
  uint16_t next = ((b->write + 1) & BUFFER_MASK);

  if (b->read == next)
    return BUFFER_FAIL; // voll

  b->data[(b->write)] = byte;
  // buffer.data[buffer.write & BUFFER_MASK] = byte; // absolut Sicher
  b->write = next;

  return BUFFER_SUCCESS;
}

//writes data into buffer
//overwrites value if buffer is full
void BufferOverwriteIn(struct Buffer *b, uint8_t byte)
{
  uint16_t next = ((b->write + 1) & BUFFER_MASK);

  if (b->read == next){
      b->read = (b->read+1) & BUFFER_MASK;
  }
  b->data[b->write] = byte;
  // buffer.data[buffer.write & BUFFER_MASK] = byte; // absolut Sicher
  b->write = next;

//  return BUFFER_SUCCESS;
}


//puts current buffer value into pByte
//returns success state
uint8_t BufferOut(struct Buffer *b, uint8_t *pByte)
{
  if (b->read == b->write)
    return BUFFER_FAIL;

  *pByte = b->data[b->read];

  b->read = (b->read+1) & BUFFER_MASK;

  return BUFFER_SUCCESS;
}
