/*RobotMgr.h*/
/*Rajath Surapur Ramesh*/
#ifndef RobotMgr_H
#define RobotMgr_H
#include "includes.h"
#include "BfrPair.h"
#include "PBuffer.h"
#define hereiam 0x09
/*Global */
extern CPU_INT08U Floor[39][18];
extern OS_Q	ParserQueue;
/*Function Prototypes*/
void RobotManagerTask(void *data);
void AckCommand(CPU_INT08U Type);
void AddRobot(CPU_INT08U Robots);
void framemsg(PBuffer *bfr,CPU_INT08U CurrentRobot);
void mailboxqueue(PBuffer *bfr,CPU_INT08U CurrentRobot);
void HereIam(CPU_INT08U Robot,CPU_INT08U x,CPU_INT08U y);
void stop(CPU_INT08U Robot);

/*Robotdata structure datatype*/
typedef struct 
{
  CPU_INT08U  Robotdatalength;
  CPU_INT08U  destinationAddr;
  CPU_INT08U  sourceAddr;
  CPU_INT08U  messageType;
  union
  {
    struct
    {
      CPU_INT08U RobotAddress;
      struct
      {
        CPU_INT08U X;
        CPU_INT08U Y;
      }Pos[10];
    }direction;
  }command;
}Robotdata;

/*Function Prototypes*/
void CreateRobotManagerTask(void);

#endif