#include <iostream>

using namespace std;

#define MAX_PW_SALT_LEN (3 + 16 + 1)

int main(void){
	/* This is a unique way of declaring the size of an array. sizeof is used */
	char salt[MAX_PW_SALT_LEN + sizeof("rounds=999999999$")];
	cout << sizeof(salt);
}
