/*******************************************************************************
    This module implements a linux character device driver for the QVGA chip.
    Copyright (C) 2006  Embedded Artists AB (www.embeddedartists.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*******************************************************************************/
#include "lpc246x.h"				/* LPC23xx/24xx definitions */
#include "lcd_hw.h"
#include <general.h>
#include <config.h>
#include <printf_P.h>

/******************************************************************************
** Function name:		mdelay
**
** Descriptions:		
**
** parameters:			delay length
** Returned value:		None
** 
******************************************************************************/
void mdelay( unsigned int delayInMs )
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


/******************************************************************************
** Function name:		lcd_hw_init
**
** Descriptions:		Initialize hardware for LCD controller, it includes
**						both EMC and I/O initialization. External CS2 is used 
**						for LCD controller.		
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void lcd_hw_init(void)
{
  unsigned int regVal;

#if (QVGA_LCD_IF == PARALLEL_16_LCD_IF) || (QVGA_LCD_IF == PARALLEL_8_LCD_IF)
  /****************************************************************
   * Initialize EMC for CS2
   ****************************************************************/
  regVal = PINSEL4;
  regVal &= 0x0FFFFFFF;
  regVal |= 0x50000000;
  PINSEL4 = regVal;
 
  regVal = PINSEL5;
  regVal &= 0xF0F0F000;
  regVal |= 0x05050555;
  PINSEL5 = regVal;

  PINSEL6 = 0x55555555;
  PINSEL8 = 0x55555555;

  regVal = PINSEL9;
  regVal &= 0x0F000000;
  regVal |= 0x50555555;
  PINSEL9 = regVal;

  // Initialize EMC for CS2
  EMC_STA_CFG2 = 0x00000081;	/* 16 bit, byte lane state, BLSn[3:0] are low. */ 
  EMC_STA_WAITWEN2  = 0x1;		/* WE delay 2(n+1)CCLK */
  EMC_STA_WAITOEN2  = 0x2;		/* OE delay, 2(n)CCLK */
  EMC_STA_WAITRD2   = 0x10;		/* RD delay, 17(n+1)CCLK */ 
  EMC_STA_WAITPAGE2 = 0x1F;		/* Page mode read delay, 32CCLK(default) */
  EMC_STA_WAITWR2   = 0x8;		/* Write delay, 10(n+2)CCLK */
  EMC_STA_WAITTURN2 = 0x5;		/* Turn arounc delay, 5(n+1)CCLK */
  EMC_STA_EXT_WAIT  = 0x0;		/* Extended wait time, 16CCLK */
#endif

#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)
  //init SPI #0
  PINSEL0 |= 0xc0000000;
  PINSEL1 |= 0x0000003f;
  IODIR0  |= 0x1c0;
  IOSET0   = 0x1c0;

  S0SPCCR = 57;    
  S0SPCR  = 0x38;
#endif

  /****************************************************************
   * Setup control of backlight
   ****************************************************************/
  FIO3DIR = BACKLIGHT_PIN;
#if 0
  FIO3SET = BACKLIGHT_PIN;		/* in V1.0 board, set pins high = turn light on */
#else
  FIO3CLR = BACKLIGHT_PIN;		/* in V1.1 board, set pins low = turn light on */
#endif
  return;
}

#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)

#define ACTIVATE_CS   IOCLR0 = 0x40
#define DEACTIVATE_CS IOSET0 = 0x40
#define SET_RS        IOSET0 = 0x80
#define RESET_RS      IOCLR0 = 0x80
#define SET_READ      IOSET0 = 0x100
#define SET_WRITE     IOCLR0 = 0x100

static unsigned char
sendSpi(unsigned char indata)
{
	unsigned int failsafe;
	
  S0SPDR = indata;
  failsafe = 0;
  while(((S0SPSR & 0x80) == 0) && (failsafe < 5000))
    failsafe++;
  
  if (failsafe >= 5000)
  {
    S0SPCCR = 57;
    S0SPCR  = 0x38;
  }
  return S0SPDR;
}
#endif

void
writeToDisp(unsigned short data)
{
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)
  SET_RS;
  SET_WRITE;
  ACTIVATE_CS;
  sendSpi(data >> 8);
  sendSpi(data & 0xff);
  DEACTIVATE_CS;  
#elif (QVGA_LCD_IF == PARALLEL_16_LCD_IF)

  LCD_DATA_16 = data;

#elif (QVGA_LCD_IF == PARALLEL_8_LCD_IF)

  LCD_DATA_8 = (data >> 8) & 0xff;
  LCD_DATA_8 = data & 0xff;

#endif

}

unsigned short
readFromDisp(void)
{
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)
  unsigned short msb, lsb;

  SET_RS;
  SET_READ;
  ACTIVATE_CS;
  sendSpi(0);
  sendSpi(0);
  DEACTIVATE_CS;  
  ACTIVATE_CS;
  msb = sendSpi(0);
  lsb = sendSpi(0);
  DEACTIVATE_CS;
  return (msb << 8) | lsb;
#elif (QVGA_LCD_IF == PARALLEL_16_LCD_IF)
  volatile unsigned short value;

  value = LCD_DATA_16;  //dummy data
  value = LCD_DATA_16;
  return value;

#elif (QVGA_LCD_IF == PARALLEL_8_LCD_IF)

  volatile unsigned short value;

  value = LCD_DATA_8;  //dummy data
  value = LCD_DATA_8;  //dummy data ???
  value  = (unsigned short)LCD_DATA_8 << 8;
  value |= (unsigned short)LCD_DATA_8;
  return value;

#endif
}

void
writeToReg(unsigned char addr, unsigned short data)
{
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)
  RESET_RS;
  SET_WRITE;
  ACTIVATE_CS;
  sendSpi(data >> 8);
  sendSpi(data & 0xff);
  DEACTIVATE_CS;  

#elif (QVGA_LCD_IF == PARALLEL_16_LCD_IF)

  LCD_COMMAND_16 = addr;
  LCD_DATA_16    = data;
  LCD_COMMAND_16 = 0x22;  //restore index register to GRAM

#elif (QVGA_LCD_IF == PARALLEL_8_LCD_IF)

  LCD_COMMAND_8 = 0;
  LCD_COMMAND_8 = addr;

  LCD_DATA_8 = (data >> 8) & 0xff;
  LCD_DATA_8 = data & 0xff;

  LCD_COMMAND_8 = 0;
  LCD_COMMAND_8 = 0x22;  //restore index register to GRAM

#endif
}

unsigned short
readFromReg(unsigned char addr)
{
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)
  unsigned char reg;

  RESET_RS;
  SET_WRITE;
  ACTIVATE_CS;
  sendSpi(addr);
  SET_READ;
  reg = sendSpi(0);
  DEACTIVATE_CS;
  return reg;
#elif (QVGA_LCD_IF == PARALLEL_16_LCD_IF)

  LCD_COMMAND_16 = addr;
  return LCD_DATA_16;

#elif (QVGA_LCD_IF == PARALLEL_8_LCD_IF)
unsigned short readVal;

  LCD_COMMAND_8 = 0;
  LCD_COMMAND_8 = addr & 0xff;

  readVal  = ((unsigned short)LCD_DATA_8 << 8) & 0xff00;
  readVal |= (unsigned short)LCD_DATA_8 & 0x00ff;

  return readVal;

#endif
}


/******************************************************************************
** Function name:		lcd_init
**
** Descriptions:		Read LCD controller R49 and R50 to make sure
**						the controller has been initialized correctly,
**						then, turn on system, set display, adjust GAMMA,
**						finally, display.		
**
** parameters:			None
** Returned value:		TRUE or FALSE
** 
******************************************************************************/
unsigned int lcd_init(void)
{
  /****************************************************************
   * Check if contact with Lcd controller (read register R00 = 0x8989)
   ****************************************************************/
  if (readFromReg(0) != 0x8989)
    return( FALSE );

  /****************************************************************
   * Initialize Lcd controller (long sequence) 
   ****************************************************************/
  writeToReg(0x07, 0x0021);
  writeToReg(0x00, 0x0001);
  writeToReg(0x07, 0x0723);
  writeToReg(0x10, 0x0000);
  mdelay(200);
  writeToReg(0x07, 0x0033);
  writeToReg(0x11, 0x6830);
  writeToReg(0x02, 0x0600);
  writeToReg(0x0f, 0x0000);
  writeToReg(0x01, 0x2b3f);
  writeToReg(0x0b, 0x5308);
  writeToReg(0x25, 0xa000);
  
  return( TRUE );
}

