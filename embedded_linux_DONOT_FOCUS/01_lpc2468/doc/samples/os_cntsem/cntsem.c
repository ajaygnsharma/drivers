/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2008 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates how to create and use
 *    counting semaphores.
 *
 *****************************************************************************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>

#define PRODUCER_STACK_SIZE 1024
#define CONSUMER_STACK_SIZE 1024
#define INIT_STACK_SIZE     1024

static tU8 producerStack[PRODUCER_STACK_SIZE];
static tU8 consumerStack[CONSUMER_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tCntSem mySem;

static void producer(void* arg);
static void consumer(void* arg);
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
 *    Send imaginary information to consumer
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
producer(void* arg)
{
  tU8 error;

  printf("Starting producer\n");
  for(;;)
  {
    osSemGive(&mySem, &error);
    if(error == OS_ERROR_NULL)
    {
      printf("osSemGive: null pointer supplied");
    }

    printf("Producer: producing ...\n");
    osSleep(50);
  }
}

/*****************************************************************************
 *
 * Description:
 *    Consumes imaginary information sent from producer
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
consumer(void* arg)
{
  tU8 error;

  printf("Starting consumer\n");
  for(;;)
  {
    osSemTake(&mySem, 0, &error);
    if(error == OS_ERROR_NULL)
    {
      printf("osSemTake: null pointer supplied");
    }
    printf("Consumer: consuming ...\n");
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
  tU8 pid;

  eaInit();
  printf("\n*******************************************************\n");
  printf("*                                                     *\n");
  printf("* Welcome to the pre-emptive operating system from    *\n");
  printf("* Embedded Artists, for the LPC2xxx microcontroller.  *\n");
  printf("*                                                     *\n");
  printf("* This is a sample application demonstrating how to   *\n");
  printf("* create and use a counting semaphore.                *\n");
  printf("*                                                     *\n");
  printf("*******************************************************\n");

  osSemInit(&mySem, 0);

  osCreateProcess(consumer, consumerStack, CONSUMER_STACK_SIZE, &pid, 2, NULL, &error);
  osStartProcess(pid, &error);

  osCreateProcess(producer, producerStack, PRODUCER_STACK_SIZE, &pid, 3, NULL, &error);
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
