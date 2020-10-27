/*SerIOdriver.c*/
/*Rajath Surapur Ramesh*/

#include "Includes.h"
#include "SerIODriver.h"

#include "assert.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define USART_RXNE  0x20  // Rx not Empty Status Bit
#define USART_TXE   0x80  // Tx Empty Status Bit
#define USART_RXNEIE  0x20  // Rx Not Empty Interrupt Mask
#define USART_TXEIE   0x80  // Tx Empty Interrupt Mask
#define SETENA1 (*((CPU_INT32U*)0xE000E104))
#define USART2ENA 0x00000040
#define Timeout 0
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

// If not already defined, use the default buffer size of 4.
#ifndef BfrSize
#define BfrSize 1
#endif

 OS_SEM	openOBfrs;	 
 OS_SEM	closedIBfrs;	
// Allocate the input buffer pair.
static BfrPair iBfrPair;
static CPU_INT08U iBfr0Space[BfrSize];
static CPU_INT08U iBfr1Space[BfrSize];

// Allocate the output buffer pair.
static BfrPair oBfrPair;
static CPU_INT08U oBfr0Space[BfrSize];
static CPU_INT08U oBfr1Space[BfrSize];



void InitSerIO(void)
{
  OS_ERR      err;


  BfrPairInit(&iBfrPair, iBfr0Space, iBfr1Space, BfrSize);
  BfrPairInit(&oBfrPair, oBfr0Space, oBfr1Space, BfrSize);
  OSSemCreate(&openOBfrs, "Empty Bfrs", 1, &err);
   OSSemCreate(&closedIBfrs, "Full Buffers", 0, &err);
    assert(err == OS_ERR_NONE);
  USART2->CR1 |= USART_TXEIE;
  USART2->CR1 |= USART_RXNEIE;
  SETENA1 = USART2ENA;
  
}

CPU_INT16S PutByte(CPU_INT16S c)
{
  OS_ERR      osErr;

  if (PutBfrClosed(&oBfrPair))
    {
    OSSemPend(&openOBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    assert(osErr==OS_ERR_NONE);

    BfrPairSwap(&oBfrPair);
    }  
   
 CPU_INT16S Char= PutBfrAddByte(&oBfrPair, c);
  
  USART2->CR1 |= USART_TXEIE;

  return Char;
}
void ServiceTx(void)
{
  OS_ERR      osErr;
 
  if (!(USART2->SR & USART_TXE))
    return;
  
  if (!GetBfrClosed(&oBfrPair))
    {
    USART2->CR1 &= ~USART_TXEIE;
    return; 
    }
   
  USART2->DR =GetBfrRemByte(&oBfrPair);

  if (!GetBfrClosed(&oBfrPair))
    {
    OSSemPost(&openOBfrs, OS_OPT_POST_1, &osErr);
    assert(osErr==OS_ERR_NONE);
    }
}

CPU_INT16S GetByte(void)
{
  OS_ERR      osErr;

  CPU_INT16S c;
  
  if (!GetBfrClosed(&iBfrPair))
    {
   OSSemPend(&closedIBfrs, Timeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);	
    assert(osErr==OS_ERR_NONE);
    BfrPairSwap(&iBfrPair);
  }		
  c =GetBfrRemByte(&iBfrPair);
  USART2->CR1 |= USART_RXNEIE;

  return c;
}  

void ServiceRx(void)
{
  OS_ERR      osErr;

  if (!(USART2->SR & USART_RXNE))
    return;

  if (PutBfrClosed(&iBfrPair))
    {
    USART2->CR1 &= ~USART_RXNEIE;
    return;
    }
    
  PutBfrAddByte(&iBfrPair, (CPU_INT16S) USART2->DR);

  if (PutBfrClosed(&iBfrPair))
    {
    OSSemPost(&closedIBfrs, OS_OPT_POST_1, &osErr);
    assert(osErr==OS_ERR_NONE);
    }

}

void TxFlush(void)
{
  OS_ERR      osErr; 

  ClosePutBfr(&oBfrPair);

  OSSemPend(&openOBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
  assert(osErr==OS_ERR_NONE);

  BfrPairSwap(&oBfrPair);

  USART2->CR1 |= USART_TXEIE;
}




/*--------------- S e r P r i n t f ( ) ---------------

PURPOSE:
Provides functionality of standard C printf( ) function printing
to the RS232 Tx rather than to the screen.

INPUT PARAMETERS:
format  - Standard printf format string
...     - Zero or more parameters giving values to print according
          to the "format" format string
*/


/*--------------- E x t I n t e r r u p t ( ) ---------------*/

/*
PURPOSE
External interrupt exception handler. 
Handle rx and tx interrupts.
*/
void SerialISR(void)
{
  /* Disable interrupts. */
  CPU_SR_ALLOC();
  OS_CRITICAL_ENTER();  
  
	/* Tell kernel we're in an ISR. */
	OSIntEnter();

  /* Enable interrupts. */
  OS_CRITICAL_EXIT();
	
  // Service rx interrupt.
  ServiceRx();

  // Service tx interrupt.
  ServiceTx();

	/* Give the O/S a chance to swap tasks. */
	OSIntExit ();
}
