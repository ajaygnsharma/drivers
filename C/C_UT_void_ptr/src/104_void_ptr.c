/*
 ============================================================================
 Name        : 104_void_ptr.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
  int num = 101;
  int *pi = &num;
  printf("Value of pi: %d\n", *pi);
  void* pv = pi;
  pi = (int*) pv;
  printf("Value of pi: %d\n", *pi);

  //value of pi: 101
  //value of pi: 101
	return EXIT_SUCCESS;
}
