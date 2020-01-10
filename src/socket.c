#include "../include/socket.h"

int sfd;

void init_sock(struct host * _host)
{
    int val = 1;

    if (-1 == (sfd = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP)))
        fatal("init_sock: socket");

    /* ip hdr */
    if (-1 == setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, &val, sizeof(val)))
        fatal("init_sock: setsockopt");
}