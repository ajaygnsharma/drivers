/*
 * 21_bitset_for_loop.cpp
 *
 *  Created on: Jun 1, 2023
 *      Author: asharma
 */
#include <iostream>
using namespace std;

int main(void){
	for(unsigned int i = 0; i < 16; i++){
		uint16_t alm = (1 << i);
		cout << hex << alm << endl;
	}
	return 0;
}
