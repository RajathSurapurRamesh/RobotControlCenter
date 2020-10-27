/*Buffer.C*/
/*Rajath Surapur Ramesh*/

#include "includes.h"
#include <stdio.h>
#include "Buffer.h"

/*initializing the buffer fields */
void BfrInit(Buffer *bfr,CPU_INT08U *bfrSpace,CPU_INT16U size)
 {
bfr->putIndex =0;
bfr->getIndex =0;
bfr->closed = FALSE;
bfr->size =size;
bfr->buffer=bfrSpace;
 }

/*Resetting the put index and get index of a Buffer*/                       
void BfrReset(Buffer *bfr)
{
  bfr->putIndex=0;
  bfr->getIndex=0;
  bfr->closed =FALSE;
}
/*checking status of the buffer wjether it is open or closed */
CPU_BOOLEAN BfrClosed(Buffer *bfr)
 {
 return(bfr->closed);
 }

/*Marking the buffer as closed*/
 void BfrClose(Buffer *bfr)
 {
 bfr->closed = TRUE;
 }

/* Marks the buffer as opened */
void BfrOpen(Buffer *bfr)
 {
 bfr->closed = FALSE;
}


/*checking whether the buffer is full or not by comparing  with the size of the buffer*/
CPU_BOOLEAN BfrFull(Buffer *bfr)
{
return (bfr->putIndex >= bfr->size);
}

/*checking whether the buffer is empoty or not by comparing  with the size of the buffer*/     
 CPU_BOOLEAN BfrEmpty(Buffer *bfr)
 {

 return (bfr->getIndex >= bfr->size);
 }

/*Adding a byte to a buffer at position putindex abd increment putindex by 1,if the buffer beccomes
full mark it closed */
CPU_INT16S BfrAddByte(Buffer *bfr ,CPU_INT16S theByte)
{
  if (BfrFull(bfr))
  {
    return -1;
  }
  bfr->buffer[bfr->putIndex++]=theByte;
  if (BfrFull(bfr))
  {
    BfrClose(bfr);
  }
      return (theByte);
}

 /*Returns the byte from position getindex and increments getindex by 1.marks the
 buffer as open if the buffer is empty mark it open*/

 CPU_INT16S BfrRemoveByte(Buffer *bfr)
 {
 CPU_INT08U theByte;
 if(BfrEmpty(bfr))
 {
return -1;
 }
 theByte = bfr->buffer[bfr->getIndex++];
 if(BfrEmpty(bfr))
 {
 BfrOpen(bfr);
 }
return (theByte);
}


      