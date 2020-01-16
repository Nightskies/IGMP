#include "../include/msg.h"
#include "../include/socket.h"

void send_membership_report(const uint32_t src, const uint32_t group)
{
    char * packet = NULL;

    struct sockaddr_in dst_addr;
    memset(&dst_addr, 0 , sizeof(struct sockaddr_in));
    
    packet = build_packet(src, IGMPV2_HOST_MEMBERSHIP_REPORT, group);
    
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = group;

    if (-1 == sendto(sfd, packet, MIN_IP_LEN + RAOPT_LEN + MIN_IGMPV2_LEN, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr))) 
        SYS_ERROR("sendto");

    printf(STYLE_GREEN_BOLD "sent <membership report> to [%s]\n" STYLE_RESET, inet_ntoa(dst_addr.sin_addr));
    
    free(packet);
}

void accept_query(struct host * _host)
{
    char * packet = (char *)malloc(BUF_SIZE);
    if (packet == NULL)
        ERROR("malloc returned Null");

    socklen_t socklen = sizeof(struct sockaddr_in);

    ssize_t nbytes;

    ip * ip_hdr;
    igmp * igmp_hdr;

    if (-1 == (nbytes = recv(sfd, packet, BUF_SIZE, 0)))
        SYS_ERROR("recevfrom");

    ip_hdr = (ip *)packet;

    struct in_addr ip_saddr, ip_daddr;
    ip_saddr.s_addr = ip_hdr->saddr;
    ip_daddr.s_addr = ip_hdr->daddr;

    igmp_hdr = (igmp *)(packet + MIN_IP_LEN + RAOPT_LEN);

    // General query
    if (ip_hdr->daddr == parse_to_ip(ALLHOSTS_GROUP))
    {
        printf(STYLE_GREEN_BOLD "accept <general query>\n" STYLE_RESET);
        struct group_list * next = NULL;

        for (next = _host->head; next != NULL; next = next->next)
        {
                next->data->timer = timer(igmp_hdr->code);
                printf(STYLE_GREEN_BOLD "Group[%s] set time = %u" STYLE_RESET, 
                    parse_to_str(next->data->group), next->data->timer);

                // timer
                switch(fork())
                {
                    case -1:
                        SYS_ERROR("fork");
                    
                    case 0:
                        usleep(1000 * next->data->timer);
                        send_membership_report(_host->if_addr, next->data->group);
                        next->data->timer = 0;
                        exit(EXIT_SUCCESS);

                    default:
                        free(packet);
                        break;
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
                printf(STYLE_GREEN_BOLD "accept <specific query>\n" STYLE_RESET);

                next->data->timer = timer(igmp_hdr->code);
                printf(STYLE_GREEN_BOLD "Group[%s] set time = %u" STYLE_RESET, 
                    parse_to_str(next->data->group), next->data->timer);

                switch(fork())
                {
                    case -1:
                        SYS_ERROR("fork");
                    
                    case 0:
                        usleep(1000 * next->data->timer);
                        send_membership_report(_host->if_addr, next->data->group);
                        next->data->timer = 0;
                        exit(EXIT_SUCCESS);

                    default:
                        free(packet);
                        flag = false;
                        break;
                }
            }
        }
    } 
}

void send_leave_group(struct host * _host, const uint32_t group)
{
    char * packet = NULL;

    struct sockaddr_in dst_addr;
    memset(&dst_addr, 0 , sizeof(struct sockaddr_in));
    
    packet = build_packet(_host->if_addr, IGMP_HOST_LEAVE_MESSAGE, group);

    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = parse_to_ip(ALLRTRS_GROUP);

    if (-1 == sendto(sfd, packet, MIN_IP_LEN + RAOPT_LEN + MIN_IGMPV2_LEN, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr)))
        SYS_ERROR("sendto");
    
    printf(STYLE_GREEN_BOLD "sent <leave group> to [%s]\n" STYLE_RESET , ALLRTRS_GROUP);

    pop(_host, group);
    free(packet);
}