/*
 ============================================================================
 Name        : ut_while_loop_continue.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	int a = 0;
	int status = 1;

	while(1){
		if(status == 1){
			sleep(2);
			printf("Continuing\n");
			status = 0;
			continue;
		}
		a = a+1;
		printf("a=%d\n",a);
		status = 1;
	}

	return EXIT_SUCCESS;
}
