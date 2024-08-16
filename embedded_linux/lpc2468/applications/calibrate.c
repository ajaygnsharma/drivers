/*
    This application is used to calibrate the touch screen. The application
    is based on the ts_calibrate application by Charles Flynn.

    Copyright (C) 2008  Embedded Artists AB (www.embeddedartists.com)

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>

#include "images.h"
#include "font5x7.h"


#define PAGE_SIZE 4096
#define TS_PATH		"/dev/ts0"
#define TSRAW_PATH	"/dev/tsraw0"
#define FB_PATH		"/dev/fb0"
#define EEPROM_PATH 	"/sys/bus/i2c/devices/0-0050/data0"

#define IOC_H3600_TS_MAGIC  'f'
#define TS_GET_CAL	_IOR(IOC_H3600_TS_MAGIC, 10, struct ts_calibration)
#define TS_SET_CAL	_IOW(IOC_H3600_TS_MAGIC, 11, struct ts_calibration)

#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 320

#define SCREEN_WIDTH		DISPLAY_WIDTH
#define SCREEN_HEIGHT		DISPLAY_HEIGHT

#define BUTTON_WIDTH 36
#define CLOSE_X0 (DISPLAY_WIDTH/2)-(BUTTON_WIDTH/2)
#define CLOSE_Y0 250
#define CLOSE_X1 CLOSE_X0+BUTTON_WIDTH
#define CLOSE_Y1 CLOSE_Y0+22

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF

#define EEPROM_HEAD_VALUE 0xEAAE
#define EEPROM_FOOT_VALUE 0xAEEA
#define EEPROM_OFFSET 0x7FB0

struct ts_calibration {
	int xscale;
	int xtrans;
	int yscale;
	int ytrans;
	int xyswap;
};

struct ts_event {
	short pressure;
	short x;
	short y;
	short millisecs;
};

struct ts_point {
  ushort x;
  ushort y;
};

struct eeprom_data {
	int head;
	struct ts_calibration calib;
	int foot;
};

static int  xa[5], ya[5];
static unsigned char const  
	font_mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};


static struct ts_calibration new_cal;
static void 
saveCalibration(struct ts_calibration* calib)
{
	FILE* fd;
	struct eeprom_data data;

	fd = fopen(EEPROM_PATH, "r+b");
	if (fd == NULL) {
		printf("Failed to open %s\n", EEPROM_PATH);
		return ;
	}

	data.head = EEPROM_HEAD_VALUE;
	data.calib = *calib;
	data.foot = EEPROM_FOOT_VALUE;

	fseek (fd, EEPROM_OFFSET, SEEK_SET);

	// write data
	if (fwrite(&data, 1, sizeof(struct eeprom_data), fd) < 0)
		printf("Failed to store calibration data\n");

	fclose(fd);
}

static int
calibrateFromStorage(void)
{
	FILE* fd;
	int tsfd;
	int ret = 0;
	struct eeprom_data data;

	fd = fopen(EEPROM_PATH, "r+b");
	if (fd == NULL) {
		printf("Failed to open %s\n", EEPROM_PATH);
		ret = -1;
		goto err;
	}

	fseek (fd, EEPROM_OFFSET, SEEK_SET);

	if (fread(&data, 1, sizeof(struct eeprom_data), fd) < 0) {
		printf("Failed to read %s\n", EEPROM_PATH);
		ret = -1;
		goto err_close;
	}
	
	if (data.head != EEPROM_HEAD_VALUE || data.foot != EEPROM_FOOT_VALUE) {
		printf("Invalid calibration data in EEPROM\n");
		ret = -1;
		goto err_close;
	}

	tsfd = open(TSRAW_PATH, O_RDONLY);
	if (tsfd < 0) {
		printf("Couldn't open %s\n", TSRAW_PATH);
		ret = -1;
		goto err_close;
	}

	if (ioctl(tsfd, TS_SET_CAL, (void *)&data.calib) != 0) {
		perror("TS_SET_CALIBRATION ioctl fail\n");
		ret = -1;
	}

	close(tsfd);

err_close:
	fclose(fd);
err:

	return ret;
}

static void 
lcd_point(unsigned char* frame, 
          unsigned short x, 
          unsigned short y, 
          unsigned short color)
{
  if( x >= DISPLAY_WIDTH )  
  {
    return;
  }
  
  if(y >= DISPLAY_HEIGHT)
  {
    return;
  }

  *((unsigned short *)(frame + x*2 + 240*2*y)) = color;


  return;
}

static void 
hLine(unsigned char* frame, 
      unsigned short x0, 
      unsigned short y0, 
      unsigned short x1, 
      unsigned short color) 
{
  unsigned short bak;

  if (x0 > x1) 						
  {
    bak = x1;
    x1 = x0;
    x0 = bak;
  }
  lcd_point(frame, x0, y0, color);
  x0++;
   
  while(x1 >= x0)
  {
    lcd_point(frame, x0, y0, color);
    x0++;
  }
  return;
}

static void 
vLine(unsigned char* frame, 
      unsigned short x0, 
      unsigned short y0, 
      unsigned short y1, 
      unsigned short color)
{
  unsigned short bak;

  if(y0 > y1) 						
  {
    bak = y1;
    y1 = y0;
    y0 = bak;
  }

  while(y1 >= y0)
  {
    lcd_point(frame, x0, y0, color);
    y0++;
  }
  return;
}

static int 
lcd_putChar(unsigned char* frame, int x, int y, unsigned char ch)
{  
  unsigned char data     = 0;
  int i        = 0;
  int j        = 0;
  unsigned short color = COLOR_BLACK;

  if(x >= (DISPLAY_WIDTH - 8) ) 
  {
    return 0;
  }

  if(y >= (DISPLAY_HEIGHT - 8) )
  {
    return 0;
  }

  if( (ch < 0x20) || (ch > 0x7f) )
  {
    ch = 0x20;
  }
   
  ch -= 0x20; 

  for(i=0; i<8; i++)
  {
    data = font5x7[ch][i];
      
    for(j=0; j<6; j++)
    {
      if( (data&font_mask[j])==0 )
      {
        color = COLOR_WHITE;
      }
      else
      {
        color = COLOR_BLACK;
      }

      lcd_point(frame, x, y, color);       
      x++;
    }
      
    y++;
    x -= 6;
  }

  return 1;
}

static void 
lcd_putString(unsigned char* frame, int x, int y, char *pStr)
{
  while(1)
  {  
    if( (*pStr)=='\0' )
    {
      break;
    }

    if( lcd_putChar(frame, x, y, *pStr++) == 0 )
    {
      break;
    }

    x += 6;
  }
}



static void 
lcd_drawRect(unsigned char* frame, 
             unsigned short x0, 
             unsigned short y0, 
             unsigned short x1, 
             unsigned short y1, 
             unsigned short color)
{  
  hLine(frame, x0, y0, x1, color);
  hLine(frame, x0, y1, x1, color);
  vLine(frame, x0, y0, y1, color);
  vLine(frame, x1, y0, y1, color);
  return;
}

static void lcd_fillRect(unsigned char* frame, int x0, int y0, 
	int x1, int y1, unsigned short color)
{
  int i = 0;

  if(x0 > x1)
  {
    i  = x0;
    x0 = x1;
    x1 = i;
  }

  if(y0 > y1)
  {  
    i  = y0;
    y0 = y1;
    y1 = i;
  }
   
  if(y0 == y1) 
  {  
    hLine(frame, x0, y0, x1, color);
    return;
  }

  if(x0 == x1) 
  {  
    vLine(frame, x0, y0, y1, color);
    return;
  }

  while(y0 <= y1)						
  {  
    hLine(frame, x0, y0, x1, color);
    y0++;
  }
}


static void 
lcd_fillScreen(unsigned char* frame, 
               unsigned short color)
{
	unsigned short i = 0;
	unsigned short j = 0;

	for(i=0; i < DISPLAY_HEIGHT; i++)
	{
		for(j=0; j<DISPLAY_WIDTH; j++)
		{
			lcd_point(frame, j, i, color);
		}
	}
}

static void 
drawImage(unsigned char* frame, int x, int y, unsigned short* image)
{
	int w = image[0];
	int h = image[1];
	int i = 0;
	image += 4;

	frame += (x*2 + y*2*DISPLAY_WIDTH);

	for (i = 0; i < h; i++) {
		memcpy(frame, image, w*2);
		frame += DISPLAY_WIDTH*2;
		image += w;

	}
	
}

static void 
drawBackground(unsigned char* frame)
{
	lcd_fillScreen(frame, COLOR_WHITE);

	drawImage(frame, 50, 50, (unsigned short *)_logo);

	lcd_putString(frame, 45, 190, "Touch square to calibrate");
}

static int oldX = -1;
static int oldY = -1;

static void 
drawTouchSquare(unsigned char* frame, int x, int y)
{
	/* remove old square */
	if (oldX >= 0) {
		lcd_drawRect(frame,
			oldX-3,
			oldY-3,
			oldX+3,
			oldY+3,
			COLOR_WHITE); 	
	}

	lcd_drawRect(frame,
		x-3,
		y-3,
		x+3,
		y+3,
		COLOR_BLACK); 

	oldX = x;
	oldY = y;
}

static int 
do_calibration(int fd, unsigned char* frame)
{
  int k, xsum, ysum, cnt, rv;
  struct ts_event ts_ev;
  struct ts_point screenCalPos[5];
  int x,y;
  unsigned short xraw,yraw;


/* Screen Offsets  for test calibration */
#define OFF1 20
#define OFF2 50

  screenCalPos[0].x = OFF1;
  screenCalPos[0].y = OFF1;
  screenCalPos[1].x = SCREEN_WIDTH - OFF2 - 1;
  screenCalPos[1].y = OFF2;
  screenCalPos[2].x = OFF2;
  screenCalPos[2].y = SCREEN_HEIGHT - OFF2 - 1;
  screenCalPos[3].x = SCREEN_WIDTH - OFF1 - 1;
  screenCalPos[3].y = SCREEN_HEIGHT - OFF1 - 1;
  screenCalPos[4].x = SCREEN_WIDTH / 2;
  screenCalPos[4].y = SCREEN_HEIGHT / 2;

/*
    Enter here with ts events pertaining to screen coords screenCalPos[k]
*/

  drawBackground(frame);

#define MAX_CAL_POINTS	5

  for (k = 0; k < MAX_CAL_POINTS ; k++) 
  {
    /*
	for each screen position take the average of 5 points.
    */

    printf("\nTouch screen at x=%d y=%d\n",screenCalPos[k].x,screenCalPos[k].y);
    drawTouchSquare(frame, screenCalPos[k].x, screenCalPos[k].y);

    cnt = xsum = ysum = 0;
    
    /* now loop until we have 5 samples - TODO why 5 ? */
    while (1) {
      /*
	This read() call will block until the user taps the screen
	upon which it will proceed to gather samples until the pen
	is lifted. It takes the average of the 'cnt' samples.
	It will not accept any less than 5 samples.
      */

      if ((rv = read(fd, &ts_ev, sizeof(struct ts_event))) == -1) {
	usleep(100);
	continue;
      }
      else if (rv != sizeof(struct ts_event)) {
	usleep(100);
	continue;
      }

      /*
	Dont exit the while loop until we have 5 events
	AND the PEN is UP denoted by pressure=0
	*/

      if (ts_ev.pressure == 0  &&  cnt > 5)
      {
	break;
      }
      else
      {
	  /* accumulate the x coords */
	  xsum += ts_ev.x;
	  ysum += ts_ev.y;
	  /* increment the event counter */
	  cnt++;
      }
    } /* endwhile */

    /*
    	Enter here with - agregate of the x,y coords stored in xsum & ysum.
    */


    /* take the average of the x & y points */
    xa[k] = xsum/cnt;
    ya[k] = ysum/cnt;
    printf(" k=%d  AveX=%3d  AveY=%3d  (cnt=%3d)\n", k, xa[k], ya[k], cnt);
  }

  /* Enter here with the average x,y coords of the MAX_CAL_POINTS */

  /* get calibration parameters */
  {
    int *xp, *yp, x, y, dx, dy, flag;
    int xscale0, xscale1, xtrans0, xtrans1;
    int yscale0, yscale1, ytrans0, ytrans1;
    int xyswap;
    
    flag = 0;
  
    /* Calculate ABS(x1-x0) */
    dx = ((x = xa[1] - xa[0]) < 0 ? -x : x);
    dy = ((y = ya[1] - ya[0]) < 0 ? -y : y);

    /* CF

	(0,0) --------->Y
X	+-------+---------------+-
|	|	|		|
V	|	|xa0,ya0	|
	+-------O---------------+ --
	|	|		| dx or dy
	+-------+---------------O --
^	|	|		| xa1,ya1
|	|	|		|
Y	+-------+---------------+
	(0,0) -------->X

	Work out where the origin is, either at the TLH or BLH corners.
        Initialise xp such that it points to the array containing the X coords
        Initialise yp such that it points to the array containing the Y coords
    */
    if (dx < dy) {
      xyswap = 1;
      xp = ya, yp = xa;
    }
    else {
      xyswap = 0;
      xp = xa;
      yp = ya;
    }
    
    xraw=xp[4]; yraw=yp[4];	/* used for check later */

    /*

	We have MAX_CAL_POINTS sets of x,y coordinates.

	If we plot Xcal against Xraw we get two equations, each of a straight
		line. One for the Xcoord and the other for the Y coord.
		This line models the linear characteristics of the ts A/D
		converter.

	Xcal = m*Xraw + Cx
	Ycal = m*Yraw + Cy

	X/Ycal is the calibrated coord which is the pixel pos on the screen.
	X/Yraw is the uncalibrated X coord.
	m is the xscale ( gradient of line)
	Cx/y is the trans (constant)
	
	xscale
	  'm' can be got by calculating the gradient between two data points
	  Example Xscale0 = (Xcal1 - Xcal0 ) / (Xraw1 - Xraw0)

	trans = Xcal - mXraw
	  What is actualy done is to take the Ave of two measurements
	  Example  Xtrans0 = ( (Xcal0 - mXraw0) + (Xcal3 - mXraw3) ) / 2

	We repeat the above procedure to calculate 
	Yscale0 and Ytrans0 and repeat the whole lot again using two
	new data indexes 1 and 2 giving 4 new variables
	Xscale1, Xtrans1, Yscale1,Ytrans1, making a total of eight.

	The final calibration variables are the average of data ponts 
		0,3 and 1,2
	xscale = (Xscale0 + Xscale1) / 2
	yscale = (Yscale0 + Yscale1) / 2
	xtrans = (Xtrans0 + Xtrans1) /2
	ytrans = (Ytrans0 + Ytrans1) /2
	
    */
    xscale0 = ( (screenCalPos[0].x - screenCalPos[3].x) << 8 ) / 
      (((xp[0] - xp[3])));

    printf("Xc0=%d Xc3=%d Xr0=%d Xr3=%d xscale0=%d\n",
    	screenCalPos[0].x, screenCalPos[3].x, xp[0],xp[3],xscale0 );

    xtrans0 = ( (screenCalPos[0].x - ((xp[0]*xscale0) >> 8)) + 
		(screenCalPos[3].x - ((xp[3]*xscale0) >> 8)) ) / 2;
    yscale0 = ( (screenCalPos[0].y - screenCalPos[3].y) << 8 ) / 
      (((yp[0] - yp[3])));
    ytrans0 = ( (screenCalPos[0].y - ((yp[0]*yscale0) >> 8)) + 
		(screenCalPos[3].y - ((yp[3]*yscale0) >> 8)) ) / 2;

    xscale1 = ( (screenCalPos[1].x - screenCalPos[2].x) << 8 ) / 
      (((xp[1] - xp[2])));

    printf("Xc1=%d Xc2=%d Xr1=%d Xr2=%d xscale1=%d\n",
    	screenCalPos[1].x, screenCalPos[2].x, xp[1],xp[2],xscale1 );

    xtrans1 = ( (screenCalPos[1].x - ((xp[1]*xscale1) >> 8)) +
		(screenCalPos[2].x - ((xp[2]*xscale1) >> 8)) ) / 2;
    yscale1 = ( (screenCalPos[1].y - screenCalPos[2].y) << 8 ) / 
      (((yp[1] - yp[2])));
    ytrans1 = ( (screenCalPos[1].y - ((yp[1]*yscale1) >> 8)) 
		+(screenCalPos[2].y - ((yp[2]*yscale1) >> 8)) ) / 2;
    
    printf("xs0:%d, xs1:%d, xt0:%d, xt1:%d\n", xscale0, xscale1,
	      xtrans0, xtrans1);
    printf("ys0:%d, ys1:%d, yt0:%d, yt1:%d\n", yscale0, yscale1,
	      ytrans0, ytrans1);
    new_cal.xscale = (xscale0 + xscale1) / 2;
    printf("AveXscale=%d\n",new_cal.xscale);
    new_cal.xtrans = (xtrans0 + xtrans1) / 2;
    new_cal.yscale = (yscale0 + yscale1) / 2;
    new_cal.ytrans = (ytrans0 + ytrans1) / 2;
    new_cal.xyswap = xyswap;
  }

    printf("New calibration:\n");
    printf("  xscale:%5d, xtrans:%5d\n", new_cal.xscale,
	    new_cal.xtrans);
    printf("  yscale:%5d, xtrans:%5d\n", new_cal.yscale,
	    new_cal.ytrans);
    printf("xyswap:%s\n", (new_cal.xyswap == 0 ? "N" : "Y"));

  /* Now check it with centre coords*/
  printf("CHECK with Centre Coords (160,120): xraw=%d yraw=%d\n",xraw,yraw);
  x = ( ( new_cal.xscale * xraw ) >> 8 ) + new_cal.xtrans;
  y = ( ( new_cal.yscale * yraw ) >> 8 ) + new_cal.ytrans;
  printf("CHECK: x(centre)=%d y(centre)=%d\n",x,y);

  /* store this calibration in the device */
#if 1
  if (ioctl(fd, TS_SET_CAL, (void *)&new_cal) != 0)
  {
    perror("TS_SET_CALIBRATION ioctl fail\n");
    return -1;
  }
#endif


  return 0;
}

int main(int argc, char **argv)
{
	int 		fb_fd;
	int 		ts_fd;
	int 		frame_offset;
	unsigned char *	frame_map   = NULL;
	int 		frame_len   = 0;
	int 		err = 0;
	int 		len = 0;
	struct ts_event event;
	struct fb_fix_screeninfo finfo;

	if (argc >= 2 && strcmp(argv[1], "--eeprom") == 0) {		
		return calibrateFromStorage();
	}

	fb_fd = open(FB_PATH, O_RDWR);
	if (fb_fd < 0) {
		printf("Couldn't open %s\n", FB_PATH);
		err = 1;
		goto err_out;
	}

	ts_fd = open(TSRAW_PATH, O_RDONLY);
	if (ts_fd < 0) {
		printf("Couldn't open %s\n", TSRAW_PATH);
		err = 1;
		goto err_close_fb;
	}

	/* Get the type of video hardware */
	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0 ) {
		printf("Couldn't get fb hardware info\n");
		err = 1;
		goto err_close_ts;
	}

	frame_offset = (((long)finfo.smem_start) -
		(((long)finfo.smem_start)&~(PAGE_SIZE-1)));
	frame_len = finfo.smem_len + frame_offset;
	frame_map = (unsigned char *)mmap(NULL, frame_len, PROT_READ|PROT_WRITE,
		MAP_SHARED, fb_fd, 0);

	do_calibration(ts_fd, frame_map);

	saveCalibration(&new_cal);

	close(ts_fd);
	ts_fd = open(TS_PATH, O_RDONLY);
	if (ts_fd < 0) {
		printf("Couldn't open %s\n", TS_PATH);
		err = 1;
		goto err_close_fb;
	}

	lcd_fillRect(frame_map, 10, 155, DISPLAY_WIDTH-10, DISPLAY_HEIGHT, COLOR_WHITE);
	lcd_putString(frame_map, 45, 190, "Draw to test calibration");

	lcd_drawRect(frame_map, CLOSE_X0, CLOSE_Y0, CLOSE_X1, CLOSE_Y1, COLOR_BLACK);
	lcd_putString(frame_map, CLOSE_X0+7, CLOSE_Y0+8, "Quit");

	while(1) {

		len = read(ts_fd, &event, sizeof(struct ts_event));

		if (len <= 0) {
			printf("Failed to read from %s\n", TS_PATH);
			err = 1;
			goto err_close_ts;
		}

		if (event.x > CLOSE_X0 && event.x < CLOSE_X1 &&
			event.y > CLOSE_Y0 && event.y < CLOSE_Y1) {
			break;
		}


		lcd_point(frame_map, event.x, event.y, COLOR_BLACK);
	}

	lcd_fillScreen(frame_map, COLOR_BLACK);


err_close_ts:
	close(ts_fd);
err_close_fb:
 	close(fb_fd);
err_out:
	return err;
}

