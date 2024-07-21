/*
 ============================================================================
 Name        : 103_double_ptr.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : A small program to show use of double pointer to allocate memory
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int my_asprintf(char **buf, char *msg){
  int n = -1;
  if(msg != NULL){
    n = strlen(msg);
    *buf = malloc(n + 1);
    if(*buf != NULL){
      strcpy(*buf, msg);
      buf[n] = '\0';
    }else{
      n = -1;
    }
  }
  return n;
}

int main(void) {
  char *buf = NULL;
  int n = my_asprintf(&buf, "hi there");
	printf("%s, n=%d\n",buf, n);
	free(buf);
	return EXIT_SUCCESS;
}
