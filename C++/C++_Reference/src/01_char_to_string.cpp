/*
 ============================================================================
 Name        : c++_char_to_string.cpp
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C++,
 ============================================================================
 */

#include <iostream>
#include <string>

using namespace std;

int main(void) {
	char name[] = "Ajay Sharma";

	string s = name; /* String class can directly take the c style string */

	cout << s << endl;
	printf("%s\n",s.c_str());

	return 0;
}
