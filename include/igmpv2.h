#ifndef _IGMPV2_H_
#define _IGMPV2_H_

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/igmp.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

#include <sys/ioctl.h>
#include <net/if.h>

#include <time.h>

#include "error.h"

#define MIN_IGMPV2_LEN 8
#define MIN_IP_LEN 20
#define RAOPT_LEN 4

#define BUF_SIZE 64

typedef struct iphdr ip;
typedef struct igmphdr igmp;
typedef struct ethhdr eth;

typedef enum type_query { general, specific } type_query;

// build igmp packet
char * build_packet(const uint32_t src, int type, const uint32_t group);

// build check sum igmp
uint16_t build_csum_igmp(uint16_t * addr, int len);

// sets a random number from the range [0, MAX_RESPONSE_TIME]
uint32_t get_rand_num(const unsigned char max_res_time);

#endif // _IGMPV2_H_
