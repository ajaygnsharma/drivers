/*
 ============================================================================
 Name        : 101_cfg_read_getline.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

char *find_key(char *buf, char *key){
  char *ptr = NULL;

  ptr = strstr(buf,key);

  if(ptr != NULL){
    char *p = strpbrk(ptr,"=");
    p++;
    return p;
  }else{
    return NULL;
  }
}

struct cfg_s {
  int val1;
  char str[50];
  int val2;
};

void create_data(void){
  FILE *pFile;
  char *buf = NULL;
  char *f_buf = NULL;
  char *to = NULL;
  ssize_t n = 0;
  struct cfg_s cfg = {10, "my_security", 20};

  char *ptr = "mysecurity";
  pFile = fopen("test.txt", "w");

  n = asprintf(&buf, "TEMPERATURE_MAX=%d\n"
                     "SNMP_C2GROUP_SECURITY=%s\n"
                     "BURST_THOLD=%d\n",
                     cfg.val1, cfg.str, cfg.val2 );
  //cfg.val2 = 50;

  fwrite(buf , n, 1, pFile);
  //fprintf(pFile, "SNMP_SEC2GROUP_SECURITY=mysecurity\n");
  //fputs(buffer,pFile);
  fclose(pFile);
  return;
}

int
main(int argc, char *argv[])
{
  FILE *stream;
  char *line = NULL;
  size_t len = 0;
  ssize_t n;
  size_t t = 0;
  char user_title[33];
  int temp_min=0;
  char *ptr = NULL;

  /* First load defaults */
  create_data();

  stream = fopen("/home/asharma/eclipse-workspace1/101_cfg_read_getline/src/test.txt", "r");
  if (stream == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  struct stat st;
  stat("/home/asharma/eclipse-workspace1/101_cfg_read_getline/src/test.txt", &st);
  printf("filesize=%ld\n",st.st_size);

  char *buf = malloc(st.st_size);

  while ((n = getline(&line, &len, stream)) != -1) {
    strcpy(&buf[t], line);
    t += n;
  }

  ptr = find_key(buf,"TEMPERATURE_MAX");
  if(ptr != NULL){
    temp_min = strtoul(ptr, NULL, 10);
    if(temp_min == ULONG_MAX){
      temp_min = 0; // or default
    }
  }

  ptr = find_key(buf,"BURST_THOLD");
  if(ptr != NULL){
    temp_min = strtoul(ptr, NULL, 10);
  }

  ptr = find_key(buf,"SNMP_SEC2GROUP_SECURITY");
  if(ptr != NULL){
    int i = 0;
    while(*ptr != '\n'){
      user_title[i] = *ptr;
      ptr++;
      i++;
    }
    user_title[i] = '\0';
  }else{
    strcpy(user_title,"my_security");
  }

  printf("burst_thold=%d\n",temp_min);
  printf("SEC2GROUP=%s\n",user_title);
  printf("temp_min=%d\n",temp_min);

  //free(line);
  fclose(stream);
  exit(EXIT_SUCCESS);
}
