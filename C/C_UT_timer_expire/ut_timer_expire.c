/*
 * ut_timer_expire.c
 *
 *  Created on: Oct 7, 2019
 *      Author: asharma
 */


static void ADC_Handler(int sig){
	puts("ADC timer expired");
}

int adc_timer_init(void){
    struct itimerspec ts;
    struct sigaction  sa;
    struct sigevent sev;
    int j;

    /* Establish handler for notification signal */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = ADC_Handler;
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    	perror("sigaction");

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMAX;

    /* Notify via signal */
    /* Notify using this signal */

    /* Allows handler to get ID of this timer */
    if (timer_create(CLOCK_REALTIME, &sev, &tid) == -1)
    	perror("timer_create");
    printf("ADC Timer ID: %ld\n", (long) tid);

    ts.it_value.tv_sec=0;
    ts.it_value.tv_nsec=0;
    ts.it_interval.tv_sec = 2; //0
    ts.it_interval.tv_nsec = 0;//125000;

    if (timer_settime(tid, 0, &ts, NULL) == -1)
    	perror("timer_settime");
}

int main(void){
	adc_timer_init();

	while(1){
		pause();
	}
}
