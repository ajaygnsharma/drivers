/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2008 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates the use of multiple instantiation
 *    processes.
 *
 *****************************************************************************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>

#define PROC1_STACK_SIZE  1024
#define PROC2A_STACK_SIZE 1024
#define PROC2B_STACK_SIZE 1024
#define INIT_STACK_SIZE   1024

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 proc2aStack[PROC2A_STACK_SIZE];
static tU8 proc2bStack[PROC2B_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;
static tU8 pid2a;
static tU8 pid2b;

static void proc1(void* arg);
static void proc2(void* arg);
static void initProc(void* arg);

typedef struct
{
  tU32 param1;
  tU8  param2;
} tProcParams;

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
 *    A process entry function 
 *
 * Params:
 *    [in] arg - The input parameters to the instantiation of the process. 
 *
 ****************************************************************************/
static void
proc1(void* arg)
{
  tProcParams *pArg = (tProcParams *)arg;

  printf("\nStarting process #1 with the following parameters\n");
  printf("  param1 = %d\n  param2 = %c\n", pArg->param1, pArg->param2);

  for(;;)
  {
    printf("Process: 1 running...\n");
    osSleep(pArg->param1);
  }
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function. 
 *
 * Params:
 *    [in] arg - The input parameters to the instantiation of the process. 
 *
 ****************************************************************************/
static void
proc2(void* arg)
{
  tProcParams *pArg = (tProcParams *)arg;

  printf("\nStarting process #2 with the following parameters\n");
  printf("  param1 = %d\n  param2 = %c\n", pArg->param1, pArg->param2);
  
  for(;;)
  {
    printf("Process: 2%c running...\n", pArg->param2);
    osSleep(pArg->param1);
  }
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
  tU8                error;
  static tProcParams procData1;
  static tProcParams procData2a;
  static tProcParams procData2b;

  eaInit();
  printf("\n*******************************************************\n");
  printf("*                                                     *\n");
  printf("* Welcome to the pre-emptive operating system from    *\n");
  printf("* Embedded Artists, for the LPC2xxx microcontroller.  *\n");
  printf("*                                                     *\n");
  printf("* This is a sample application demonstrating how to   *\n");
  printf("* create and start multiple instantiation processes.  *\n");
  printf("*                                                     *\n");
  printf("*******************************************************\n");
  
  procData1.param1 = 60;
  procData1.param2 = '1';
  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 2, (void*)&procData1, &error);
  osStartProcess(pid1, &error);

  procData2a.param1 = 100;
  procData2a.param2 = 'A';
  osCreateProcess(proc2, proc2aStack, PROC2A_STACK_SIZE, &pid2a, 3, (void*)&procData2a, &error);
  osStartProcess(pid2a, &error);

  procData2b.param1 = 50;
  procData2b.param2 = 'B';
  osCreateProcess(proc2, proc2bStack, PROC2B_STACK_SIZE, &pid2b, 4, (void*)&procData2b, &error);
  osStartProcess(pid2b, &error);

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
