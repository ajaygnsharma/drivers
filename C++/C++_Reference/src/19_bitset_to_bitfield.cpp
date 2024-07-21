/*
 * 19_bitset_to_bitfield.cpp
 *
 *  Created on: May 1, 2023
 *      Author: asharma
 */
#include <bitset>
#include <iostream>
#include <array>
#include <bit>
#include <cstdint>

using namespace std;

struct alarm_bits_s{
		uint8_t a_rx_input_low : 1;/*------------------Byte 1----------------------*/
		uint8_t a_vdc_low      : 1;
		uint8_t a_vdc_high     : 1;
		uint8_t a_idc_low      : 1;
		uint8_t a_idc_high     : 1;

		uint8_t b_rx_input_low : 1;
		uint8_t b_vdc_low      : 1;
		uint8_t b_vdc_high     : 1;
		uint8_t b_idc_low      : 1; /*----------------- Byte 2--------------------*/
		uint8_t b_idc_high     : 1;

		uint8_t a_rx_simulated : 1;
		uint8_t b_rx_simluated : 1;
		uint8_t ref_10MHZ      : 1;
		uint8_t internal_10MHz : 1;
		uint8_t wguide_sw_fault: 1;
		uint8_t emergency_override:1;
};

/* Alarms as bitfields takes 2 bytes. Easy to remember */
union alarm_bits_u{
	struct alarm_bits_s alarm_bits;
	uint16_t all;
};

struct alarm_bits_s status_get_alarms_bits(void){
	bitset<16> a{0b1000111101011011};
	union alarm_bits_u u;

	u.all = a.to_ulong();

	return u.alarm_bits;
}

int main(void){
	struct alarm_bits_s bits = status_get_alarms_bits();
	printf("bit0=%d\nbit15=%d\n",bits.a_rx_input_low, bits.emergency_override);
	return 0;
}
