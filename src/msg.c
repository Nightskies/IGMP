#include "../include/msg.h"
#include "../include/socket.h"

void send_membership_report(const uint32_t src, const uint32_t group)
{
    char * packet;

    struct sockaddr_in dst_addr;//
    memset(&dst_addr, 0 , sizeof(struct sockaddr_in));
    
    packet = build_packet(src, IGMPV2_HOST_MEMBERSHIP_REPORT, group);

    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = group;

    if (-1 == sendto(sfd, packet, MIN_IP_LEN + RAOPT_LEN + MIN_IGMPV2_LEN, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr))) 
        fatal("sendto");

    free(packet);
}

void accept_query(struct host * _host)
{
    char * packet = (char *)malloc(BUF_SIZE);
    if(packet == NULL)
    {
        fprintf(stderr, "accept_query: Error malloc \n");
        exit(EXIT_FAILURE);
    }

    socklen_t socklen = sizeof(struct sockaddr_in);

    struct sockaddr_in src_addr;
    memset(&src_addr, 0 , socklen);

    ssize_t nbytes;

    ip * ip_hdr;
    igmp * igmp_hdr;

    if (-1 == (nbytes = recvfrom(sfd, packet, BUF_SIZE, 0, (struct sockaddr *)&src_addr, &socklen)))
        fatal("recevfrom");

    ip_hdr = (ip *)packet;

    struct in_addr ip_saddr, ip_daddr;
    ip_saddr.s_addr = ip_hdr->saddr;
    ip_daddr.s_addr = ip_hdr->daddr;

    printf("ip: PACKETSIZE[%zd], saddr[%s]->daddr[%s]\n", nbytes, inet_ntoa(ip_saddr), inet_ntoa(ip_daddr));

    igmp_hdr = (igmp *)(packet + MIN_IP_LEN + RAOPT_LEN);

    printf("igmp: TYPE[%x], MAX RESPONSE TIME[%d]\n", igmp_hdr->type, igmp_hdr->code);

    // General query
    if (ip_hdr->daddr == INADDR_ALLHOSTS_GROUP)
    {
        struct group_list * next = NULL;

        for (next = _host->head; next != NULL; next = next->next)
        {
            if (next->data->timer == 0)
            {
                next->data->timer = timer(igmp_hdr->code);

                // timer
                switch(fork())
                {
                    case -1:
                        fatal("accept_query: fork");
					    exit(EXIT_FAILURE);
                    
                    case 0:
                        sleep(next->data->timer);
                        send_membership_report(_host->if_addr, next->data->group);
                        next->data->timer = 0;
                        exit(EXIT_SUCCESS);

                    default:
                        break;

                }
            }
            
            else if (next->data->timer < igmp_hdr->code)
            {
                next->data->timer = timer(igmp_hdr->code);
                
                switch(fork())
                {
                    case -1:
                        fatal("accept_query: fork");
					    exit(EXIT_FAILURE);
                    
                    case 0:
                        sleep(next->data->timer);
                        send_membership_report(_host->if_addr, next->data->group);
                        next->data->timer = 0;
                        exit(EXIT_SUCCESS);

                    default:
                        break;

                }
            }
            
        }
        
    }

    // General specific query
    else
    {
        struct group_list * next = NULL;

        bool flag = true;

        for (next = _host->head; next != NULL && flag; next = next->next)
        {
            if (ip_hdr->daddr == next->data->group);
            {
                next->data->timer = timer(igmp_hdr->code);

                switch(fork())
                {
                    case -1:
                        fatal("accept_query: fork");
					    exit(EXIT_FAILURE);
                    
                    case 0:
                        sleep(next->data->timer);
                        send_membership_report(_host->if_addr, next->data->group);
                        next->data->timer = 0;
                        exit(EXIT_SUCCESS);

                    default:
                        flag = false;
                        break;

                }
            }
        }
    } 
}

void send_leave_group(struct host * _host, const uint32_t group)
{
    char * packet;

    struct sockaddr_in dst_addr;
    memset(&dst_addr, 0 , sizeof(struct sockaddr_in));
    
    packet = build_packet(_host->if_addr, IGMP_HOST_LEAVE_MESSAGE, INADDR_ALLRTRS_GROUP);

    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = INADDR_ALLRTRS_GROUP;

    if (-1 == sendto(sfd, packet, MIN_IP_LEN + RAOPT_LEN + MIN_IGMPV2_LEN, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr))) 
        fatal("send_leave_group: sendto");
    
    free(packet);
    pop(_host, group);
}