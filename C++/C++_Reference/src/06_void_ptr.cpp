/*
 * 06_void_ptr.cpp
 *
 *  Created on: Feb 16, 2022
 *      Author: asharma
 */



#include <iostream>

using namespace std;

/* Take the void ptr and cast it to another valid "type" pointer
 * and then do whatever you normally with a ptr
 */
void copy_foo(void *mem){
	char buff[100];
	char *ptr;
	ptr = static_cast<char *>(mem);

	for(int i = 0; i < 10; i++){
		buff[i] = ptr[i];
	}

	for(int i = 0; i < 10; i++){
		printf("%d\n",buff[i]);
	}
}

void square(void){
	int i = 42;

	int *p1 = &i;
	*p1 = *p1 * *p1;
	printf("%d\n",*p1);
}

void valid_invalid(void){
	int i = 0;
#if(0)
	double *dp = &i; /* Invalid. double ptr to int type */
#endif
#if(0)
	int *ip = i; /* Invalid. Should point to address of a type */
#endif
	int *p = &i; /* valid. */
}

void what(void){
	int i = 10;
	int *ptr = &i;

	if(ptr){
		printf("checks if ptr is null or not\n");
	}
	if(*ptr){
		printf("checks if the value of ptr is > 0\n");
	}

	if(ptr == nullptr){
		printf("ptr is not nullptr\n");
	}
}

void legal(void){
	int i = 42;

	void *p = &i;/* Valid because we want don't know the type and want to assign */
#if(0)
	long *lp = &i; /* Invalid because we want the type to be long */
#endif
}

int main(void){
	double obj = 3.14;
	double *pd = &obj; /* valid ptr type */

	void *pv = &obj; /* void ptr can point to any type */

	pv = pd;

	if(pv == pd){
		printf("ptrs equal\n");
	}

	char src[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	copy_foo(src);

	/* pd = pd + 1; */
	*pd = 4.5;
	printf("%f\n",obj); /* Prints 4.5 */

	square();

	valid_invalid();

	what();

	legal();
}
