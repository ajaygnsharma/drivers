/*
 ============================================================================
 Name        : ut_sscanf_less_scanned.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

char *str = "rouser -s usm auth_no_priv auth -V systemview";

int main(void) {
  char *privilege = NULL;
  char *name = NULL;
  char *view = NULL;

  sscanf(str, "%ms -s usm %ms auth -V %ms", &privilege, &name, &view);
  printf("p=%s, n=%s, v=%s\n", privilege, name, view);
  free(privilege);
  free(name);
  free(view);

	return EXIT_SUCCESS;
}
