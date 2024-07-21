/*
 * 09_serialize_to_string_with_pad.cpp
 *
 *  Created on: Mar 16, 2022
 *      Author: asharma
 */
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

int main(void){
	char buff[10] = {"Foo"};
	char id = 1;
	char id_buff[3];

	int n = snprintf(id_buff, 3, "%02d", id);
	string server_str = id_buff;
	server_str = server_str + buff + "\r\n";
	cout << server_str << endl;

	return 0;
}
