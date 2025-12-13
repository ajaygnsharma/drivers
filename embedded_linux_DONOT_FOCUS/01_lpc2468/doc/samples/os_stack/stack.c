/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2008 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates how to check the stack usage of
 *    processes.
 *
 *****************************************************************************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>

#define PROC1_STACK_SIZE 1024
#define PROC2_STACK_SIZE 1024
#define INIT_STACK_SIZE  1024

#define DUMMY_SIZE 100  /* use approximately 10 percent more of the stack for demonstration */

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 proc2Stack[PROC2_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;
static tU8 pid2;

static void proc1(void* arg);
static void proc2(void* arg);
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
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc1(void* arg)
{
  tU8  dummy[DUMMY_SIZE]; /* declare local variable, which will be placed on the local stack */
  tU16 cnt;

  /* fill dummy variable (placed on stack) with dummy values */
  for (cnt=0;
       cnt<DUMMY_SIZE;
       cnt++)
    dummy[cnt] = 0x11;

  for(;;)
  {
    printf("Process: 1\n");
    osSleep(50);
  }
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function. 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc2(void* arg)
{
  for(;;)
  {
    printf("Process: 2\n");
    osSleep(50);
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
  tU8 error;

  eaInit();
  printf("\n*******************************************************\n");
  printf("*                                                     *\n");
  printf("* Welcome to the pre-emptive operating system from    *\n");
  printf("* Embedded Artists, for the LPC2xxx microcontroller.  *\n");
  printf("*                                                     *\n");
  printf("* This is a sample application demonstrating how to   *\n");
  printf("* check the stack usage of a process.                 *\n");
  printf("*                                                     *\n");
  printf("*******************************************************\n");
  
  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 2, NULL, &error);
  osStartProcess(pid1, &error);
  osCreateProcess(proc2, proc2Stack, PROC2_STACK_SIZE, &pid2, 3, NULL, &error);
  osStartProcess(pid2, &error);
  
  /* Sleep for a while and then measure the stack usage. In a real application 
     the stack measurement must be done after a longer time to get a good
     picture of the worst case. In a real application it is also better to 
     put the measurment in process with lower priority and not in the init process. */
  osSleep(500);
  printf("Stack usage process 1: %d %%\n", osStackUsage(pid1));
  printf("Stack usage process 2: %d %%\n", osStackUsage(pid2));
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
