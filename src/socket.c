#include "../include/socket.h"

int sfd;

void init_sock(struct host * _host)
{
    int val = 1;

    struct sockaddr_in _addr;
    memset(&_addr, 0, sizeof(struct sockaddr_in));

    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);


    if (-1 == (sfd = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP)))
        fatal("init_sock: socket");

    if (-1 == bind(sfd, (struct sockaddr *)&_addr, sizeof(_addr)))
        fatal("init_sock: bind");

    /* ip hdr */
    if (-1 == setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, &val, sizeof(val)))
        fatal("init_sock: setsockopt");

    /*
    if (-1 == setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)))
        fatal("setsockopt");
    */ 

    struct in_addr addr;
    memset(&addr, 0 , sizeof(struct in_addr));

    addr.s_addr = _host->if_addr;

    if (-1 == setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_IF, &addr, sizeof(addr)))
        fatal("setsockopt");
}