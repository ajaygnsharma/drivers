/*
 * 10_atoi_with_pad.cpp
 *
 *  Created on: Mar 17, 2022
 *      Author: asharma
 */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

using namespace std;

union cli_fifo_msg_s{
	char buff[1024 + 2];
	struct packet_s{
		char id;
		char payload[1024];
	}packet;
};

int main(void){
  union cli_fifo_msg_s fmsg;
  union cli_fifo_msg_s fmsg_rsp;

  sprintf(fmsg.packet.payload, "%d\r\n", 15);

  fmsg.packet.id = 1;

	memcpy(fmsg_rsp.buff, fmsg.buff, sizeof(fmsg.buff));




	int col = 0, type = 0;
	sscanf(fmsg_rsp.packet.payload, "%2d", &col);

	type = fmsg_rsp.packet.id;
  printf("col=%d, id=%d",col,type);
}
