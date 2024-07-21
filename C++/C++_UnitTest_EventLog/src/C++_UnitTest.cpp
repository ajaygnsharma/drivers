//============================================================================
// Name        : C++_UnitTest.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "lib_event_log.h"

using namespace std;

void print_page(){
	int idx = 0;
	int n = 0;

	for(idx=0; idx < 25; idx++) {
		char buf[80];
		n = EventLog().next_entry(buf);
		if(n != -1) {
			printf("%s\n", buf);
		} else {
			if(idx == 0)
				printf("No New Events\n");
			break;
		}
	}
}

int main() {
	uint32_t val = 30;
	EventLog e0 = EventLog(EVT_CIS, EVT_INF | EVT_DATA, static_cast<double>(val));
	e0.enqueue();
	uint16_t shp_time = 1440;
	EventLog e1 = EventLog(EVT_SHP, EVT_INF | EVT_DATA, static_cast<double>(shp_time));
  e1.enqueue();

  uint32_t ip_addr = 0x0A0A0C20;
	EventLog e2 = EventLog(EVT_CIA, EVT_INF | EVT_DATA, static_cast<double>(ip_addr));
  e2.enqueue();

	uint32_t gateway = 0x0A0A0C01;
	EventLog e3 = EventLog(EVT_CIG, EVT_INF | EVT_DATA, static_cast<double>(gateway));
  e3.enqueue();

	uint32_t netmask = 0xFF000000;
	EventLog e4 = EventLog(EVT_CIM, EVT_INF | EVT_DATA, static_cast<double>(netmask));
  e4.enqueue();

  int32_t tzo = -100;
	EventLog e5 = EventLog(EVT_TZO, EVT_INF | EVT_DATA, static_cast<double>(tzo));
  e5.enqueue();

  uint32_t cis = 3;
	EventLog e6 = EventLog(EVT_CIS, EVT_INF | EVT_DATA, static_cast<double>(cis));
  e6.enqueue();

  uint32_t bam = 10;
	EventLog e7 = EventLog(EVT_BAM, EVT_INF | EVT_DATA, static_cast<double>(bam));
  e7.enqueue();

  uint32_t bsm = 10;
	EventLog e8 = EventLog(EVT_BSM, EVT_INF | EVT_DATA, static_cast<double>(bsm));
  e8.enqueue();

  print_page();

	return 0;
}
