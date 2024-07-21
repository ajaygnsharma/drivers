/*
 * static_cast.cpp
 *
 *  Created on: Feb 14, 2022
 *      Author: asharma
 */
#include <stdio.h>
#include <iostream>

using namespace std;

struct cli_fifo_msg_s{
	uint8_t type;
	/* I convert this to a pointer instead
	char data[1024];*/
	const char *data;
};

int main(void){
	/* Don't do like this. Not a clean way.
		char *buff = static_cast<char *>(&fifo_rsp);
		*/
	static struct cli_fifo_msg_s fifo_rsp = {
			.type = 20,
			.data = "Foo"
	};

	char buff[1024];
	ssize_t nbytes = sprintf(buff, "%hhd%s", fifo_rsp.type, fifo_rsp.data);
	printf("%s",buff);
}
