//============================================================================
// Name        : struct_initialization.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

struct cfg_s{
	int a;
	struct foo_s{
		int b;
		int c;
		int d;
	} foo ;
};

static const struct cfg_s cfg = {
	.a = 10,
	.foo = { .b = 100, .c = 200, .d = 300 }
};

int main() {
  printf("a=%d, b=%d, c=%d, d=%d\n", cfg.a, cfg.foo.b, cfg.foo.c, cfg.foo.d);
	return 0;
}
