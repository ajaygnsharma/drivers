/*
 * chronos_timer_api.cpp
 *
 *  Created on: Sep 14, 2022
 *      Author: asharma
 */
#include <stdint.h>
#include <unistd.h>

#include <chrono>
#include <iostream>


using namespace std::chrono;
using namespace std;

time_point<system_clock> t1;

struct timer_s{
	uint32_t delay = 0;
	time_point<system_clock> t0;
}timers[5];

enum timer_type_e{
	SECOND_TIMER,
	MILLISECOND_TIMER,
};

void lib_timer_init(uint8_t i, uint32_t d, enum timer_type_e timer_type){
	if(timer_type == SECOND_TIMER){
		timers[i].delay = d * 1000;
	}else{
		timers[i].delay = d;
	}
	timers[i].t0 = system_clock::now();
}

bool lib_timer_expired(uint8_t i){
	bool expired = false;
	t1 = system_clock::now();
  duration<float> fs = (t1 - timers[i].t0);
  milliseconds ms = duration_cast<milliseconds>(fs);

  if(ms.count() > timers[i].delay){
  	expired = true;
  }

	return expired;
}

void lib_timer_reload(uint8_t i){
	timers[i].t0 = system_clock::now();
}

int main(void){
	lib_timer_init(0, 5000, MILLISECOND_TIMER);
	while(!lib_timer_expired(0)){
		//lib_timer_reload(0);
		sleep(1);
	}
	cout << "Timer expired" << endl;
}
