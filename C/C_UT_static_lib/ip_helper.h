#ifndef __IP_HELPER_H__
#define __IP_HELPER_H__

#include <stdint.h>

int ip_range_invalid(uint32_t address);
int subnet_range_valid(uint8_t subnet);
int is_ip_network_or_broadcast(uint32_t addr, uint32_t netmask);
uint8_t convert_mask_to_subnet(uint32_t mask);
uint32_t convert_ip_to_ulong(const char* cp);
uint32_t is_private_ip(uint32_t ip);
uint32_t parse_interface_file(const char *field);
uint32_t get_ntp_cfg_ip(void);
int set_ntp_cfg_ip(uint32_t ip_addr);

#endif
