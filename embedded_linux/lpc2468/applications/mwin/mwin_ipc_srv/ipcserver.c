/*
 * Nano-X IPC server demonstration- start one copy of ipccserver first, then
 * run as many ipcclients as you want.
 *
 * Copyright (c) 2002 Alex Holden <alex@alexholden.net>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "nano-X.h"
typedef unsigned char GR_PROP;
 
 /**
   * GrChangeProperty:
:   *
:   * @wid: the ID of the window the property is attached to
:   * @property: the name of the property
:   * @data: the data to be stored in the property
:   * @len: the length of the data to be stored in the property
:   *
:   * Changes the data stored in the specified property attached to the
:   * specified window. If the property does not already exist, it will be
:   * created. The property name should be a zero terminated string. To
:   * create a global property, attach it the root window
:   * (GR_ROOT_WINDOW_ID). If data is a null pointer but len is 1, the
:   * property is created with no data block attached to it. If len is 0,
:   * the property is deleted.
:   */
/*void GrChangeProperty(GR_WINDOW_ID wid, GR_CHAR *property,
                        GR_PROP *data, GR_COUNT len);*/
 
/**
:   * GrGetWindowProperty:
:   *
:   * @wid: the ID of the window the property is attached to
:   * @property: the name of the property
:   * @data: pointer to a pointer to hold the address of the returned data
:   * @Returns: the length of the returned data block
:   *
:   * Searches for a property with the specified name in the list of
:   * properties attached to the specified window and returns the data
:   * block associated with the property. If no such property is found, the
:   * length is zero and the a null pointer is stored in the data pointer.
:   * If the property exists but no data block is associated with it, 1 is
:   * returned for the length and the data pointer is set to NULL. The data
:   * block is allocated with malloc() at call time, and it is the callers
:   * responsibility to free it when no longer needed.
:   */
/* GR_COUNT GrGetWindowProperty(GR_WINDOW_ID wid, GR_CHAR *property,
                          GR_PROP **data);*/
 
/*: These are basically much simplified versions of XChangeProperty() and 
: XGetWindowProperty(). It's also possible to create an analog of 
: XDeleteProperty() with just a #define:
:*/ 
#define GrDeleteProperty(wid, property) GrChangeProperty(wid, \
                                               property, NULL, 0)

static void handle_packet(GR_WINDOW_ID wid, GR_EVENT *event)
{
	GR_EVENT_CLIENT_DATA *pkt = (GR_EVENT_CLIENT_DATA *)event;

	if(pkt->len == 4 && !memcmp(pkt->data, "ping", 4)) {
		printf("Got ping packet from window %d with serial %lu, "
				"sending pong reply\n", pkt->rid, pkt->serial);
		GrSendClientData(wid, pkt->rid, pkt->serial,
			4, 4, "pong");
	} else printf("Got unknown packet from window %d\n", pkt->rid);
}

int main(int argc, char *argv[])
{
	GR_EVENT event;
	GR_WINDOW_ID wid;

	if(GrOpen() < 0) {
		fprintf(stderr, "Couldn't connect to Nano-X server\n");
		return 1;
	}

	wid = GrNewWindow(GR_ROOT_WINDOW_ID, 0, 0, 1, 1, 0, 0, 0);
	if(!wid) {
		fprintf(stderr, "Couldn't get a window\n");
		GrClose();
		return 1;
	}

	GrSelectEvents(wid, GR_EVENT_MASK_CLIENT_DATA);

	//GrChangeProperty(GR_ROOT_WINDOW_ID, "demo_ipc_server", (GR_PROP *)&wid,
			//sizeof(wid));

	printf("Registered server window %d\n", wid);

	while(1) {
		GrGetNextEvent(&event);
		switch(event.type) {
			case GR_EVENT_TYPE_CLIENT_DATA:
				handle_packet(wid, &event);
				break;
			default:
				fprintf(stderr, "Got unknown event %d\n",
						event.type);
				break;
		}
	}

	GrClose();

	return 0;
}
