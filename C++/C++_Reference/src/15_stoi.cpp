/*
 * 15_stoi.cpp
 *
 *  Created on: Nov 4, 2022
 *      Author: asharma
 */
/*
 * 05_null_ptr.cpp
 *
 *  Created on: Feb 14, 2022
 *      Author: asharma
 */


#include <iostream>
using namespace std;

int main(void){
	//string str = "C0A80115";
	string str = "ffffffff";
	size_t sz;
	uint32_t val = stoul(str, &sz, 16 );

	cout << val;

	return 0;
}





