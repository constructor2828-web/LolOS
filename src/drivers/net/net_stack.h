#ifndef NET_STACK_H
#define NET_STACK_H

#include <stdint.h>

int net_dns_resolve(const char* host, uint8_t out_ip[4]);

#endif
