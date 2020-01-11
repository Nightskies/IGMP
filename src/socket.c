#include "../include/socket.h"

int sfd;

void init_sock(void)
{
    int val = 1;

    if (-1 == (sfd = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP)))
        SYS_ERROR("socket");

    /* ip hdr */
    if (-1 == setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, &val, sizeof(val)))
        SYS_ERROR("setsockopt");
}