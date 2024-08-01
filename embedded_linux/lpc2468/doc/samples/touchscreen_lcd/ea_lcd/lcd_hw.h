/*
    This module implements a linux character device driver for the XXX chip.
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
*/

#ifndef _LCD_HW_
#define _LCD_HW_

/******************************************************************************
 * Interface configuration
 *****************************************************************************/
#define PARALLEL_16_LCD_IF    0
#define PARALLEL_8_LCD_IF     1
#define SERIAL_8_LCD_IF       2

#define QVGA_LCD_IF           PARALLEL_16_LCD_IF


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
#define LCD_COMMAND_16  (*((volatile unsigned short *) 0x82000000)) 
#define LCD_DATA_16     (*((volatile unsigned short *) 0x82000002))

#define LCD_COMMAND_8   (*((volatile unsigned char *) 0x82000000)) 
#define LCD_DATA_8      (*((volatile unsigned char *) 0x82000002))

#define BACKLIGHT_PIN 0x10000000  //P3.28

extern void mdelay( unsigned int delay );
extern void lcd_hw_init( void );
extern unsigned int lcd_init(void);

extern void           writeToDisp(unsigned short data);
extern unsigned short readFromDisp(void);
extern void           writeToReg(unsigned char addr, unsigned short data);
extern unsigned short readFromReg(unsigned char addr);

#endif /* _LCD_HW_ */
