#include "../include/socket.h"

int ssfd, rsfd;

void init_sock(void)
{
    int val = 1;

    // send socket
    if (-1 == (ssfd = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP)))
        SYS_ERROR("socket");

    // recv socket
    if (-1 == (rsfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))))
        SYS_ERROR("socket");

    // ip hdr
    if (-1 == setsockopt(ssfd, IPPROTO_IP, IP_HDRINCL, &val, sizeof(val)))
        SYS_ERROR("setsockopt");
}