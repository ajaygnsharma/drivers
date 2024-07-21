/*
 ============================================================================
 Name        : network_interface_parse.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

uint32_t parse_interface_file(void){
	FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char *token[80];
    char delim[2] = " ";
    int i = 0, j = 0;
    int line_num=0;
    char buf[200];
    char addr[200];
    char total_buf[200] = {};
    struct in_addr inp;

    fp = fopen("./interfaces", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
    	i = 0;
    	j = 0;
    	strcpy(buf,line);
    	if(buf[0] == '#'){
        	continue;
        }else{
        	if(sscanf (buf,"  address %s",addr)){
        		if(addr[0]!='\0'){
        			if(inet_aton(addr, &inp))
        				inp.s_addr = ntohl(inp.s_addr);
        				return inp.s_addr;
        		}
        	}
        }
    }
    fclose(fp);
    if (line)
        free(line);
    return 0;
}

void writeip(char *ip)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char *token[80];
    char delim[2] = " ";
    int i = 0, j = 0;
    int line_num=0;
    char buf[200];
    char addr[200];
    char total_buf[200] = {};

    fp = fopen("./interfaces", "r+");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
    	i = 0;
    	j = 0;
    	strcpy(buf,line);
    	if(buf[0] == '#'){
        	continue;
        }else{
        	if(sscanf (buf,"  address %s",addr)){
        		if(addr[0]!='\0'){
        			sprintf(buf,"  address %s\n",ip);
        		}
        	}
        }
    	strcat(total_buf,buf);
    }

    printf("%s",total_buf);
    fclose(fp);

    fp = fopen("./interfaces", "w");
    if (fp == NULL)
    	exit(EXIT_FAILURE);
    fprintf(fp,"%s",total_buf);
    fflush(fp);

    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
	printf("%x",parse_interface_file());
}
