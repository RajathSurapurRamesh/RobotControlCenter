/*RobotCtrl.h*/
/*Rajath Surapur Ramesh*/
#ifndef RobotCtrl_H
#define RobotCtrl_H
#define RobotNumber 17
#include "PktParser.h"
typedef struct
{
  CPU_INT08U ControlRobotAddr;
  CPU_INT08U ControlX;
  CPU_INT08U ControlY;
  CPU_BOOLEAN STOP;
  CPU_INT08U Retry;
}Robotfloor; 
typedef struct
{
    CPU_INT08U Address;
    CPU_INT08U RobotAddress;
    CPU_INT08U hereX;
    CPU_INT08U hereY;
}Here; //for the Hereiam and stop message 

/*Global Variables*/
extern OS_MUTEX Mutex;
extern OS_Q     MQueue[RobotNumber];
extern OS_Q     RobotQueue[RobotNumber];

/*Function Prototypes*/
void CreateRobot(Robotfloor Robots);
void HereIAM(CPU_INT08U CurrentRobot);
#endif