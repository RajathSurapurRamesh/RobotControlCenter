/*Buffer.h*/
/*Rajath Surapur Ramesh*/

#include <stdio.h>
#include "includes.h"
#ifndef Buffer_H
#define Buffer_H
#define buffer_data_size 4


typedef struct
{
  volatile CPU_BOOLEAN closed;    /*True if buffer has data ready to process ,ready to be emptied or being emptied */
                         /*False if buffer is not ready to process ,ready to fill pr being filled */ 
  CPU_INT16U size;       /*The capacity of the buffer in bytes*/
  CPU_INT16U putIndex;   /*The position where the next byte is added*/
  CPU_INT16U getIndex;   /*The position of the next byte to remove */
  CPU_INT08U *buffer;    /*the address of the buffer data space*/
}Buffer;

/*Function prototype*/
/*initiazing the buffer*/
void BfrInit(Buffer *bfr,CPU_INT08U *bfrSpace,CPU_INT16U size);

/*Resting the buffer*/
void BfrReset (Buffer *bfr);

/*Testing the buffers*/
CPU_BOOLEAN BfrClosed (Buffer *bfr);

/*closing the buffer*/
void BfrClose(Buffer *bfr);

/*opeing the buffer*/
void BfrOpen(Buffer *bfr);

/*Test whether or not a buffer is full*/
CPU_BOOLEAN BfrFull(Buffer *bfr);

/*Test whether or not buffer is empty*/
CPU_BOOLEAN BfrEmpty(Buffer *bfr);

/*Add a byte to a buffer at position putindex*/
CPU_INT16S BfrAddByte (Buffer *bfr ,CPU_INT16S theByte);

/*Remove the byte from the buffer at the postion get index*/
CPU_INT16S BfrRemoveByte(Buffer *bfr);

#endif