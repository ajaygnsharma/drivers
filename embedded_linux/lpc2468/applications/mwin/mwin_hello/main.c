/*
 * main.c
 *
 *  Created on: Apr 22, 2012
 *      Author: user1
 */
/*
 * Arc drawing demo for Nano-X
 *
 * Copyright (C) 2002 Alex Holden <alex@alexholden.net>
 * Modified by G Haerr
 */
#include <stdio.h>
#include <nano-X.h>

#define RECT_X0 50
#define RECT_X1 190
#define RECT_Y0 50
#define RECT_Y1 150

static GR_WINDOW_ID mainwid;
static GR_WINDOW_ID rectwid;
static GR_GC_ID rectgc;


static void
handleevent(GR_EVENT *ep)
{
	GR_EVENT_MOUSE mouse;
	switch(ep->type){
	case GR_EVENT_TYPE_MOUSE_MOTION:
	  mouse = ep->mouse;
	  if(mouse.wid==rectwid){
	    GrPoint(rectwid, rectgc, mouse.x, mouse.y);
	  }
      break;
	case GR_EVENT_TYPE_CLOSE_REQ:
	  GrClose();
	  exit(0);
	}
}

int
main(void)
{
	GR_SCREEN_INFO si;

	if (GrOpen() < 0)
		exit(-1);

	GrGetScreenInfo(&si);

	mainwid = GrNewWindow(GR_ROOT_WINDOW_ID, 0, 0,
			si.cols, si.rows, 0, WHITE, BLACK);

	rectwid = GrNewWindow(mainwid, RECT_X0, RECT_Y0, RECT_X1-RECT_X0,
			RECT_Y1-RECT_Y0, 1, YELLOW, BLACK);

	GrSelectEvents(rectwid, GR_EVENT_MASK_MOUSE_MOTION);

	rectgc=GrNewGC();
	GrSetGCForeground(rectgc, RED);

	GrMapWindow(mainwid);
	GrMapWindow(rectwid);

	while (GR_TRUE) {
		GR_EVENT event;

		GrGetNextEvent(&event);
		handleevent(&event);
	}

	return 0;
}


