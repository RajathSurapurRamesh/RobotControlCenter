
/*framer.c*
/*Rajath Surapur Ramesh*/
#include "includes.h"
#include "PBuffer.h"
#include "PktParser.h"
#include "MemMgr.h"
#include "Framer.h"


/*Constant  */
#define Timeout 0
void PrintHeader();
#define FramerPrio 2 // Framer task priority
#define	FRAMER_STK_SIZE     256
#define PacketLength 0x09  //Packet Length
#define SourceAddress 0x02 //Source Address
#define DestinationAdd 0x01 //Destination Address
#define errormessage 0x0B
#define Ackn 0x0A
#define step 0x07  
#define Preambl1 0x03  //Preamble byte 1
#define Preambl2 0xaf  //Premmble byte 2
#define Preambl3 0xef  //Preamble byte 3

/*----- g l o b a l s -----*/
// Task Control Block
OS_TCB FramerTCB;
OS_Q FramerQueue;
void TxFlush(void);
/* Stack space for task stack */
CPU_STK FramerStk[FRAMER_STK_SIZE];

CPU_VOID CreateFramerTask(CPU_VOID)
{
  OS_ERR		osErr;/* -- OS Error code */
  
  /* Create Framer task. */	
  OSTaskCreate(&FramerTCB,
               "Framer Task",
               FramerTask, 
               NULL,
               FramerPrio,
               &FramerStk[0],
               FRAMER_STK_SIZE / 10,
               FRAMER_STK_SIZE,
               0,
               0,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  assert(osErr == OS_ERR_NONE);
}
/*----framer datatype---*/
typedef struct
{
  
  CPU_INT08U Code;
  CPU_INT08U Type;
  CPU_INT08U CurrentRobot;
}Framer;

 
CPU_VOID FramerTask(CPU_VOID *data)
{
  PBuffer *fBfr = NULL;
  OS_MSG_SIZE msgSize;
  OS_ERR osErr;
  for(;;)
  {
    if (fBfr == NULL)
    {
      //Wait until the buffer is arrived
      fBfr =  OSQPend(&FramerQueue,
                      Timeout,
                      OS_OPT_PEND_BLOCKING,
                      &msgSize,
                      NULL,
                      &osErr);
      assert(osErr==OS_ERR_NONE);
    }
 //typecasting to the framer structure type and checking which command we have received 
   Framer *framer = (Framer *)fBfr->bfr;
    switch(framer->Type)
    {
    case errormessage:
      PrintHeader();
    Framedata(DestinationAdd,errormessage,framer->Code);
    break;
    case Ackn:
      PrintHeader();
     Framedata(DestinationAdd,Ackn,framer->Code);
      break;
    case step:
      PrintHeader();
    Framedata(framer->CurrentRobot,step,framer->Code);
      break;
    }
    
  
    Free(fBfr);
    fBfr = NULL;
}
}
//Printing the header field 
void PrintHeader()
{
      PutByte(Preambl1);
      PutByte(Preambl2);
      PutByte(Preambl3);
      PutByte(PacketLength);
}

//printing the Robot information  

void Framedata(CPU_INT08U Robot,CPU_INT08U Type,CPU_INT08U code)
{
      PutByte(Robot);
      PutByte(SourceAddress);
      PutByte(Type);
      PutByte(code);
      PutByte(Preambl1^Preambl2^Preambl3^PacketLength^Robot^SourceAddress^Type^code);
      TxFlush();
}