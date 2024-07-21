/*
 ============================================================================
 Name        : 105_clamp_value.cpp
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C++,
 ============================================================================
 */

#include <iostream>
using namespace std;

int main() {
  int v = 4096;
  int a = 4095;
  if(v > a){
    v = 4095;
  }

  printf("v=%d, a=%d\n", v, a);
  return 0;
}
