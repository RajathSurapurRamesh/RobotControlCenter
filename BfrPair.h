/*BfrPair.h*/
/*Rajath Surapur Ramesh*/
#include "Buffer.h"
#ifndef BfrPair_H
#define BfrPair_H
#define NumBfrs 2


typedef struct 
{
  CPU_INT08U putBfrNum;  //The index of the put buffer     
  Buffer buffers[NumBfrs]; //The 2 buffers
}BfrPair;

/*function prototype*/

/*initializing the buffers*/
void BfrPairInit (BfrPair *bfrPair,CPU_INT08U *bfr0Space,CPU_INT08U *bfr1Space,CPU_INT16U size);

/*Resetting the buffer*/
void PutBfrReset (BfrPair *bfrPair);

/*Adding a byte to the put buffer*/
void *PutBfrAddr(BfrPair *bfrPair);

/*Adding a byte to the get buffer*/
void *GetBfrAddr(BfrPair *bfrPair);

/*Testing the buffers*/
CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair);
CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair);

/*closing the put buffer*/
void ClosePutBfr(BfrPair *bfrPair);

/*opening the get buffer*/
void OpenGetBfr (BfrPair *bfrPair);

/*Adding a byte to the put buffer*/ 
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair,CPU_INT16S byte);

/*Removing a byte from the get buffer */
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair);

/*Checking whether the Buffer pair is swappable*/
CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair);

/*Swap the buffer*/
void BfrPairSwap(BfrPair *bfrPair);
#endif