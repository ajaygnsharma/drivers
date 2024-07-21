/*
 ============================================================================
 Name        : 108_ms_timelapse.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

int main(void) {
  struct timeval  t1,t2;
  gettimeofday(&t1, NULL);
  // convert tv_sec & tv_usec to millisecond  return EXIT_SUCCESS;
  double _t1 =  (t1.tv_sec * 1000) + (t1.tv_usec / 1000) ;
  sleep(1);
  gettimeofday(&t2, NULL);
  double _t2 =  (t2.tv_sec * 1000) + (t2.tv_usec / 1000);
  printf("t1.sec=%ld, t1.usec=%ld, t2.sec=%ld, t2.usec=%ld\n",\
      t1.tv_sec, t1.tv_usec, t2.tv_sec, t2.tv_usec);
  printf("msec passed=%f\n", _t2 - _t1);
}
