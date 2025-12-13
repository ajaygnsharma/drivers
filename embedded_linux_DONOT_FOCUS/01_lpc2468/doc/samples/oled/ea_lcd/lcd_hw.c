
/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2009 Embedded Artists AB
 *
 * Description:
 *    Hardware interface to the display
 *
 *****************************************************************************/

 /******************************************************************************
 * Includes
 *****************************************************************************/
 
#include "lpc246x.h"
#include "lcd_hw.h"
#include <general.h>
#include <config.h>
#include <printf_P.h>

/******************************************************************************
 * Defines, macros, and typedefs
 *****************************************************************************/

 #define EA_DISP_QVGA_OLED_V1_ID 0x63D6
 
/* chip select controlled thorugh P0.6 */
#define ACTIVATE_CS   IOCLR0 = 0x40
#define DEACTIVATE_CS IOSET0 = 0x40
 
/******************************************************************************
 * Local variables
 *****************************************************************************/

/******************************************************************************
 * Local functions
 *****************************************************************************/
 
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)

/*****************************************************************************
 *
 * Description:
 *    SPI communication
 *
 * Params:
 *    [in] indate - data to send over SPI.
 *
 * Return:
 *    Read value from SPI
 *
 ****************************************************************************/
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
    S0SPCCR = 8;
    S0SPCR  = 0x38;
  }
  return S0SPDR;
}
#endif
 
/******************************************************************************
 * Public functions
 *****************************************************************************/ 

/*****************************************************************************
 *
 * Description:
 *    Delay a numberof milliseconds
 *
 * Params:
 *    [in] delayInMs - number of milliseconds to delay.
 *
 ****************************************************************************/
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

/*****************************************************************************
 *
 * Description:
 *    Initialize the HW for display controller, EMC, PINSEL, SPI.
 *
 ****************************************************************************/
void lcd_hw_init(void)
{


#if (QVGA_LCD_IF == PARALLEL_16_LCD_IF) || (QVGA_LCD_IF == PARALLEL_8_LCD_IF)
  unsigned int regVal;

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

  S0SPCCR = 8;    
  S0SPCR  = 0x038;  
#endif

  /****************************************************************
   * Setup control of backlight
   ****************************************************************/
  FIO3DIR = BACKLIGHT_PIN;
  FIO3CLR = BACKLIGHT_PIN;

  return;
}

/*****************************************************************************
 *
 * Description:
 *    Write display data
 *
 * Params:
 *    [in] data - the data to write to the display.
 *
 ****************************************************************************/
void
writeToDisp(unsigned short data)
{
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)

  ACTIVATE_CS;
  sendSpi(0x72);  
  sendSpi(data >> 8);
  sendSpi(data & 0xff);
  DEACTIVATE_CS;  
  
#elif (QVGA_LCD_IF == PARALLEL_16_LCD_IF)

  LCD_DATA_16 = data;

#elif (QVGA_LCD_IF == PARALLEL_8_LCD_IF)

  LCD_DATA_8 = (data >> 8)& 0xff;
  LCD_DATA_8 = data & 0xff;

#endif
}

/*****************************************************************************
 *
 * Description:
 *    Read data from the display
 *
 * Return:
 *    Read data.
 *
 ****************************************************************************/
unsigned short
readFromDisp(void)
{
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)
  unsigned short msb, lsb;
  
  ACTIVATE_CS;
  sendSpi(0x73);
  msb = sendSpi(0x0);  
  lsb = sendSpi(0x0); 
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

  value  = (unsigned short)LCD_DATA_8 << 8;
  value |= (unsigned short)LCD_DATA_8;
  return value;

#endif
}

/*****************************************************************************
 *
 * Description:
 *    Enable a register index. Any value is written using writeToDisp when
 *    the register has been indexed.
 *
 * Return:
 *    Read data.
 *
 ****************************************************************************/
void
writeToReg(unsigned short data)
{
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)

  ACTIVATE_CS;
  sendSpi(0x70);
  sendSpi(data >> 8);
  sendSpi(data & 0xff);
  DEACTIVATE_CS;  
  
#elif (QVGA_LCD_IF == PARALLEL_16_LCD_IF)

  LCD_COMMAND_16 = data;  
  
#elif (QVGA_LCD_IF == PARALLEL_8_LCD_IF)

  LCD_COMMAND_8 = data & 0xff; 

#endif
}

/*****************************************************************************
 *
 * Description:
 *    Read data from a register
 *
 * Return:
 *    Read data.
 *
 ****************************************************************************/
unsigned short
readFromReg(unsigned char addr)
{
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)

  unsigned short msb, lsb;
  ACTIVATE_CS;  
  sendSpi(0x70);
  sendSpi(0xf >> 8);
  sendSpi(0xf & 0xff);    
  DEACTIVATE_CS;  
  
  ACTIVATE_CS;    
  sendSpi(0x73);
  msb = sendSpi(0x0);  
  lsb = sendSpi(0x0); 
  DEACTIVATE_CS;
  
  return (msb << 8) | lsb;
  
#elif (QVGA_LCD_IF == PARALLEL_16_LCD_IF)

  LCD_COMMAND_16 = (unsigned short)addr << 8;
  return (unsigned char)LCD_COMMAND_16;

#elif (QVGA_LCD_IF == PARALLEL_8_LCD_IF)

  LCD_COMMAND_8 = addr;
  return LCD_COMMAND_8; 

#endif
}

/*****************************************************************************
 *
 * Description:
 *    Initialize the display
 
 * Return:
 *    TRUE if initialization was successful; otherwise FALSE.
 *
 ****************************************************************************/
unsigned int lcd_init(void)
{
  unsigned int result;

#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)

  result = readFromReg(0x0f);;
  if (result != EA_DISP_QVGA_OLED_V1_ID)
  {
    return( FALSE );
  }
    
  writeToReg(0x03);
  writeToDisp(0x0130);

#elif (QVGA_LCD_IF == PARALLEL_8_LCD_IF)

  //set 8-bit mode
  writeToReg(0x24);

  //set cpu interface
  writeToReg(0x02);
  writeToDisp(0x0000);

  //set 65k colors  (two 8-bit transbers, msb first, then lsb)
  writeToReg(0x03);
  writeToDisp(0x0130);

  writeToReg(0x0f);
  result = ((unsigned short)LCD_DATA_8 << 8) | (unsigned short)LCD_DATA_8;
  if (result != EA_DISP_QVGA_OLED_V1_ID)
  {
    return( FALSE );
  }

#elif (QVGA_LCD_IF == PARALLEL_16_LCD_IF)

  //set 16-bit mode
  writeToReg(0x23);

  //set cpu interface
  writeToReg(0x02);
  writeToDisp(0x0000);

  //set 65k colors, one 16-bit transfer per pixel
  writeToReg(0x03);
  writeToDisp(0x0130);

  writeToReg(0x0f);
  result = LCD_DATA_16;
  if (result != EA_DISP_QVGA_OLED_V1_ID)
  {
    return( FALSE );
  }

#endif

  //set standby off
  writeToReg(0x10);
  writeToDisp(0x0000);

  //100ms delay
  mdelay(100);

  //enable ARVDD & ARVSS
  FIO3SET = BACKLIGHT_PIN;

  //32 ms delay
  mdelay(32);
  
  //set display on
  writeToReg(0x05);
  writeToDisp(0x0001);

  //enable image data transfer
  writeToReg(0x22);

  return( TRUE );

}

