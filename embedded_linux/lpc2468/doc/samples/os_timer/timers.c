/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2008 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates how to create and use RTOS timers.
 *
 *****************************************************************************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>

#define PROC1_STACK_SIZE 1024
#define PROC2_STACK_SIZE 1024
#define INIT_STACK_SIZE  1024

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 proc2Stack[PROC2_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];

static tTimer timer1;
static tTimer timer2;
static tTimer timer3;
static tTimer timer4;

static void proc1(void* arg);
static void proc2(void* arg);
static void callback1(void);
static void callback2(void);
static void callback3(void);
static void callback4(void);
static void initProc(void* arg);

/*****************************************************************************
 *
 * Description:
 *    The first function to execute 
 *
 ****************************************************************************/
int
main(void)
{
  tU8 error;
  tU8 pid;
  
  osInit();
  osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
  osStartProcess(pid, &error);
  
  osStart();
  return 0;
}

/*****************************************************************************
 *
 * Description:
 *    
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc1(void* arg)
{
  printf("Starting procTimer1\n");
  osCreateTimer(&timer1, callback1, TRUE, 50);
  osCreateTimer(&timer2, callback2, FALSE, 500);
  printf("Deleting procTimer1\n");
  osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc2(void* arg)
{
  printf("Starting procTimer2\n");
  osCreateTimer(&timer3, callback3, TRUE, 75);
  osCreateTimer(&timer4, callback4, FALSE, 1000);
  printf("Deleting procTimer2\n");
  osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    A timer callback function 
 *
 ****************************************************************************/
static void
callback1(void)
{
  printf("Callback 1\n");
}

/*****************************************************************************
 *
 * Description:
 *    A timer callback function 
 *
 ****************************************************************************/
static void
callback2(void)
{
  tU8 error;
  printf("Callback 2\n");
  osDeleteTimer(&timer1, &error);
}

/*****************************************************************************
 *
 * Description:
 *    A timer callback function 
 *
 ****************************************************************************/
static void
callback3(void)
{
  printf("Callback 3\n");
}

/*****************************************************************************
 *
 * Description:
 *    A timer callback function 
 *
 ****************************************************************************/
static void
callback4(void)
{
  tU8 error;
  printf("Callback 4\n");
  osDeleteTimer(&timer3, &error);
}

/*****************************************************************************
 *
 * Description:
 *    The entry function for the initialization process. 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
initProc(void* arg)
{
  tU8 error;
  tU8 pid;

  eaInit();
  osInitTimers(&error);
  printf("\n*******************************************************\n");
  printf("*                                                     *\n");
  printf("* Welcome to the pre-emptive operating system from    *\n");
  printf("* Embedded Artists, for the LPC2xxx microcontroller.  *\n");
  printf("*                                                     *\n");
  printf("* This is a sample application demonstrating how to   *\n");
  printf("* create and use timers.                              *\n");
  printf("*                                                     *\n");
  printf("*******************************************************\n");

  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid, 2, NULL, &error);
  osStartProcess(pid, &error);
  osCreateProcess(proc2, proc2Stack, PROC2_STACK_SIZE, &pid, 3, NULL, &error);
  osStartProcess(pid, &error);
  
  osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/
void
appTick(tU32 elapsedTime)
{
}
