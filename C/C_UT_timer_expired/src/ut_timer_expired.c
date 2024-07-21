/*
 ============================================================================
 Name        : ut_timer_expired.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#define _POSIX_C_SOURCE 199309

static volatile sig_atomic_t gotAlarm = 0;

/*
 * ut_timer_expire.c
 *
 *  Created on: Oct 7, 2019
 *      Author: asharma
 */
static void ADC_Handler(int sig){
	  gotAlarm = 1;
}

int adc_timer_init(void){
	struct itimerval itv;
    struct sigaction  sa;
    struct sigevent sev;
    timer_t tid;
    int j;
    int maxSigs;
    int sigCnt = 0;

    /* Establish handler for notification signal */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = ADC_Handler;
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    	perror("sigaction");

    itv.it_value.tv_sec = 5;
    itv.it_value.tv_usec = 0;
    itv.it_interval.tv_sec = 2;
    itv.it_interval.tv_usec = 0;

    maxSigs = 10;

    /* Allows handler to get ID of this timer */
    //if (timer_create(CLOCK_REALTIME, &sev, &tid) == -1)
    //	perror("timer_create");
    //printf("ADC Timer ID: %ld\n", (long) tid);
    if (setitimer(ITIMER_REAL, &itv, NULL) == -1)
    	perror("timer_settime");

    while(1){
    	if (gotAlarm) {                     /* Did we get a signal? */
    		gotAlarm = 0;
    		sigCnt++;
    		printf("Signal Count=%d\n",sigCnt);
    		if (sigCnt >= maxSigs) {
    			printf("Done\n");
    			exit(EXIT_SUCCESS);
    		}
    	}
    }
}

int main(void){
	adc_timer_init();
}
