/*
 * 17_sscanf_date_time.cpp
 *
 *  Created on: Feb 1, 2023
 *      Author: asharma
 */
#include <iostream>
#include <cstring>

using namespace std;

struct LNB_stat_s{
	int16_t volt;
	int16_t amp;
	int16_t pin;
};

struct stat_entry_s {
  char   date_time[80];
  struct LNB_stat_s LNB_A;
  struct LNB_stat_s LNB_B;
  int16_t  ten_mhz;
};

int main(void){
	struct stat_entry_s s;

	memset((void *)&s, '\0', sizeof(s));

	string token = "02/01/23-17:38:00,100,125,150,175,200,225,250";
	int n = sscanf(token.c_str(), "%17[^,],%hd,%hd,%hd,%hd,%hd,%hd,%hd",
				s.date_time, &s.LNB_A.volt, &s.LNB_A.amp, &s.LNB_A.pin,
				&s.LNB_B.volt, &s.LNB_B.amp, &s.LNB_B.pin,
				&s.ten_mhz);

	fprintf(stderr,"n=%d\n",n);
	fprintf(stderr,"date_time=%s\n",s.date_time);
	fprintf(stderr,"Amp  A=%d\n", s.LNB_A.amp);
	fprintf(stderr,"Volt A=%d\n", s.LNB_A.volt);
	fprintf(stderr,"Pin  A=%d\n", s.LNB_A.pin);

	fprintf(stderr,"Amp  B=%d\n", s.LNB_B.amp);
	fprintf(stderr,"Volt B=%d\n", s.LNB_B.volt);
	fprintf(stderr,"Pin  B=%d\n", s.LNB_B.pin);

	fprintf(stderr,"TenMHz=%d\n", s.ten_mhz);

	return 0;
}
