/*
 * 04_serialize_to_string.cpp
 *
 *  Created on: Feb 14, 2022
 *      Author: asharma
 */
#include <stdio.h>
#include <iostream>

using namespace std;

struct point_10MHz_s {
  float level;
  int16_t adc;
};

struct point_10MHz_s get_cal_10MHz_level(uint8_t temp, uint8_t pwr){
  struct point_10MHz_s a;
  a.adc   = 456;
  a.level = 34.56;
  return a;
}

int main(void){
	uint8_t i = 1;
	uint8_t j  = 2;
	string rsp;

	struct point_10MHz_s pt = get_cal_10MHz_level(i, j);
  rsp = "CT0="+to_string(pt.adc)+","+to_string(pt.level);
  cout << rsp << endl;
}
