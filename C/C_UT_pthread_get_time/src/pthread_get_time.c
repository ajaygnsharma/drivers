/*
 ============================================================================
 Name        : pthread_get_time.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void measure(void){
  struct timespec tp;
  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);
  int amps = 0;
  int amp = 0;

  while(1){
	  amps += amp;

	  clock_gettime(CLOCK_REALTIME, &tp);
	  if((tp.tv_sec - start.tv_sec) >= 5){
		  start.tv_sec = tp.tv_sec;
		  amp += 1;
		  printf("start.tv_sec=%ld\n",start.tv_sec);
	  }
  }
}

int main(void) {
	measure();
	return EXIT_SUCCESS;
}
