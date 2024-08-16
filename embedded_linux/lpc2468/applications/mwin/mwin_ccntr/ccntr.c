/*
 * Arc drawing demo for Nano-X
 *
 * Copyright (C) 2002 Alex Holden <alex@alexholden.net>
 * Modified by G Haerr
 */
#include <stdlib.h>
//#define MWINCLUDECOLORS
#include "nano-X.h"

static void
draw(GR_EVENT *ep)
{
	GR_WINDOW_ID wid = ((GR_EVENT_EXPOSURE *)ep)->wid;
	GR_GC_ID gc = GrNewGC();
	int x = 40;
	int y = 40;
	int rx = 30;
	int ry = 10;
	int yoff = (ry + 2);

	//GrSetGCForeground(gc, GREEN);

	/* filled arc*/

	/* outlined arc*/


	GrSetGCForeground(gc, BLACK);
	GrSetGCBackground(gc, WHITE);

	GrText(wid, gc, x, y, "IP : 10.0.0.98", -1, 0);
	y += yoff;
	GrText(wid, gc, x, y, "ADC: 19", -1, 0);

	GrDestroyGC(gc);
}

int
main(int ac, char **av)
{
	GR_EVENT ev;
	GR_WINDOW_ID wid;

	if (GrOpen() < 0)
		exit(-1);

	wid = GrNewWindowEx(GR_WM_PROPS_BORDER|GR_WM_PROPS_CAPTION|
		GR_WM_PROPS_CLOSEBOX, "arcdemo",
		GR_ROOT_WINDOW_ID, 0, 0, 250, 90, WHITE);

	GrSelectEvents(wid, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_CLOSE_REQ);
	GrMapWindow(wid);

	while (1) {
		GrGetNextEvent(&ev);

		if (ev.type == GR_EVENT_TYPE_CLOSE_REQ)
			break;
		if (ev.type == GR_EVENT_TYPE_EXPOSURE)
			draw(&ev);
	}

	GrClose();

	return 0;
}
