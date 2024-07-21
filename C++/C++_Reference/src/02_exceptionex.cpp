/*
 * c++_exceptionex.cpp
 *
 *  Created on: Feb 14, 2022
 *      Author: asharma
 */



#include <iostream>
using namespace std;

int main(void){
	//const char *str = "Ajay Sharma";
	const char *str = "100";
	int num = 0;
	if(str != NULL){
		try{
			string req = str;
			num = stoi(req,nullptr);
			cout<< num <<endl;
		}catch(int e){
			cerr << "exception " << e <<endl;
		}
	}


}
