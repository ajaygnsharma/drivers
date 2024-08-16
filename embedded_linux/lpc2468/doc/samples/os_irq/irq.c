/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2008 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates the use of timer interrupts
 *    and how interrupt service routines can signal events to processes.
 *
 *****************************************************************************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc246x.h>

#define CONSUMER_STACK_SIZE 1024
#define INIT_STACK_SIZE     1024

#define MAX_BUFFER_SIZE 500

static tU8 consumerStack[CONSUMER_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tCntSem mySem;

static void consumer(void* arg);
static void initProc(void* arg);
static void initTimer1Irq (void);
static void timer1Isr (void);

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
 *    The interrupt service routine that is exectuted whenever a timer
 *    counter 1 interrupt occurs. This function can be an ordinary
 *    C-function since the RTOS saves and restores all necessary registers.
 *
 ****************************************************************************/
static void
timer1Isr (void)
{
  static tU32 counter = 0;
  
  //
  //do some processing, for example fill a buffer with data...
  //
  
  //increment counter
  counter++;
  if (counter == MAX_BUFFER_SIZE)
  {
    tU8 error;

    counter = 0;
    osSemGive(&mySem, &error);
    if (error != OS_OK)
    {
      printf("\nERROR: Serious isr error...\n");
    }
  }

  T1IR   = 0xff;        //reset all IRQ flags
  VICVectAddr = 0x00;        //dummy write to end interrupt
}

/*****************************************************************************
 *
 * Description:
 *    Setup the Timer Counter 1 Interrupt to 500Hz interrupt rate
 *
 ****************************************************************************/
static void
initTimer1Irq (void)
{
  //set up timer
  T1TCR = 0x01;
  T1MR0 = (Fpclk / 500) - 1;     //Divide to get number of ticks for 500 Hz irq rate
  T1IR  = 0xff;                  //reset all flags before enable IRQs
  T1MCR = 0x03;

  //setup VIC
  VICIntSelect &= ~0x20;
  VICIntEnable |=  0x20;
  VICVectAddr5  = (tU32)timer1Isr;
  VICVectCntl5  = 0x05;
}

/*****************************************************************************
 *
 * Description:
 *    Will consume the buffers that are sent from the timer ISR.
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
    printf("Consumer: consuming imaginary buffer data...\n");
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
  printf("* create timer interrupts and signal events to        *\n");
  printf("* processes.                                          *\n");
  printf("*                                                     *\n");
  printf("*******************************************************\n");

  osSemInit(&mySem, 0);
  initTimer1Irq();
  osCreateProcess(consumer, consumerStack, CONSUMER_STACK_SIZE, &pid, 3, NULL, &error);
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
