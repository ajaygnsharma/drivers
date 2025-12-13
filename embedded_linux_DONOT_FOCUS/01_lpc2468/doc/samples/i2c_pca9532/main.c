/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2006 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates how interface with a pca9532.
 *
 *****************************************************************************/

#include <general.h>
#include <printf_P.h>
#include <ea_init.h>
#include <lpc246x.h>
#include "pca9532.h"
#include "i2c.h"

void testI2C(void);

/*****************************************************************************
 *
 * Description:
 *    Delay execution by a specified number of milliseconds by using
 *    timer #1. A polled implementation.
 *
 * Params:
 *    [in] delayInMs - the number of milliseconds to delay.
 *
 ****************************************************************************/
static void
delayMs(tU16 delayInMs)
{
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02;          //stop and reset timer
  T1PR  = 0x00;          //set prescaler to zero
  T1MR0 = delayInMs * (Fpclk / 1000);
  T1IR  = 0xff;          //reset all interrrupt flags
  T1MCR = 0x04;          //stop timer on match
  T1TCR = 0x01;          //start timer
  
  //wait until delay time has elapsed
  while (T1TCR & 0x01)
    ;
}

/*****************************************************************************
 *
 * Description:
 *
 ****************************************************************************/
tU8
getKeys(void)
{
  tU8 commandString[] = {0x00};
  tU8 reg;


  //read current register value
  pca9532(commandString, 1, &reg, 1);
  
  return (~reg) & 0x0f;
}

/*****************************************************************************
 *
 * Description:
 *
 ****************************************************************************/
void
checkKeys(void)
{
  tU8 keys;
  
  keys = getKeys();

  if (keys & 0x01)
  {
    printf("Pushed key 1\n");
    IOCLR0  = 0x000F8000; 
    IOCLR1  = 0xFFFF0000;
    FIO2CLR = 0x0000FFFE;
    FIO2CLR = 0x1;
  }
  else if (keys & 0x02)
  {
    printf("Pushed key 2\n");
    FIO2SET = 0x1;
    IOSET1  = 0xFFFF0000;
    FIO2SET = 0x0000FFFE;
  }
  else if (keys & 0x04)
    printf("Pushed key 3\n");
  else if (keys & 0x08)
    printf("Pushed key 4\n");
}

/*****************************************************************************
 *
 * Description:
 *
 ****************************************************************************/
void
setLed(int led, int on)
{
  tU8 commandString[] = {0x08, 0x00};
  tU8 reg;

  //adjudt address if LED >= 5
  if (led >= 5)
    commandString[0] = 0x09;

    //read current register value
  pca9532(commandString, 1, &reg, 1);


  //update for new register value
  switch (led)
  {
  case 1:
  case 5:
    reg &= ~0x03;
    if (on == 1)
      reg |= 0x01;
    break;
  case 2:
  case 6:
    reg &= ~0x0c;
    if (on == 1)
      reg |= 0x04;
    break;
  case 3:
  case 7:
    reg &= ~0x30;
    if (on == 1)
      reg |= 0x10;
    break;
  case 4:
  case 8:
    reg &= ~0xc0;
    if (on == 1)
      reg |= 0x40;
    break;
  default:
    break;
  }
  commandString[1] = reg;
  pca9532(commandString, sizeof(commandString), NULL, 0);
}

/*****************************************************************************
 *
 * Description:
 *    The first function to execute 
 *
 ****************************************************************************/
int
main(void)
{
  IODIR0  |= 0x000F8000; 
  IODIR1  |= 0xFFF00000;
  FIO2DIR |= 0x0000FFFF;

  eaInit();
  i2cInit();

  //initialize PCA9532
  tU8 initCommand[] = {0x12, 0x97, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00};
  pca9532(initCommand, sizeof(initCommand), NULL, 0);

  printf("\n***************************************************");
  printf("\n*                                                 *");
  printf("\n* Test program for LPC24xx EDU Board...           *");
  printf("\n* (C) Embedded Artists 2008                       *");
  printf("\n*                                                 *");
  printf("\n* Test - PCA9532                                  *");
  printf("\n*                                                 *");
  printf("\n***************************************************\n");
  
  for(;;)
  {
    tU8 i;

    //control LEDs
    static tU8 ledCnt = 1;

    switch (ledCnt)
    {
    case 1:  setLed(1, 1); ledCnt++; break;
    case 2:  setLed(2, 1); ledCnt++; break;
    case 3:  setLed(3, 1); ledCnt++; break;
    case 4:  setLed(4, 1); ledCnt++; break;
    case 5:  setLed(1, 0); ledCnt++; break;
    case 6:  setLed(2, 0); ledCnt++; break;
    case 7:  setLed(3, 0); ledCnt++; break;
    case 8:  setLed(4, 0); ledCnt++; break;
    case 9:  setLed(5, 1); ledCnt++; break;
    case 10: setLed(6, 1); ledCnt++; break;
    case 11: setLed(7, 1); ledCnt++; break;
    case 12: setLed(8, 1); ledCnt++; break;
    case 13: setLed(5, 0); ledCnt++; break;
    case 14: setLed(6, 0); ledCnt++; break;
    case 15: setLed(7, 0); ledCnt++; break;
    case 16: setLed(8, 0); ledCnt=1; break;
    default: ledCnt=1; break;
    }
    
    for(i = 0; i < 5; i++)
    {
      checkKeys();
      delayMs(10);
    }
  }
}
