/*PKtParser.c*/
/*Rajath Surapur Ramesh*/
#include "PktParser.h"

#include "includes.h"
#include "error.h"
#include "Memmgr.h"
#include "PBuffer.h"
#include "Framer.h"
#define Preamble1Err 1  
#define Preamble2Err 2  
#define Preamble3Err 3
#define CheckSumErr  4
#define PacketlengthErr 5
#define HeaderLength         5
#define Preamble1            0x03
#define Preamble2            0xAF
#define Preamble3            0xEF
#define PacketLength         8
#define checkSumbyte         0x00
#define Timeout 0
/*c o n s t a n t */

#define ParserPrio 6  // Parser task priority

/* Size of the Process task stack */
#define	PARSER_STK_SIZE     256 

/*----- g l o b a l s -----*/

// Process Task Control Block
 static OS_TCB parserTCB;
OS_Q	ParserQueue;

  
/* Stack space for Process task stack */
 static CPU_STK parserStk[PARSER_STK_SIZE];




/*----- C r e a t e P a r s e r T a s k ( ) -----

PURPOSE
Create and initialize the Parser Task.
*/
CPU_VOID CreateParserTask(CPU_VOID)
{
	OS_ERR		osErr;/* -- OS Error code */

	/* Create Parser task. */	
  OSTaskCreate(&parserTCB,
               "Parser Task",
               ParsePkt, 
               NULL,
               ParserPrio,
               &parserStk[0],
               PARSER_STK_SIZE / 10,
               PARSER_STK_SIZE,
               0,
               100,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);

   assert(osErr == OS_ERR_NONE);
   OSQCreate(&ParserQueue, "Parser Queue",PoolSize, &osErr);
   
   assert(osErr == OS_ERR_NONE);
   OSQCreate(&FramerQueue, "Framer Queue",PoolSize, &osErr);
   assert(osErr == OS_ERR_NONE);
   
   
   
}
PBuffer *pBfr=NULL;

CPU_VOID ParsePkt(CPU_VOID *data)
{
  PBuffer *pBfr=NULL;
  static ParserState parseState = P1;                         /*current parser state*/ 
  CPU_INT16S c;                                       /*Getting next byte from file*/
  CPU_INT08U i=0;                                  /*initialization of local variable*/
  CPU_INT08U checkSum =0;
  for(;;)
  {
  c = GetByte();
   if (pBfr==NULL)
      pBfr=Allocate();
     if(c>=0)
    {  
      switch(parseState)
      {
        case P1:
     if (c==Preamble1)                               /*checking if the byte we got is equal to the Preambl1*/
      {   
        parseState=P2;
        checkSum=c;                                  /*putting the byte to checksum in order to xor with upcoing bytes*/
      }     
      else 
      {
        parseState=ER;                               /*changing parstate equal to error*/
        ErrorState(Preamble1Err);                   /*calling the  error function*/
      }
      break;
  case P2:
    if (c==Preamble2)                                
      {
        parseState=P3;
        checkSum=checkSum^c;                          /* xoring the previous byte with the current byte and storing it*/
      } 
      else
      {
        parseState=ER;                               
        ErrorState(Preamble2Err);                   
      }
      break;
  case P3:
    if (c==Preamble3)
      {
        parseState=L;
        checkSum=checkSum^c;
        
      }
      else 
      {
        parseState=ER;                              
      ErrorState(Preamble3Err);
      }
      break;
  case L:
                            
      if (c<PacketLength)                          
      {
        ErrorState(PacketlengthErr);
        parseState=ER;                             
      }
      else
      {
        
         AddByte(pBfr,(c-HeaderLength));
         
      parseState=D;
      checkSum=checkSum^c; 
      i=0;
      }
      break;
 case D:
    i++; 
    AddByte(pBfr,c);                       /*storing the coming bytes into the defined structure of payload */
    checkSum=checkSum^c;                          
    if (i>=pBfr->bfr[0])                
    {
      parseState=CheckSm;
    }
    break;
 case CheckSm:
    checkSum=checkSum^c;   
    if (checkSum==checkSumbyte)                  /*checking whether entire checksum byte is eqaul to zero*/
    {
      OS_ERR osErr;
      parseState=P1;
      
      OSQPost(&ParserQueue, pBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
      assert(osErr==OS_ERR_NONE);
      // Indicate that a new input buffer is needed.
      pBfr = NULL; 
    }
     else
     {
       ErrorState(CheckSumErr);
       parseState=ER;                           
       checkSum=0;
       i=0;
     }
    break;
      
    case ER:
     while (c==Preamble1)                   
     {
       c=GetByte();                          /*if yes get the next byte */
     if (c==Preamble2)                      
      {
        parseState=P3;                        /*changing  parsestate to P3*/
        checkSum=(Preamble1^Preamble2);       /*calculating the preamble1 and preamble 2 checksum*/
     }
        else
          break;
       }  
     break;
  }
}
  }
}

/*------------ErrorState()---------
//Purpose:Posts the semaphore closedpayloadBfrs and gets the error code*/
CPU_VOID ErrorState(CPU_INT08U errorcode)
{
  PBuffer *eBfr = NULL;
  
     if (eBfr == NULL)
     eBfr = Allocate();
     
  AddByte(eBfr,errorcode);
  AddByte(eBfr,0x0B);
  
  OS_ERR osErr;
  OSQPost(&FramerQueue,eBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
  eBfr=NULL;
}
