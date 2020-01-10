#ifndef _IGMPV2_H_
#define _IGMPV2_H_

#include <stdint.h>
#include <stdbool.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/igmp.h>

#include <sys/ioctl.h>
#include <net/if.h>

#include <string.h>
#include <time.h>

#include "error.h"

#define MIN_IGMPV2_LEN 16
#define MIN_IP_LEN 20
#define RAOPT_LEN 4

#define BUF_SIZE BUFSIZ

typedef struct iphdr ip;
typedef struct igmphdr igmp;

const char * build_packet(const uint32_t src, int type, const uint32_t group);

uint16_t build_csum_igmp(uint16_t * addr, int len);

uint32_t parse_to_ip(const char * address);

uint32_t timer(const unsigned char max_res_time);

#endif // _IGMPV2_H_
