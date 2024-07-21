/*
 * 05_null_ptr.cpp
 *
 *  Created on: Feb 14, 2022
 *      Author: asharma
 */


#include <iostream>
using namespace std;

int main(void){
	int *p1 = nullptr; /* Preferred */
	int *p2 = 0;
	int *p3 = NULL;

	cout << p1;
	cout << p2;
	cout << p3;

	return 0;
}

