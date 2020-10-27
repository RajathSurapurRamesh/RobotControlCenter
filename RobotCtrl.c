/*RobotCtrl.c*/
/*Rajath Surapur Ramesh*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "assert.h"
#include "includes.h"
#include "PktParser.h"
#include "RobotMgr.h"
#include "PBuffer.h"
#include "Memmgr.h"
#include "Framer.h"
#include "RobotCtrl.h"
#define step 0x07
#define RobotPrio 1
#define ROBOT_STK_SIZE 256

OS_SEM collision;
OS_Q MQueue[RobotNumber];//Maibox Queue
CPU_INT08U Path;
OS_ERR osErr;
Robotfloor RobotIdentity[RobotNumber];
CPU_STK RobotStk[RobotNumber][ROBOT_STK_SIZE];//RobotStack
OS_TCB RobotTCB[RobotNumber];//Task Ctrl Block
OS_MUTEX Mutex; //Mutex


/*------------Function Prototypes---*/

void RobotTask(void *data);
void Steps(CPU_INT08U Robot,CPU_INT08U x,CPU_INT08U y);
CPU_INT08U Step(CPU_INT08U CurrentX,CPU_INT08U CurrentY,CPU_INT08S X,CPU_INT08S Y);


typedef enum {NoStep=0,North,NorthEast,East,SouthEast,South,SouthWest,West,NorthWest}directions;
typedef enum{MoveRobot=0x02,followpath,loop,stp}Robotmessagetype;

//Create Robot Task
void CreateRobot(Robotfloor Robots)
{
    OS_ERR osErr;
    //Create Robot Queues
    OSQCreate(&RobotQueue[Robots.ControlRobotAddr],"Robot Queue",PoolSize, &osErr);
    assert(osErr == OS_ERR_NONE);
    //Create Maibox
    OSQCreate(&MQueue[Robots.ControlRobotAddr],"MailBox Queue",1,&osErr);
    assert(osErr == OS_ERR_NONE);
    //Create Robot Ctrl Task
    OSTaskCreate(&RobotTCB[Robots.ControlRobotAddr],
                 "RobotTask",
                 RobotTask,
                 &Robots,
                 RobotPrio,
                 &RobotStk[Robots.ControlRobotAddr][256],
                 ROBOT_STK_SIZE/ 10,
                 ROBOT_STK_SIZE,
                 0,
                 0,
                 0,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &osErr);

    assert(osErr == OS_ERR_NONE);
    RobotIdentity[Robots.ControlRobotAddr].ControlX = Robots.ControlX;
    RobotIdentity[Robots.ControlRobotAddr].ControlY = Robots.ControlY;
    


}

//RobotCtrl Task
void RobotTask(void *data)
{
    //typcasting ,derefferencing and storing it in the struct type of Robotloactions
    OS_ERR osErr; 
  Robotfloor Robot = *(Robotfloor *)data;
    
    OS_MSG_SIZE msgSize;
    //OS_ERR osErr;
    PBuffer *iBfr = NULL;
    for(;;)
    {
        if(iBfr==NULL)
        {
            iBfr =  OSQPend(&RobotQueue[Robot.ControlRobotAddr],
                            Timeout,
                            OS_OPT_PEND_BLOCKING,
                            &msgSize,
                            NULL,
                            &osErr);
            assert(osErr==OS_ERR_NONE);
        }
        Robotdata *Robots = (Robotdata *)iBfr->bfr; //typecasting the pbuffer into type of Robotdata
        
        switch(Robots->messageType)
        {
            case MoveRobot:
                Steps(Robot.ControlRobotAddr,Robots->command.direction.Pos->X,Robots->command.direction.Pos->Y);
                break;
            case followpath:
                for(CPU_INT08U i=0;i<=(Robots->Robotdatalength-5)/2;i++)              //subtracting the four header from the Robotdata and looping it for the remaining part
                {
                    Steps(Robot.ControlRobotAddr,Robots->command.direction.Pos[i].X,Robots->command.direction.Pos[i].Y);
                }
                break;
            case loop:
                while(RobotIdentity[Robot.ControlRobotAddr].STOP==0)
                {
                    for(CPU_INT08U j=0;j<=(Robots->Robotdatalength-5)/2;j++)          //subtracting the four header from the Robotdata and looping it for the remaining part
                    {
                        Steps(Robot.ControlRobotAddr,Robots->command.direction.Pos[j].X,Robots->command.direction.Pos[j].Y);
                    }
                    Steps(Robot.ControlRobotAddr,Robots->command.direction.Pos->X,Robots->command.direction.Pos->Y); //Going back to original location
                }
                RobotIdentity[Robot.ControlRobotAddr].STOP=0;
                break;
        }

        Free(iBfr);
        iBfr=NULL;
    }
}
void Steps(CPU_INT08U Robot,CPU_INT08U x,CPU_INT08U y)
{ 
   OS_ERR osErr;
  //OSSemCreate(&collision,"Empty Bfrs", 1, &osErr);
    //assert(osErr == OS_ERR_NONE);
   
    PBuffer *sBfr = NULL;

    while((RobotIdentity[Robot].ControlX !=x || RobotIdentity[Robot].ControlY!=y) && !RobotIdentity[Robot].STOP) //Looping until it reaches the destination address
    {
        CPU_INT08U CurrentX=RobotIdentity[Robot].ControlX;
        CPU_INT08U CurrentY=RobotIdentity[Robot].ControlY;
        CPU_INT08S difference1=x-CurrentX;
        CPU_INT08S difference2=y-CurrentY;
        if(sBfr==NULL)
            sBfr=Allocate();
        OSMutexPend(&Mutex,NULL,OS_OPT_PEND_BLOCKING,NULL,&osErr);               //applying mutex in order for Collision avoidance ,it avoides the framerquue sending the same address loaction
        //OSSemPend(&collision, Timeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);
        assert(osErr==OS_ERR_NONE);
        Path = Step(CurrentX,CurrentY,difference1,difference2);
        if(!Path==0)                                                         //making the previously occupied floor map to zero
            Floor[RobotIdentity[Robot].ControlX][RobotIdentity[Robot].ControlY]=0;
        else
            RobotIdentity[Robot].Retry = RobotIdentity[Robot].Retry+1;  //if the recivied direction was zero,then retrying it agian
        AddByte(sBfr,Path);
        AddByte(sBfr,step);
        AddByte(sBfr,Robot);
        OSQPost(&FramerQueue,sBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);//sending the updated  direction ,address of  robot to the framer queue
        assert(osErr==OS_ERR_NONE);
        HereIAM(Robot);                                                        //calling hereIam function ,this part gets pendded
       //OSSemPost(&collision,OS_OPT_POST_1,&osErr);
        //assert(osErr==OS_ERR_NONE);
        OSMutexPost(&Mutex,OS_OPT_POST_NONE,&osErr);                            //Posting to  the mutex
        if(RobotIdentity[Robot].Retry>=10)
        {
            ErrorState(100+Robot);
        }
        sBfr=NULL;
    }
    if(RobotIdentity[Robot].STOP)                                               //if the statement is true then executing the stop commnand
    {
        PBuffer *stBfr=NULL;
        if(stBfr==NULL)
            stBfr=Allocate();
        AddByte(stBfr,0);
        AddByte(stBfr,step);
        AddByte(stBfr,Robot);
        OS_ERR osErr;
        OSQPost(&FramerQueue,stBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
        assert(osErr==OS_ERR_NONE);
        HereIAM(Robot);
    }
}

CPU_INT08U Step(CPU_INT08U CurrentX,CPU_INT08U CurrentY,CPU_INT08S X,CPU_INT08S Y)//checking for the available loaction ,if the floor is free for that particular paramter of  diffrences and Current location  .Then it is occupying that floor position
{                                                                                      
    if(X==0 && Y>0)
    {
        if(Floor[CurrentX][CurrentY+1]==0)
            Path=North;
        else if(Floor[CurrentX+1][CurrentY+1]==0)
            Path=NorthEast;
        else if(Floor[CurrentX-1][CurrentY]==0)
            Path=NorthWest;
        else if(Floor[CurrentX-1][CurrentY]==0)
            Path=West;
        else if(Floor[CurrentX+1][CurrentY]==0)
            Path=East;
        else if(Floor[CurrentX][CurrentY-1]==0)
            Path=South;
        else if(Floor[CurrentX-1][CurrentY-1]==0)
            Path=SouthWest;
        else if(Floor[CurrentX+1][CurrentY-1]==0)
            Path=SouthEast;
        else
            Path=NoStep;
    }
    else if(X==0 && Y<0)
    {
        if(Floor[CurrentX][CurrentY-1]==0)
            Path=South;
        else if(Floor[CurrentX-1][CurrentY-1]==0)
            Path=SouthWest;
        else if(Floor[CurrentX+1][CurrentY-1]==0)
            Path=SouthEast;
        else if(Floor[CurrentX-1][CurrentY]==0)
            Path=West;
        else if(Floor[CurrentX+1][CurrentY]==0)
            Path=East;
        else if(Floor[CurrentX-1][CurrentY+1]==0)
            Path=NorthWest;
        else if(Floor[CurrentX][CurrentY+1]==0)
            Path=North;
        else if(Floor[CurrentX+1][CurrentY+1]==0)
            Path=NorthEast;
        else
            Path=NoStep;
    }
    else if(X>0 && Y==0)
    {
        if(Floor[CurrentX+1][CurrentY]==0)
            Path=East;
        else if(Floor[CurrentX+1][CurrentY+1]==0)
            Path=NorthEast;
        else if(Floor[CurrentX+1][CurrentY-1]==0)
            Path=SouthEast;
        else if(Floor[CurrentX][CurrentY+1]==0)
            Path=North;
        else if(Floor[CurrentX][CurrentY-1]==0)
            Path=South;
        else if(Floor[CurrentX-1][CurrentY+1]==0)
            Path=NorthWest;
        else if(Floor[CurrentX-1][CurrentY]==0)
            Path=West;
        else if(Floor[CurrentX-1][CurrentY-1]==0)
            Path=SouthWest;
        else
            Path=NoStep;
    }
    else if(X<0 && Y==0)
    {
        if(Floor[CurrentX-1][CurrentY]==0)
            Path=West;
        else if(Floor[CurrentX-1][CurrentY+1]==0)
            Path=NorthWest;
        else if(Floor[CurrentX-1][CurrentY-1]==0)
            Path=SouthWest;
        else if(Floor[CurrentX][CurrentY+1]==0)
            Path=North;
        else if(Floor[CurrentX][CurrentY-1]==0)
            Path=South;
        else if(Floor[CurrentX+1][CurrentY+1]==0)
            Path=NorthEast;
        else if(Floor[CurrentX+1][CurrentY]==0)
            Path=East;
        else if(Floor[CurrentX+1][CurrentY-1]==0)
            Path=SouthEast;
        else
            Path=NoStep;
    }
    else if(X>0 && Y>0)
    {
        if(Floor[CurrentX+1][CurrentY+1]==0)
            Path=NorthEast;
        else if(Floor[CurrentX][CurrentY+1]==0)
            Path=1;
        else if(Floor[CurrentX-1][CurrentY+1]==0)
            Path=NorthWest;
        else if(Floor[CurrentX-1][CurrentY]==0)
            Path=West;
        else if(Floor[CurrentX+1][CurrentY]==0)
            Path=East;
        else if(Floor[CurrentX][CurrentY-1]==0)
            Path=South;
        else if(Floor[CurrentX-1][CurrentY-1]==0)
            Path=SouthWest;
        else if(Floor[CurrentX+1][CurrentY-1]==0)
            Path=SouthEast;
        else
            Path=NoStep;
    }
    else if(X<0 && Y>0)
    {
        if(Floor[CurrentX-1][CurrentY+1]==0)
            Path=NorthWest;
        else if(Floor[CurrentX][CurrentY+1]==0)
            Path=1;
        else if(Floor[CurrentX-1][CurrentY]==0)
            Path=West;
        else if(Floor[CurrentX+1][CurrentY+1]==0)
            Path=NorthEast;
        else if(Floor[CurrentX-1][CurrentY-1]==0)
            Path=SouthWest;
        else if(Floor[CurrentX+1][CurrentY]==0)
            Path=East;
        else if(Floor[CurrentX][CurrentY-1]==0)
            Path=South;
        else if(Floor[CurrentX+1][CurrentY-1]==0)
            Path=SouthEast;
        else
            Path=NoStep;
    }
    else if(X>0 && Y<0)
    {
        if(Floor[CurrentX+1][CurrentY-1]==0)
            Path=SouthEast;
        else if(Floor[CurrentX][CurrentY-1]==0)
            Path=South;
        else if(Floor[CurrentX+1][CurrentY]==0)
            Path=East;
        else if(Floor[CurrentX-1][CurrentY-1]==0)
            Path=SouthWest;
        else if(Floor[CurrentX-1][CurrentY]==0)
            Path=West;
        else if(Floor[CurrentX+1][CurrentY+1]==0)
            Path=NorthEast;
        else if(Floor[CurrentX][CurrentY+1]==0)
            Path=1;
        else if(Floor[CurrentX-1][CurrentY+1]==0)
            Path=NorthWest;
        else
            Path=NoStep;
    }
    else if(X<0 && Y<0)
    {
        if(Floor[CurrentX-1][CurrentY-1]==0)
            Path=SouthWest;
        else if(Floor[CurrentX-1][CurrentY]==0)
            Path=West;
        else if(Floor[CurrentX][CurrentY-1]==0)
            Path=South;
        else if(Floor[CurrentX+1][CurrentY-1]==0)
            Path=SouthEast;
        else if(Floor[CurrentX+1][CurrentY]==0)
            Path=East;
        else if(Floor[CurrentX-1][CurrentY+1]==0)
            Path=NorthWest;
        else if(Floor[CurrentX][CurrentY+1]==0)
            Path=North;
        else if(Floor[CurrentX+1][CurrentY+1]==0)
            Path=NorthEast;
        else
            Path=NoStep;
    }
    return Path ;
}
//Updating robot Current Position
void HereIAM(CPU_INT08U CurrentRobot)
{

    PBuffer *hBfr=NULL;
    OS_MSG_SIZE msgSize;
    OS_ERR osErr;

    if (hBfr == NULL)
    {
        hBfr = OSQPend(&MQueue[CurrentRobot],
                       NULL,
                       OS_OPT_PEND_BLOCKING,
                       &msgSize,
                       NULL,
                       &osErr);
        assert(osErr==OS_ERR_NONE);
    }
    Here *CurrenRobotPosition = (Here *)hBfr->bfr;
    switch(CurrenRobotPosition->Address)
    {
        case hereiam:
            RobotIdentity[CurrentRobot].ControlX= CurrenRobotPosition->hereX;    //updating the X and y positon sent from the control center to the Current Robot ,
            RobotIdentity[CurrentRobot].ControlY = CurrenRobotPosition->hereY;   //updating the X and y positon sent from the control center to the Current Robot 
            Floor[RobotIdentity[CurrenRobotPosition->RobotAddress].ControlX][RobotIdentity[CurrenRobotPosition->RobotAddress].ControlY]=1; //updating the floor 
            break;
        case stp:
            RobotIdentity[CurrentRobot].STOP=TRUE;                               //if the command is to stop  then updating the particular robot to stop 
            break;
    }
    Free(hBfr);
}
