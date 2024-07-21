/*
 ============================================================================
 Name        : strcspn.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <string.h>

int main ()
{
  char str[] = "CT0=[0,1]";
  int i = 0, j = 0;
  sscanf(str, "CT0=[%d,%d]", &i, &j);
  printf ("i=%d,j=%d\n",i,j);

  return 0;
}
