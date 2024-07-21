/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2019.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 23-5 */

/*
 * As an example: refer to timers/ptmr_null_evp.c in The Linux programming
 * interface.
 *
*/
#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#define TIMER_SIG SIGRTMAX              /* Our timer notification signal */
#define BUF_SIZE 1000

/** Clock tick count */
static volatile uint32_t gs_ul_clk_tick = 0;
static volatile uint32_t snmp_time_ticks = 0;
static volatile uint32_t gs_ul_clk_secs = 0;

// This timer controls rs485 transmit timing.
volatile uint32_t rs485_timer = 0;
// This timer controls the when to reset the RS485 link in legacy mode.
volatile uint32_t rs485_legacy_timer = 0;
// This timer controls FSK transmit timing.
volatile uint32_t fsk_timer = 0;
// This timer controls the when to reset the FSK link in legacy mode.
volatile uint32_t fsk_legacy_timer = 0;
// This timer controls the update interval for attenuator setting update.
volatile uint32_t attenuation_timer = 0;
// This timer controls the update interval for the power in and power out cache.
volatile uint32_t pi_pout_cache_timer = 0;
// This timer controls the temperature update interval for min/max temperature.
volatile uint32_t temperature_update_timer = 0;
// Use this inside routines. Not for keeping time between calls.
volatile uint32_t general_timer = 0;
// This timer controls when too much time between bursts has occurred.
volatile uint32_t burst_timer = 0;
// This timer controls tx pup delay timing.
volatile uint32_t tx_start_delay_timer = 0;
// This timer controls maximum time to pulse the waveguide switch.
volatile uint32_t switching_timer = 0;
// This timer is used by sys_wait_ms. Do not use outside this module.
volatile uint32_t wait_timer = 0;
// This timer is used to determine clone link failure.
volatile uint32_t clone_link_timer = 0;
// This timer is used to control clone link message frequency.
volatile uint32_t clone_last_sent_timer = 0;
// This timer is used to hold off waveguide switching.
volatile uint32_t switching_delay_timer = 0;
// This timer is used to delay agc/alc.
volatile uint32_t agc_alc_delay_timer = 0;
// This timer controls time interval between Form C relay changes.
volatile uint32_t update_relay_timer = 0;
// This timer controls the microprocessor activity led blink rate.
volatile uint32_t up_activity_timer = 0;
// This timer controls the external led blink rate.
volatile uint32_t external_led_timer = 0;
// This timer holds off alarm gathering when needed.
volatile uint32_t delay_alarms_timer = 0;
// This timer sets the time for gathering various statistics.
volatile uint32_t log_stats_timer = 0;
// This timer handles the MDIX switching.
volatile uint32_t mdix_timer = 0;
// This timer handles requests for ntp updates.
volatile uint32_t ntp_timer = 0;
// This timer gates sending of traps.
volatile uint32_t snmp_trap_timer = 0;
// This timer gates statistic collection.
volatile uint32_t statistics_timer = 0;
// This timer gates voltage collection time.
volatile uint32_t volt_timer = 0;
// This timer gates ampere collection time.
volatile uint32_t amps_timer = 0;
// This timer gates when a WG switch fault occurs.
volatile uint32_t wg_fail_timer = 0;
// This timer gates the automatic redundant position setting.
volatile uint32_t red_pos_timer = 0;

/* Return a string containing the current time formatted according to
   the specification in 'format' (see strftime(3) for specifiers).
   If 'format' is NULL, we use "%c" as a specifier (which gives the'
   date and time as for ctime(3), but without the trailing newline).
   Returns NULL on error. */
char *
currTime(const char *format)
{
    static char buf[BUF_SIZE];  /* Nonreentrant */
    time_t t;
    size_t s;
    struct tm *tm;

    t = time(NULL);
    tm = localtime(&t);
    if (tm == NULL)
        return NULL;

    s = strftime(buf, BUF_SIZE, (format != NULL) ? format : "%c", tm);

    return (s == 0) ? NULL : buf;
}

static void
handler(int sig, siginfo_t *si, void *uc)
{
	gs_ul_clk_tick++;
	if(gs_ul_clk_tick % 100 == 0) snmp_time_ticks++;
	if(gs_ul_clk_tick % 1000 == 0) gs_ul_clk_secs++;
	if(rs485_timer) rs485_timer--;
	if(rs485_legacy_timer) rs485_legacy_timer--;
	if(fsk_timer) fsk_timer--;
	if(fsk_legacy_timer) fsk_legacy_timer--;
	if(attenuation_timer) attenuation_timer--;
	if(pi_pout_cache_timer) pi_pout_cache_timer--;
	if(temperature_update_timer) temperature_update_timer--;
	if(general_timer) general_timer--;
	if(burst_timer) burst_timer--;
	if(tx_start_delay_timer) tx_start_delay_timer--;
	if(switching_timer) switching_timer--;
	if(wait_timer) wait_timer--;
	if(clone_link_timer) clone_link_timer--;
	if(clone_last_sent_timer) clone_last_sent_timer--;
	if(switching_delay_timer) switching_delay_timer--;
	if(agc_alc_delay_timer) agc_alc_delay_timer--;
	if(update_relay_timer) update_relay_timer--;
	if(up_activity_timer) up_activity_timer--;
	if(external_led_timer) external_led_timer--;
	if(delay_alarms_timer) delay_alarms_timer--;
	if(log_stats_timer) log_stats_timer--;
	if(mdix_timer) mdix_timer--;
	if(ntp_timer) ntp_timer--;
	if(snmp_trap_timer) snmp_trap_timer--;
	if(statistics_timer) statistics_timer--;
	if(volt_timer) volt_timer--;
	if(amps_timer) amps_timer--;
	if(wg_fail_timer) wg_fail_timer--;
	if(red_pos_timer) red_pos_timer--;
}

// See lwip/sys.h for more information
// Returns number of milliseconds expired
// since lwip is initialized
uint32_t sys_now(void)
{
  return gs_ul_clk_tick;
}
//
// Wait milliseconds.
//
void sys_wait_ms(uint32_t ms)
{
  wait_timer = ms;
  while(wait_timer);
}

//
// Read for clock time (sec).
//
uint32_t sys_get_sec(void)
{
  return gs_ul_clk_secs;
}

int main(void){
    struct itimerspec ts;
    struct sigaction  sa;
    struct sigevent   sev;
    timer_t tid = 0;
    int j;

    /* Establish handler for notification signal */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1)
        printf("sigaction");

    /* Allows handler to get ID of this timer */
    if (timer_create(CLOCK_REALTIME, NULL, &tid) == -1)
    	printf("timer_create");
    printf("Timer ID: %ld\n", (long) tid);

    ts.it_value.tv_sec=2;
    ts.it_value.tv_nsec=0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 1000000;

    if (timer_settime(tid, 0, &ts, NULL) == -1)
    	printf("timer_settime");

    sleep(5);
    printf("gs_ul_clk_tick=%d\n",gs_ul_clk_tick);
}
