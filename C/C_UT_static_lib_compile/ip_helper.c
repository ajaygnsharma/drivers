//
// Helper functions for ip address related stuff.
//
#include <string.h>

#define _BSD_SOURCE             /* To get NI_MAXHOST and NI_MAXSERV
                                   definitions from <netdb.h> */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>              /* Regular Exp     POSIX */
#include <stdint.h>

/*******************************************************************************
*******************************************************************************/
uint32_t is_private_ip(uint32_t ip)
{
  uint32_t is_private = 0;

  if(ip >= 0x0A000000 && ip <= 0x0AFFFFFF ||
     ip >= 0xAC100000 && ip <= 0xAC1FFFFF ||
     ip >= 0xC0A80000 && ip <= 0xC0A8FFFF) is_private = 1;
  return is_private;
}
/*******************************************************************************
*******************************************************************************/
int ip_range_invalid(uint32_t address)
{
  // Any address < 1.0.0.0 or > 223.255.255.255 is not allowed.
  if(address < 0x01000000 || address > 0xDFFFFFFF) return 1;
  // Loopback address would make us unreachable.
  if((address & 0xFF000000) == 0x7F000000) return 1;
  return 0;
}
/*******************************************************************************
*******************************************************************************/
int subnet_range_valid(uint8_t subnet)
{
  if(subnet < 8 || subnet > 30) return 1;
  return 0;
}
/*******************************************************************************
*******************************************************************************/
int is_ip_network_or_broadcast(uint32_t addr, uint32_t netmask)
{
  uint32_t broadcast, network;
  // Is the host address 0?
  network = addr & netmask;
  if(network == addr) return 1;
  // Is the host address the subnet broadcast?
  broadcast = addr | ~netmask;
  if(addr == broadcast) return 1;
  return 0;
}
/*******************************************************************************
*******************************************************************************/
uint8_t convert_mask_to_subnet(uint32_t mask)
{
  uint16_t idx;
  uint8_t subnet = 0;
  bool zero = false;
  
  for(idx=0; idx<32; idx++) {
    if(zero == true) {
      if(mask & 0x80000000) return 0;
    } else {
      if(mask & 0x80000000) subnet++;
      else zero = true;
    }
    mask <<= 1;
  }
  return subnet;
}

#if(0)
/*******************************************************************************
*******************************************************************************/
uint32_t convert_ip_to_ulong(char* cp)
{
  uint32_t ipaddr;

  ipaddr = ipaddr_addr(cp);
  if(ipaddr == IPADDR_NONE) return 0;
  ipaddr = htonl(ipaddr);
  return ipaddr;
}
#endif

uint32_t convert_ip_to_ulong(const char *host)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    uint32_t ip_addr;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;        /* Allows IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; // TCP only

    s = getaddrinfo(host, NULL, &hints, &result);
    if (s != 0) {
        errno = ENOSYS;
        return 0;
    }
    /* Walk through returned list until we find an address structure
       that can be used to successfully connect a socket */
    for (rp = result; rp != NULL; rp = rp->ai_next) {
    	memcpy((uint8_t *)&ip_addr,&rp->ai_addr->sa_data[2], 4);
    	//printf("AI_FAMILY=%d, AI_ADDR=%X\n", rp->ai_family, htonl(ip_addr));
    }
    freeaddrinfo(result);

    return ip_addr;
}


bool saveip(uint32_t ip_address){
	struct in_addr ip_addr;
	ip_addr.s_addr = ip_address;
	printf("The IP address is %s\n", inet_ntoa(ip_addr));

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

    fp = fopen("/etc/network/interfaces", "r+");
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
        			sprintf(buf,"  address %s\n",inet_ntoa(ip_addr));
        		}
        	}
        }
    	strcat(total_buf,buf);
    }
    //printf("%s",total_buf);
    fclose(fp);

    fp = fopen("/etc/network/interfaces", "w");
    if (fp == NULL)
    	exit(EXIT_FAILURE);
    fprintf(fp,"%s",total_buf);
    fflush(fp);

    fclose(fp);
    if (line)
        free(line);
}

uint32_t parse_interface_file(const char *field){
	FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char *token[80];
    char delim[2] = " ";
    int i = 0, j = 0;
    int line_num=0;
    char buf[200]       = {};
    char addr[200]      = {};
    char total_buf[200] = {};
    struct in_addr inp = {};

    fp = fopen("/etc/network/interfaces", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
    	i = 0;
    	j = 0;
    	strcpy(buf,line);
    	if(buf[0] == '#'){
        	continue;
        }else{
        	if(!strcmp(field,"ip")){
				if(sscanf (buf,"  address %s",addr) && addr[0] != '\0'){
					break;
				}
        	}else if(!strcmp(field,"gateway")){
				if(sscanf (buf,"  gateway %s",addr) && addr[0] != '\0'){
					break;
				}
        	}else if(!strcmp(field,"netmask")){
				if(sscanf (buf,"  netmask %s",addr) && addr[0] != '\0'){
					break;
				}
        	}
        }
    }

    if(addr[0] != '\0' && inet_aton(addr, &inp)){
		inp.s_addr = ntohl(inp.s_addr);
	}

    fclose(fp);
    if (line)
        free(line);

	return inp.s_addr;
}

bool read_conf(char *buf){
	FILE * fp;
	int i = 0, j = 0;
	char *line  = NULL;
	ssize_t len = 0, read, fsize, result;

	fp = fopen("/root/ntp.conf", "r");
	if (fp == NULL)
		printf("cant open file\n");

	fseek(fp , 0 , SEEK_END);
	fsize = ftell(fp);
	rewind(fp);

	result = fread(buf,1,fsize, fp);
	if (result != fsize) {fputs ("Reading error",stderr); return false;}

	fclose (fp);
	return true;
}

uint32_t get_ntp_cfg_ip(void){
    regex_t    preg;
    char       *pattern = "\\b[[:digit:]]{3}.[[:digit:]]{3}.[[:digit:]]{3}.[[:digit:]]{3}\\b";

	int        rc;
	size_t     nmatch = 2;
	regmatch_t pmatch[2];
	struct in_addr inp = {};

	char buf[400];
	read_conf(buf);

	if (0 != (rc = regcomp(&preg, pattern, REG_EXTENDED|REG_NEWLINE))) {
		printf("regcomp() failed, returning nonzero (%d)\r\n", rc);
	}

	if (0 != (rc = regexec(&preg, buf, nmatch, pmatch, 0))) {
		printf("Failed to match '%s' with '%s',returning %d.\r\n",
				buf, pattern, rc);
	}
	else {
		printf("With the whole expression, "
				"a matched substring \"%.*s\" is found at position %d to %d.\r\n",
				pmatch[0].rm_eo - pmatch[0].rm_so, &buf[pmatch[0].rm_so],
				pmatch[0].rm_so, pmatch[0].rm_eo - 1);
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

int set_ntp_cfg_ip(uint32_t ip_addr){
   regex_t    preg;
   char       *pattern = "\\b[[:digit:]]{3}.[[:digit:]]{3}.[[:digit:]]{3}.[[:digit:]]{3}\\b";
   int        rc;
   size_t     nmatch = 2;
   regmatch_t pmatch[2];
   struct in_addr addr;
   addr.s_addr = ip_addr;
   char *ntp_ip = NULL;

   ntp_ip = inet_ntoa(addr);

   char buf[400];
   read_conf(buf);
   printf("%s",buf);

   if (0 != (rc = regcomp(&preg, pattern, REG_EXTENDED|REG_NEWLINE))) {
      printf("regcomp() failed, returning nonzero (%d)\r\n", rc);
   }

   if (0 != (rc = regexec(&preg, buf, nmatch, pmatch, 0))) {
      printf("Failed to match '%s' with '%s',returning %d.\r\n",
             buf, pattern, rc);
   }
   else {
      printf("With the whole expression, "
             "a matched substring \"%.*s\" is found at position %d to %d.\r\n",
             pmatch[0].rm_eo - pmatch[0].rm_so, &buf[pmatch[0].rm_so],
             pmatch[0].rm_so, pmatch[0].rm_eo - 1);
   }
   regfree(&preg);

   memcpy(&buf[pmatch[0].rm_so], ntp_ip, strlen(ntp_ip));

   //printf("%s",buf);
   FILE * fp;
   fp = fopen("/root/ntp.conf", "w");
   if (fp == NULL)
	   perror("Cannot open file");
   fprintf(fp,"%s",buf);
   fflush(fp);

   fclose(fp);

   return 0;
}

