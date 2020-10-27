/*framer.h*/
/*Rajath Surapur Ramesh*/
#ifndef Framer_H
#define Framer_H

/*Function Prototypes*/
#include "SerIODriver.h"
CPU_VOID CreateFramerTask();
void Framedata(CPU_INT08U Robot,CPU_INT08U Type,CPU_INT08U Error);
CPU_VOID FramerTask(CPU_VOID *data);
extern OS_Q FramerQueue;

#endif