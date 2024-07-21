/*
 * 20_void_ptr_cast.cpp
 *
 *  Created on: May 2, 2023
 *      Author: asharma
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void){

	void *dest = NULL;
	unsigned long src = 0x1234;

	dest = malloc(8);

#if(0)
	memcpy(dest, (void *)&src, sizeof(src));
	printf("%lx\n",*((unsigned long *)dest + 0));
#endif
	memcpy(static_cast<void *>(dest),&src, sizeof(src));
	printf("%lx\n",*(static_cast<unsigned long *>(dest) + 0));
}
