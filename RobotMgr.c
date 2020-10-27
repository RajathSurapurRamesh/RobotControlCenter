/*RobotMgr.c*/
/*Rajath Surapur Ramesh*/
#include "SerIODriver.h"
#include "RobotMgr.h"
#include "Memmgr.h"
#include "Framer.h"
#include "PktParser.h"
#include "RobotCtrl.h"

/*c o n s t a n t */
#define BaudRate 9600
#define RobotManagerPrio 2
#define Timeout 0 // Timeout for semaphore wait
#define reset 0x00
#define ADDRobot 0x01
#define stoprobot 0x05
#define LocationOccupied 13
#define AddBadLocation 12
#define RobotExists 14
#define Present 1
#define AddBadRobotAddress 11
#define MBadRobotAddress 21
#define MNonexistentRobot 22
#define MBadLocation 23
#define FBadRobotAddress 31
#define FNonexistentRobot 32
#define FBadLocation 33
#define LBadRobotAddress 41
#define LNonexistentRobot 42
#define LBadLocation 43
#define SBadRobotAddress 51
#define SNonexistentRobot 52
#define BadMessageType 61
#define Ackn 0x0A
#define MaxXaxis 39
#define MaxYaxis 18
#define MaxRobotAd 15
#define  MinRobotAd  3
#define ROBOTMANAGER_STK_SIZE 128
static OS_TCB RobotManagerTCB;
static CPU_STK	RobotManagerStk[ROBOTMANAGER_STK_SIZE];
OS_Q RobotQueue[RobotNumber];
Robotfloor robot;
CPU_INT08U Floor[39][18];
CPU_INT08U Robot[16];
CPU_BOOLEAN stopRobot[13];
typedef enum{MoveRobot=0x02,followpath,loop,stp}messagetype;
//create Robotmanagertask
void CreateRobotManagerTask(void)
{
    OS_ERR osErr;
    OSMutexCreate(&Mutex,"Floor Ctrl",&osErr);
    assert(osErr==OS_ERR_NONE);
    OSTaskCreate(&RobotManagerTCB,
                 "Robot Manager Task",
                 RobotManagerTask,
                 NULL,
                 RobotManagerPrio,
                 &RobotManagerStk[0],
                 ROBOTMANAGER_STK_SIZE / 10,
                 ROBOTMANAGER_STK_SIZE,
                 0,
                 0,
                 NULL,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &osErr);

    assert(osErr == OS_ERR_NONE);
}

/*---------PayloadTask()-------------
Purpose : Takes the payload and prints the appropiate direction */

void RobotManagerTask(void *data)
{
    OS_ERR osErr;
    
    PBuffer *mBfr = NULL;	/* -- Current input buffer */
    OS_MSG_SIZE msgSize;

for(;;)
 {
        /* If input buffer is not assigned, get a filled buffer from the parser queue. */
if (mBfr == NULL)
        {
            mBfr =  OSQPend(&ParserQueue,
                            Timeout,
                            OS_OPT_PEND_BLOCKING,
                            &msgSize,
                            NULL,
                            &osErr);
            assert(osErr==OS_ERR_NONE);
        }
//transferring the data from thr robotdata strucutre into Robotfloor structure positions, in order to register the added robot and its location on floor
        Robotdata *RobotBfr = (Robotdata *)mBfr->bfr;
        robot.ControlRobotAddr=RobotBfr->command.direction.RobotAddress;;
        robot.ControlX=RobotBfr->command.direction.Pos->X;
        robot.ControlY=RobotBfr->command.direction.Pos->Y;
        switch(RobotBfr->messageType)
        {
            case reset:
                NVIC_GenerateCoreReset();                                       //Reset command
                break;
            case ADDRobot:
                
                if(!(robot.ControlRobotAddr<=MaxRobotAd&&robot.ControlRobotAddr>=MinRobotAd))//Check if the robot address is between valid address*/
                ErrorState(AddBadRobotAddress);
                else  if(!(Robot[robot.ControlRobotAddr]!=Present))             //checking whetehr the robot previously exists or not 
                    ErrorState(RobotExists);
                else if(!(robot.ControlX<=MaxXaxis && robot.ControlY<=MaxYaxis)) //checking whether the x and y location sent from the control center is between the valid x and y axis 
                    ErrorState(AddBadLocation);
                else if(!(Floor[robot.ControlX][robot.ControlY]==0))
                    ErrorState(LocationOccupied);
                else { 
                    CreateRobot(robot);
                    Robot[robot.ControlRobotAddr]=Present;                       //assigning present status to the particular Robotaddress
                    Floor[robot.ControlX][robot.ControlY]=Present;               //assinging present status to the particular x and y axis 
                    AckCommand(RobotBfr->messageType);
                    }
                    Free(mBfr);        
                    break;
                                
       case MoveRobot :                                                             //Check for the valid  locations and send the same valid  move command to robotctrl

                if(!(robot.ControlRobotAddr<=MaxRobotAd&&robot.ControlRobotAddr>=MinRobotAd))
                     ErrorState(MBadRobotAddress);
                else if(!(Robot[robot.ControlRobotAddr]))
                     ErrorState(MNonexistentRobot);
                else if(!(robot.ControlX<=MaxXaxis &&robot.ControlY<=MaxYaxis))
                      ErrorState(MBadLocation);
                else
                {
                framemsg(mBfr,robot.ControlRobotAddr);                           //calling the frame message in order to post it to robot queue 
                AckCommand(RobotBfr->messageType);                               //sending ackn command 
                }
                break;
            case followpath:                                                      //Check for valid  locations and send the same valid followpath  command to the robotcrtl

                if(!(robot.ControlRobotAddr<=MaxRobotAd&&robot.ControlRobotAddr>=MinRobotAd))
                  ErrorState(FBadRobotAddress);
                else if(!(Robot[robot.ControlRobotAddr]))
                  ErrorState(FNonexistentRobot);
                else  if(!(robot.ControlX<=MaxXaxis && robot.ControlY<=MaxYaxis))
                  ErrorState(FBadLocation);
                else
                  framemsg(mBfr,robot.ControlRobotAddr);
                  AckCommand(RobotBfr->messageType);
                  break;
            case loop:                                                                  //Check for valid  locations and send the same valid loop command to the robot ctrl

                if(!(robot.ControlRobotAddr<=MaxRobotAd&&robot.ControlRobotAddr>=MinRobotAd))
                    ErrorState(LBadRobotAddress);
                else if(!(Robot[robot.ControlRobotAddr]))
                    ErrorState(LNonexistentRobot);
                else if(!(robot.ControlX<=MaxXaxis&& robot.ControlY<=MaxYaxis))
                     ErrorState(LBadLocation);
                else
                 {
                      framemsg(mBfr,robot.ControlRobotAddr);
                      AckCommand(RobotBfr->messageType);
                 }
                 
                 break;
            case hereiam:                                                          
               
               robot.ControlY=robot.ControlX;
               robot.ControlX=robot.ControlRobotAddr; 
               HereIam(RobotBfr->sourceAddr,robot.ControlX,robot.ControlY);      //sending X and y positon to HereIam function
               Free(mBfr);
                break;
            case stp:                                                             
                AckCommand(stoprobot);
                if(!(robot.ControlRobotAddr<=MaxRobotAd&&robot.ControlRobotAddr>=MinRobotAd))
                    ErrorState(SBadRobotAddress);
                else if(!(Robot[robot.ControlRobotAddr]))
                    ErrorState(SNonexistentRobot); 
                else
                      stop(robot.ControlRobotAddr);                             //sending robotaddress to stop 
                Free(mBfr);
                break;
         }
        mBfr=NULL;
    }
}

void framemsg(PBuffer *bfr,CPU_INT08U CurrentRobot)
{
    OS_ERR osErr;
    OSQPost(&RobotQueue[CurrentRobot],bfr,sizeof(PBuffer),OS_OPT_POST_FIFO, &osErr);
    assert(osErr==OS_ERR_NONE);
}

void AckCommand(CPU_INT08U Type)
{
    PBuffer *oBfr = NULL;
    if (oBfr == NULL)
        oBfr = Allocate();
    AddByte(oBfr,Type);
    AddByte(oBfr,Ackn);
    OS_ERR osErr;
    OSQPost(&FramerQueue,oBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);      
    assert(osErr==OS_ERR_NONE);

}

void HereIam(CPU_INT08U Robot,CPU_INT08U x,CPU_INT08U y)
{
    PBuffer *hrBfr = NULL;
    if (hrBfr == NULL)
        hrBfr = Allocate();
    AddByte(hrBfr,hereiam);
    AddByte(hrBfr,Robot);
    AddByte(hrBfr,x);
    AddByte(hrBfr,y);
   mailboxqueue(hrBfr,Robot);                                                    //Here i am command posted to the mailbox
}


void stop(CPU_INT08U Robot)
{
    PBuffer *srBfr = NULL;
    if (srBfr == NULL)
        srBfr = Allocate();
    AddByte(srBfr,stp);
    AddByte(srBfr,Robot);
    mailboxqueue(srBfr,Robot);                                                   //Stop command posted to the mailbox
}

void mailboxqueue(PBuffer *bfr,CPU_INT08U CurrentRobot)
{
  
 OS_ERR osErr;
  OSQPost(&MQueue[CurrentRobot],bfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
}
