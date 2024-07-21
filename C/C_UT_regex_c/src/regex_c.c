/*
 ============================================================================
 Name        : regex_c.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
// C program to search and replace
// all occurrences of a word with
// other word.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>              /* Regular Exp     POSIX */
#include <stdint.h>

#define _BSD_SOURCE             /* To get NI_MAXHOST and NI_MAXSERV
                                   definitions from <netdb.h> */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

bool read_conf(char *buf){
	FILE * fp;
	int i = 0, j = 0;
	char *line  = NULL;
	ssize_t len = 0, read, fsize, result;

	fp = fopen("./ntp.org", "r+");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	fseek(fp , 0 , SEEK_END);
	fsize = ftell(fp);
	rewind(fp);

	result = fread(buf,1,fsize, fp);
	if (result != fsize) {fputs ("Reading error",stderr); return false;}

	fclose (fp);
	return true;
}

int set_ntp_cfg_ip(char *newIP)
{
   regex_t    preg;
   //char       *pattern = "\\b[0-9].[0-9].[0-9].[0-9]\\b";
   char       *pattern = ".";
   int        rc;
   size_t     nmatch = 2;
   regmatch_t pmatch[2];

   char buf[400];
   read_conf(buf);
   printf("%s",buf);

   if (0 != (rc = regcomp(&preg, pattern, 0))) {
      printf("regcomp() failed, returning nonzero (%d)\n", rc);
      exit(EXIT_FAILURE);
   }

   if (0 != (rc = regexec(&preg, buf, nmatch, pmatch, 0))) {
      printf("Failed to match '%s' with '%s',returning %d.\n",
             buf, pattern, rc);
   }
   else {
      printf("With the whole expression, "
             "a matched substring \"%.*s\" is found at position %d to %d.\n",
             pmatch[0].rm_eo - pmatch[0].rm_so, &buf[pmatch[0].rm_so],
             pmatch[0].rm_so, pmatch[0].rm_eo - 1);
   }
   regfree(&preg);

   memcpy(&buf[pmatch[0].rm_so], newIP, strlen(newIP));

   printf("%s",buf);

   return 0;
}

uint32_t get_ntp_cfg_ip(void){
    regex_t    preg;
#if(0)
    char       *pattern = "\\b[[:digit:]][[:digit:]][[:digit:]]."
    					     "[[:digit:]][[:digit:]][[:digit:]]."
    		                 "[[:digit:]][[:digit:]][[:digit:]]."
    		                 "[[:digit:]][[:digit:]][[:digit:]]\\b";
#endif
    char       *pattern = "\\b[[:digit:]]{3}.[[:digit:]]{3}.[[:digit:]]{3}.[[:digit:]]{3}\\b";

	int        rc;
	size_t     nmatch = 2;
	regmatch_t pmatch[2];
	struct in_addr inp = {};

	char buf[400];
	read_conf(buf);
	//printf("%s",buf);

	if (0 != (rc = regcomp(&preg, pattern, REG_EXTENDED|REG_NEWLINE))) {
		printf("regcomp() failed, returning nonzero (%d)\n", rc);
		exit(EXIT_FAILURE);
	}

	if (0 != (rc = regexec(&preg, buf, nmatch, pmatch, 0))) {
		printf("Failed to match '%s' with '%s',returning %d.\n",
				buf, pattern, rc);
	}
	else {
		printf("With the whole expression, "
				"a matched substring \"%.*s\" is found at position %d to %d.\n",
				pmatch[0].rm_eo - pmatch[0].rm_so,
				&buf[pmatch[0].rm_so],
				pmatch[0].rm_so,
				pmatch[0].rm_eo - 1);
	}

	regfree(&preg);

	char ip_addr[16] = {};
	strncpy(ip_addr, &buf[pmatch[0].rm_so], 16);
	if(inet_aton(ip_addr, &inp)){
		return inp.s_addr;
	}else{
		return 0;
	}
}

// Driver Program
int main()
{
	printf("NTP IP=%x",get_ntp_cfg_ip());
	//set_ntp_cfg_ip("202.203.204.205");
	return 0;
}

