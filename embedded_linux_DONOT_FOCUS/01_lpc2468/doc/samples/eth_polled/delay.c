/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2008 Embedded Artists AB
 *
 *****************************************************************************/


/******************************************************************************
 * Includes
 *****************************************************************************/


#include <general.h>
#include "lpc24xx.h"
#include <config.h>

/******************************************************************************
 * Implementation of public functions
 *****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Delay function
 *
 * Params:
 *    [in] ms - number of ms to delay
 *
 ****************************************************************************/
void 
mdelay(tU32 ms)
{
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02;          //stop and reset timer
  T1PR  = 0x00;          //set prescaler to zero
  T1MR0 = ms * (Fpclk / 1000);
  T1IR  = 0xff;          //reset all interrrupt flags
  T1MCR = 0x04;          //stop timer on match
  T1TCR = 0x01;          //start timer

  //wait until delay time has elapsed
  while(T1TCR & 0x01)
    ;
}

