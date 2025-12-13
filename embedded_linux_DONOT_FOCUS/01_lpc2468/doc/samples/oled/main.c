/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2007 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates how to create processes.
 *
 *****************************************************************************/

#include <general.h>
#include <printf_P.h>
#include <ea_init.h>
#include <lpc246x.h>
#include "lcd_hw.h"
#include "lcd_grph.h"
#include "font5x7.h"
#include <stdlib.h>

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

/*****************************************************************************
 *
 * Description:
 *    
 *
 ****************************************************************************/
static tU16
random(tU16 seed)
{
  tU16 temp;
  
  temp = rand();
  temp = temp % seed;
  return temp;
}

/*****************************************************************************
 *
 * Description:
 *    
 *
 ****************************************************************************/
void
CLineDemo(void)
{
  tU32 x0, y0;
  tU32 x1, y1;
  tU16 color;   
  tU32 no;
  const tU32 CENTER_X	= 120;
  const tU32 CENTER_Y	= 160;
  
  for(no=0; no<200; no++)
  {
    x0 = random(240);
    y0 = random(320);
    color = random(15)+1;
    
    x1 = 2*CENTER_X - x0;
    y1 = 2*CENTER_Y - y0;
    
    lcd_line(x0, y0, x1, y1, COLORS_TAB[color]);
    mdelay(10);
  }
}


/*****************************************************************************
 *
 * Description:
 *    
 *
 ****************************************************************************/
void
CRectangleDemo(void)
{
  tU32 x0, y0;
  tU32 x1, y1;
  tU16 color;
  tU32 no;
  
  for(no=0; no<100; no++)
  {
    x0 = random(240);
    y0 = random(320);
    color = random(15)+1;
    
    x1 = x0 + random(150);
    y1 = y0 + random(100);
    
    lcd_fillRect(x0, y0, x1, y1, COLORS_TAB[color]);
    mdelay(20);
  }
}


/*****************************************************************************
 *
 * Description:
 *    
 *
 ****************************************************************************/
void
CCircleDemo(void)
{
  tU32 x0, y0;
  tU8  radius;
  tU16 color;
  tU32 no;
  
  for(no=0; no<200; no++)
  {
    x0 = random(240);
    y0 = random(320);
    color = random(16);
    radius = random(50);
    
    lcd_circle(x0, y0, radius, COLORS_TAB[color]);
    mdelay(10);
  }
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
  unsigned char i;
  
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
  

  srand(3721);
  while ( 1 )
  {
	  lcd_fillScreen(RED);
	  mdelay( 300);
	  lcd_putString(0, 0, (unsigned char *)"1234567891011121314151617181920\n");
	  mdelay( 300);
  

	  lcd_fillScreen(GREEN);
	  mdelay( 300 );
	  lcd_putString(0, 40, (unsigned char *)"1234567891011121314151617181920\n");
	  mdelay( 300 );
	
	  lcd_fillScreen(YELLOW);
	  mdelay( 300 );
	  lcd_putString(0, 80, (unsigned char *)"1234567891011121314151617181920\n");
	  mdelay( 300 );
	
	  lcd_fillScreen(BLUE);
	  mdelay( 300 );
	  lcd_putString(0, 120, (unsigned char *)"1234567891011121314151617181920\n");
	  mdelay( 300 );

    lcd_fillScreen(BLACK);
    CLineDemo();
    lcd_fillScreen(BLACK);
        
    CRectangleDemo();
    lcd_fillScreen(BLACK);
        
    CCircleDemo();
    lcd_fillScreen(BLACK);
  }
  return 0;
}


