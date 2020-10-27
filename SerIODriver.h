
/*SerIoDriver.h*/
/*Rajath Surapur Ramesh*/

 #include <stdio.h>
 #include "includes.h"
#include "assert.h"
 #include "BfrPair.h"
 #ifndef SerIODriver_H
 #define SerIODriver_H

 //------------Function Prototypes-------------
void InitSerIO(void);
CPU_INT16S GetByte(void);
CPU_INT16S PutByte(CPU_INT16S c);
void ServiceRx(void);
void ServiceTx(void);
void  SerPrintf (CPU_CHAR *format, ...);
void  SerialISR (void);
void TxFlush(void);
#endif