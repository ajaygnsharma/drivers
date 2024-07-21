/*
 ============================================================================
 Name        : static_lib.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "ip_helper.h"

int main(void) {

	printf("ip=%d",parse_interface_file("ip"));

	puts("Hello World"); /* prints Hello World */
	return EXIT_SUCCESS;
}
