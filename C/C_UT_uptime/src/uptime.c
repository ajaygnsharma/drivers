/*
 ============================================================================
 Name        : uptime.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>

int main(void) {
	struct sysinfo info;
	unsigned updays, uphours, upminutes;

	if(sysinfo(&info)<0){
		perror("sysinfo failed:");
	}else{
		updays = (unsigned) info.uptime / (unsigned)(60*60*24);
		if (updays != 0)
			printf("%u day%s, ", updays, (updays != 1) ? "s" : "");
		upminutes = (unsigned) info.uptime / (unsigned)60;
		uphours = (upminutes / (unsigned)60) % (unsigned)24;
		upminutes %= 60;
		if (uphours != 0)
			printf("%2u:%02u", uphours, upminutes);
		else
			printf("%u min", upminutes);
	}

	return EXIT_SUCCESS;
}
