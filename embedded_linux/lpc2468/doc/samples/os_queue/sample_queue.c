/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2008 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates to to create and use the queue
 *    synchronization primitive.
 *
 *****************************************************************************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>

#define SENDER_STACK_SIZE   1024
#define RECEIVER_STACK_SIZE 1024
#define INIT_STACK_SIZE     1024
#define QUEUE_AREA_SIZE       50

static tU8 senderStack[SENDER_STACK_SIZE];
static tU8 receiverStack[RECEIVER_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static void* pQueueArea[QUEUE_AREA_SIZE];
static tQueue myQueue;

static void sender(void* arg);
static void receiver(void* arg);
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
sender(void* arg)
{
  tU8 counter = 1;
  tU8 error;
  printf("Starting sender\n");
  for(;;)
  {
    osPostQueue(&myQueue, (void*)counter, &error);
    if(error == OS_ERROR_QUEUE_FULL)
    {
      printf("osPostQueue: queue full\n");
    }

    printf("Sender: %d posted\n", counter++); 
    osSleep(100);
  }
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
receiver(void* arg)
{
  tU8 counter;
  tU8 error;
  printf("Starting receiver\n");
  for(;;)
  {
    counter = (tU8)osPendQueue(&myQueue, 0, &error);
    printf("Receiver: %d received\n", counter);
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
  printf("* create and use a queue.                             *\n");
  printf("*                                                     *\n");
  printf("*******************************************************\n");

  osCreateQueue(&myQueue, pQueueArea, QUEUE_AREA_SIZE);
  osCreateProcess(sender, senderStack, SENDER_STACK_SIZE, &pid, 2, NULL, &error);
  osStartProcess(pid, &error);
  osCreateProcess(receiver, receiverStack, RECEIVER_STACK_SIZE, &pid, 3, NULL, &error);
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
