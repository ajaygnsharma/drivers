/******************************************************************************
 *
 * Copyright:
 *    (C) 2007 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates how to calibrate a
 *    touch screen.
 *
 *****************************************************************************/
#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc246x.h>
#include "lcd_hw.h"
#include "lcd_grph.h"
#include "font5x7.h"
#include "Calibrate.h"

#define PROC1_STACK_SIZE 1024
#define PROC2_STACK_SIZE 1024
#define INIT_STACK_SIZE  1024

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 proc2Stack[PROC2_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];

static void proc1(void* arg);
static void proc2(void* arg);
static void initProc(void* arg);

static tCntSem touchSem;
typedef struct
{
  tU16 xPos;
  tU16 yPos;
  tU16 pressure;
} tTouch;
tTouch regTouch;

#define CS_PIN        0x00010000        //P0.16
#define ACTIVATE_CS   IOCLR0 = CS_PIN
#define DEACTIVATE_CS IOSET0 = CS_PIN

tU16 const COLORS_TAB[16] = {BLACK,
                             NAVY,
                             DARK_GREEN,
                             DARK_CYAN,
                             MAROON,
                             PURPLE,
                             OLIVE,
                             LIGHT_GRAY,
                             DARK_GRAY,
                             BLUE,
                             GREEN,
                             CYAN,
                             RED,
                             MAGENTA,
                             YELLOW,
                             WHITE
                            };

MATRIX  matrix;
POINT   display;


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
 *    The first function to execute 
 *
 ****************************************************************************/
void
setBacklight(unsigned char percent)
{
  PWM1MR5 = ((unsigned long)0x3000 * (unsigned long)(100-percent)) / (unsigned long)100;
  PWM1LER = 0x20;
}


/*****************************************************************************
 *
 * Description:
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
    S0SPCCR = 57;
    S0SPCR  = 0x38;
  }
  return S0SPDR;
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
proc1(void* arg)
{
  osSleep(50);
  while ( 1 )
  {
    tU32 Xresult;
    tU32 Yresult;
    tU32 Z1result;
    tU32 Z2result;
    tU32 pressure;

    //Read X value
    ACTIVATE_CS;
    sendSpi(0x83 | 0x50 | 0x00);
    Xresult  = ((tU32)sendSpi(0x00)) << 8;
    Xresult |= (tU32)sendSpi(0x00);
    Xresult >>= 3;
    DEACTIVATE_CS;

    //Read Y value
    ACTIVATE_CS;
    sendSpi(0x83 | 0x10 | 0x00);
    Yresult  = ((tU32)sendSpi(0x00)) << 8;
    Yresult |= (tU32)sendSpi(0x00);
    Yresult >>= 3;
    DEACTIVATE_CS;

    //Read Z1 value
    ACTIVATE_CS;
    sendSpi(0x83 | 0x30 | 0x00);
    Z1result  = ((tU32)sendSpi(0x00)) << 8;
    Z1result |= (tU32)sendSpi(0x00);
    Z1result >>= 3;
    DEACTIVATE_CS;

    //Read Z2 value
    ACTIVATE_CS;
    sendSpi(0x83 | 0x40 | 0x00);
    Z2result  = ((tU32)sendSpi(0x00)) << 8;
    Z2result |= (tU32)sendSpi(0x00);
    Z2result >>= 3;
    DEACTIVATE_CS;

    pressure = (Xresult*Z2result - Z1result) / Z1result;

//    printf("\nresult=%d         %d    %d    %d    %d", pressure, (Xresult*Z2result) / Z1result, Xresult, Z1result, Z2result);
    
    if ((pressure != 0) && (pressure < 40000))
    {
      tU8 error;

      regTouch.xPos     = Xresult;
      regTouch.yPos     = Yresult;
      regTouch.pressure = pressure;
      osSemGive(&touchSem, &error);
      if(error == OS_ERROR_NULL)
      {
        printf("osSemGive: null pointer supplied");
      }

      osSleep(10);
    }
    osSleep(5);
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
proc2(void* arg)
{
  tU8 error;
  static tU32 oldXresult = 0;
  static tU32 oldYresult = 0;
  static tU32 oldRadius = 5;
  POINT screenSample[3];
  POINT displaySample[3];

  lcd_fillScreen(WHITE);
  lcd_fontColor(BLACK, WHITE);
  lcd_putString(70, 145, (unsigned char *)"Calibration;");
  lcd_putString(70, 155, (unsigned char *)"touch the screen...");

  while(1)
  {
    if (0 == osSemTryTake(&touchSem, &error))
    {
#if 0
      tU8 a[15];
      a[0] = 'X';
      a[1] = '=';
      a[2] = (regTouch.xPos / 1000) + '0';
      a[3] = ((regTouch.xPos / 100) % 10) + '0';
      a[4] = ((regTouch.xPos / 10) % 10) + '0';
      a[5] = ((regTouch.xPos / 1) % 10) + '0';
      a[6] = ' ';
      a[7] = ' ';
      a[8] = 'Y';
      a[9] = '=';
      a[10] = (regTouch.yPos / 1000) + '0';
      a[11] = ((regTouch.yPos / 100) % 10) + '0';
      a[12] = ((regTouch.yPos / 10) % 10) + '0';
      a[13] = ((regTouch.yPos / 1) % 10) + '0';
      a[14] = 0;
      lcd_putString(70, 100, a);
#endif

      if ((regTouch.xPos > 3000) && (regTouch.yPos < 500))
      {
        lcd_drawRect(10, 10, 20, 20, WHITE);
        screenSample[0].x  = regTouch.xPos;
        screenSample[0].y  = regTouch.yPos;
        displaySample[0].x = 15;
        displaySample[0].y = 15;
        break;
      }
    }
	  osSleep(20);
    lcd_drawRect(10, 10, 20, 20, WHITE);
	  osSleep(20);
    lcd_drawRect(10, 10, 20, 20, DARK_GREEN);
  }
  
  while(0 == osSemTryTake(&touchSem, &error))
    ;
  while(1)
  {
    if (0 == osSemTryTake(&touchSem, &error))
    {
      if ((regTouch.xPos < 1000) && (regTouch.yPos < 500))
      {
        lcd_drawRect(220, 10, 230, 20, WHITE);
        screenSample[1].x  = regTouch.xPos;
        screenSample[1].y  = regTouch.yPos;
        displaySample[1].x = 225;
        displaySample[1].y = 15;
        break;
      }
    }
	  osSleep(20);
    lcd_drawRect(220, 10, 230, 20, WHITE);
	  osSleep(20);
    lcd_drawRect(220, 10, 230, 20, DARK_GREEN);
  }

#if 0
  while(0 == osSemTryTake(&touchSem, &error))
    ;
  while(1)
  {
    if (0 == osSemTryTake(&touchSem, &error))
    {
      if ((regTouch.xPos < 500) && (regTouch.yPos > 3000))
      {
        lcd_drawRect(220, 300, 230, 310, WHITE);
        screenSample[2].x  = regTouch.xPos;
        screenSample[2].y  = regTouch.yPos;
        displaySample[2].x = 225;
        displaySample[2].y = 305;
        break;
      }
    }
	  osSleep(20);
    lcd_drawRect(220, 300, 230, 310, WHITE);
	  osSleep(20);
    lcd_drawRect(220, 300, 230, 310, DARK_GREEN);
  }
#endif

  while(0 == osSemTryTake(&touchSem, &error))
    ;
  while(1)
  {
    if (0 == osSemTryTake(&touchSem, &error))
    {
      if ((regTouch.xPos > 1750) && (regTouch.xPos < 2250) && (regTouch.yPos > 3000))
      {
        lcd_drawRect(115, 300, 125, 310, WHITE);
        screenSample[2].x  = regTouch.xPos;
        screenSample[2].y  = regTouch.yPos;
        displaySample[2].x = 120;
        displaySample[2].y = 305;
        break;
      }
    }
	  osSleep(20);
    lcd_drawRect(115, 300, 125, 310, WHITE);
	  osSleep(20);
    lcd_drawRect(115, 300, 125, 310, DARK_GREEN);
  }
  
  setCalibrationMatrix( &displaySample[0], &screenSample[0], &matrix ) ;


  //now the touch screen is calibrated
  lcd_fillScreen(WHITE);
  lcd_fontColor(RED, WHITE);
  lcd_putString(70, 145, (unsigned char *)"Calibrated!");
  lcd_putString(70, 155, (unsigned char *)"touch the screen...");
  while(1)
  {
    static tU8 erase = FALSE;
    POINT displayPoint, screenSample;

    if (0 == osSemTryTake(&touchSem, &error))
    {
      tU32 radius;
  
      if (regTouch.pressure > 17000) radius = 4;
      else if (regTouch.pressure > 16000) radius = 5;
      else if (regTouch.pressure > 15000) radius = 6;
      else if (regTouch.pressure > 14000) radius = 7;
      else if (regTouch.pressure > 13000) radius = 8;
      else if (regTouch.pressure > 12000) radius = 9;
      else if (regTouch.pressure > 11000) radius = 10;
      else if (regTouch.pressure > 10000) radius = 11;
      else if (regTouch.pressure > 9000) radius = 12;
      else if (regTouch.pressure > 8000) radius = 13;
      else if (regTouch.pressure > 7000) radius = 14;
      else radius = 15;
  
      screenSample.x = regTouch.xPos;
      screenSample.y = regTouch.yPos;
      getDisplayPoint( &displayPoint, &screenSample, &matrix ) ;

      lcd_circle(oldXresult, oldYresult, oldRadius, WHITE);
      lcd_circle(displayPoint.x, displayPoint.y, radius, BLACK);
      oldXresult = displayPoint.x;
      oldYresult = displayPoint.y;
      oldRadius = radius;
      osSleep(2);
      erase = TRUE;
    }
    else if(erase == TRUE)
    {
      lcd_circle(oldXresult, oldYresult, oldRadius, WHITE);
      erase = FALSE;
    }

    lcd_putString(70, 145, (unsigned char *)"Calibrated!");
    lcd_putString(70, 155, (unsigned char *)"touch the screen...");
    osSleep(2);
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
  printf("\n***************************************************");
  printf("\n*                                                 *");
  printf("\n* Sample program for LPC2468 OEM Board...         *");
  printf("\n* (C) Embedded Artists 2006-2007                  *");
  printf("\n*                                                 *");
  printf("\n* QVGA LCD example code                           *");
#if (QVGA_LCD_IF == SERIAL_8_LCD_IF)
  printf("\n*  8-bit serial interface to QVGA LCD             *");
#elif (QVGA_LCD_IF == PARALLEL_16_LCD_IF)
  printf("\n*  16-bit parallel interface to QVGA LCD          *");
#elif (QVGA_LCD_IF == PARALLEL_8_LCD_IF)
  printf("\n*  8-bit parallel interface to QVGA LCD           *");
#endif
  printf("\n*                                                 *");
  printf("\n***************************************************\n");

  lcd_hw_init();

  if ( lcd_init() != TRUE )
  {
    printf("\nERROR: Failed to identify LCD!");
  	while( 1 );		/* Display fatal error */
  }

  //set P3.28 as PWM output (PWM1.5, third alternative function)
  PINSEL7 |= 0x03000000;
  PWM1PR  = 0x00;     //no prescaling
  PWM1MCR = 0x02;     //reset counter if MR0 match
  PWM1MR0 = 0x3000;   //period time equal about 5 ms
  PWM1MR5 = 0x0000;
  PWM1LER = 0x21;     //latch MR0 and MR5
  PWM1PCR = 0x2000;   //enable PWMENA5
  PWM1TCR = 0x09;     //enable counter and PWM

  //init SPI #0
  PINSEL0 |= 0xc0000000;
  PINSEL1 |= 0x0000003c;
  IODIR0  |= CS_PIN;
  IOSET0   = CS_PIN;
  S0SPCCR = 57;    
  S0SPCR  = 0x38;

  osSemInit(&touchSem, 0);

  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid, 2, NULL, &error);
  osStartProcess(pid, &error);
  osCreateProcess(proc2, proc2Stack, PROC2_STACK_SIZE, &pid, 3, NULL, &error);
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
