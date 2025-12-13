/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2008 Embedded Artists AB
 *
 * Description:
 *
 *****************************************************************************/

#include <general.h>
#include <printf_P.h>
#include <ea_init.h>
#include <lpc246x.h>

extern char wavSound[];

tU32 wavSoundSize();

static void udelay( unsigned int delayInUs )
{
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02;          //stop and reset timer
  T1PR  = 0x00;          //set prescaler to zero
  T1MR0 = (((long)delayInUs-1) * (long)Fpclk/1000) / 1000;
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
int main(void)
{
  short* pData = NULL;
  tU32 size = 0;
  tU32 cnt = 0;
  tU32 samples = 0;
  tU32 numSamples;
  tU32 sampleRate;
  tU16 usDelay;

  eaInit();

  printf("\n***************************************************");
  printf("\n*                                                 *");
  printf("\n* Test program for LPC2478 OEM Board...           *");
  printf("\n* (C) Embedded Artists 2008                       *");
  printf("\n*                                                 *");
  printf("\n* Test - Analog Audio                             *");
  printf("\n*                                                 *");
  printf("\n***************************************************\n");

  //Initialize DAC: AOUT = P0.26
  PINSEL1 &= ~0x00300000;
  PINSEL1 |=  0x00200000;

  pData = (short*)&wavSound[0];
  size = wavSoundSize();

  //check if RIFF file ("RIFF" at beginning of file)
  if ((pData[0] != 0x4952) || (pData[1] != 0x4646))
  {
    printf("\nERROR not a RIFF file!\n");
    return -1;
  }
  //check if WAV file ("WAVE" at beginning of file)
  if ((pData[4] != 0x4157) || (pData[5] != 0x4556))
  {
    printf("\nERROR not a WAV file!\n");
    return -1;
  }
  //check if 'fmt ' tag
  if ((pData[6] != 0x6D66) || (pData[7] != 0x2074))
  {
    printf("\nERROR no 'fmt ' tag found!\n");
    return -1;
  }
  //printf info about WAV file ("WAVE" at beginning of file)
  printf("\nInfo: 'fmt ' chunk size = %d (0x%x)", pData[8] + 256*256*pData[9], pData[8] + 256*256*pData[9]);
  printf("\nInfo: format tag = %d (1=no compression)", pData[10]);
  printf("\nInfo: audio channels = %d", pData[11]);
  printf("\nInfo: samples per second = %d", pData[12] + 256*256*pData[13]);
  sampleRate = pData[12] + 256*256*pData[13];
  printf("\nInfo: bytes per second = %d", pData[14] + 256*256*pData[15]);
  printf("\nInfo: byte alignment = %d", pData[16]);
  printf("\nInfo: bits per sample = %d", pData[17]);

  //search for 'data' tag
  cnt = 10 + pData[8]/2;
  while((pData[cnt] != 0x6164) && (pData[cnt+1] != 0x6174) && (cnt<size/2))
    cnt++;

  if (cnt >= size/2)
  {
    printf("\nERROR no 'data' tag found!\n");
    return -1;
  }  
  //get data (i.e., sample size)
  numSamples = (pData[cnt+2] + 256*256*pData[cnt+3]) / (pData[17]/8);
  printf("\nInfo: Number of samples = %d (0x%x)\n\n", numSamples, numSamples);
  
  DACR = 0x7fc0;
      
  cnt++;
  samples = 0;
  usDelay = 1000000/ sampleRate + 1;

  while(1)
  {
    samples = 0;
    cnt = 11 + pData[8]/2;
  
    while(samples++ < numSamples)
    {
      tS32 val;
      
      val = pData[cnt++];
      DACR = ((val + 0x7fff) & 0xffc0); // |  //actual value to output
  
      udelay(usDelay);
    }
    while ((FIO2PIN & 0x00000400))
    {
    }
    printf("\nP2.10 pressed!");
  }
  
  return 0;
}
