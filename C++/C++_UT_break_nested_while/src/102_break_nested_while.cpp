/*
 ============================================================================
 Name        : 102_break_nested_while.cpp
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C++,
 ============================================================================
 */
#include <unistd.h>
#include <iostream>

using namespace std;


int main(void) {
  static int _a = 0;
  static int _b = 1;
  int a;
  int b;

  while(1){
    a = 0;
    b = 0;

    if((a != _a) || (b != _b)){
      _a = a;
      _b = b;
      if(b == 0){
        printf("LED Off\n");
        continue;
      }

      switch (a) {
      case 0:  // Slowly blink green led
        printf("init_leds()\n");
        break;

      case 1: // Quickly flashing Red led
        printf(" Quickly flashing Red led\n");
        break;

      case 2: //
      default:
        printf("Solid red led\n");
        break;
      }
    }
    printf("Running\n");
    sleep(1);
  }
	return 0;
}
