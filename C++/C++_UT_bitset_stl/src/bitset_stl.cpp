/*
 ============================================================================
 Name        : bitset_stl.cpp
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C++,
 ============================================================================
 */
#include <bitset>
#include <iostream>

/*
 * alarmlog.c
 *
 * Created: 1/16/2014 8:10:00 AM
 *  Author: rmcdonald
 */
//#include "asf.h"
#include "string.h"
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

/* C++ declarations */
#include <bitset>
#include <iostream>
#include <array>

using namespace std;

const uint8_t EVT_ALM = 1;
const uint8_t EVT_CLR = 2;
const uint8_t EVT_INF = 3;

enum alarm_e{
	A_RX_INPUT_LVL_LO_ALM	= 0x0001,
	B_RX_INPUT_LVL_LO_ALM = 0x0002,
	A_RX_SIMULATED_ALM    = 0x0004,
	B_RX_SIMULATED_ALM    = 0x0008,
	REF_10MHZ_ALM     		= 0x0010,
	A_VDC_LO_TH_ALM      	= 0x0020,
	A_VDC_HI_TH_ALM      	= 0x0040,
	B_VDC_LO_TH_ALM       = 0x0080,
	B_VDC_HI_TH_ALM       = 0x0100,
	WGUIDE_SW_FAULT_ALM   = 0x0200,
	A_IDC_LO_TH_ALM       = 0x0400,
	A_IDC_HI_TH_ALM       = 0x0800,
	B_IDC_LO_TH_ALM       = 0x1000,
	B_IDC_HI_TH_ALM       = 0x2000,
	EMERG_OVERRIDE_ON     = 0x4000,
	INTERNAL_10MHZ_1      = 0x8000,
};

//static uint16_t rx_log_alarms_prev = 0;
static bitset<16> previous;

array<uint16_t, 16> alarms = {
		A_RX_INPUT_LVL_LO_ALM,
		B_RX_INPUT_LVL_LO_ALM,
		A_RX_SIMULATED_ALM,
		B_RX_SIMULATED_ALM,
		REF_10MHZ_ALM,
		A_VDC_LO_TH_ALM,
		A_VDC_HI_TH_ALM,
		B_VDC_LO_TH_ALM,
		B_VDC_HI_TH_ALM,
		WGUIDE_SW_FAULT_ALM,
		A_IDC_LO_TH_ALM,
		A_IDC_HI_TH_ALM,
		B_IDC_LO_TH_ALM,
		B_IDC_HI_TH_ALM,
		EMERG_OVERRIDE_ON,
		INTERNAL_10MHZ_1,
};

/*******************************************************************************
Monitor alarms and log when they change state.
*******************************************************************************/
static void alarm_log_check(uint16_t rx_alm){
  bitset<16> total_change;
	bitset<16> current{rx_alm};

	total_change  = current ^ previous;
	previous      = current;

	for(int i = 0; i < alarms.size(); i++){
		if(total_change[i] == true){ /* A specific alarm is changed */
			if(current[i]){            /* It is set */
				printf("alarm: %x, evt=%u\n", alarms[i], EVT_ALM);
			}else{                     /* It is clear */
				printf("alarm: %x, evt=%u\n", alarms[i], EVT_CLR);
			}
		}
	}
}

/* Alarm log timed poll */
void alarm_log_tpoll(void){
  uint16_t rx_alm   = 0b0100101011110000;
  alarm_log_check(rx_alm);

  cout << endl;

  rx_alm   = 0b1011010100001111;
  alarm_log_check(rx_alm);
}

int main()
{
    std::bitset<5> x;

    // Implicitly stays 0: x[0] = 0;
    x[1] = 1;
    x[2] = 0;
    // Note x[0-4]  valid

    std::cout << x << std::endl;

    alarm_log_tpoll();


}
