#include "../include/msg.h"
#include "../include/socket.h"
#include <sys/timerfd.h>

void send_membership_report(const uint32_t src, const uint32_t group)
{
    char * packet = NULL;

    struct sockaddr_in dst_addr;
    memset(&dst_addr, 0 , sizeof(struct sockaddr_in));
    
    packet = build_packet(src, IGMPV2_HOST_MEMBERSHIP_REPORT, group);
    
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = group;

    if (-1 == sendto(ssfd, packet, MIN_IP_LEN + RAOPT_LEN + MIN_IGMPV2_LEN, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr))) 
        SYS_ERROR("sendto");

    printf(STYLE_GREEN_BOLD "\nsent <membership report> to [%s]" STYLE_RESET, inet_ntoa(dst_addr.sin_addr));
    fflush(stdout);
    
    free(packet);
}

struct pollfd * accept_query(struct host * _host)
{
    char packet[BUF_SIZE];
    memset(packet, 0, BUF_SIZE);

    socklen_t socklen = sizeof(struct sockaddr_in);

    ssize_t nbytes;

    ip * ip_hdr;
    igmp * igmp_hdr;

    if (-1 == (nbytes = recv(rsfd, packet, BUF_SIZE, 0)))
        SYS_ERROR("recevfrom");

    ip_hdr = (ip *)(packet + sizeof(eth));

    struct in_addr ip_saddr, ip_daddr;
    ip_saddr.s_addr = ip_hdr->saddr;
    ip_daddr.s_addr = ip_hdr->daddr;

    igmp_hdr = (igmp *)(packet + sizeof(eth) + MIN_IP_LEN + RAOPT_LEN);

    // General query
    if (ip_hdr->daddr == parse_to_ip(ALLHOSTS_GROUP))
    {
        if (Debug)
        {
            printf(STYLE_GREEN_BOLD "\naccept <general query>" STYLE_RESET);
            fflush(stdout);
        }
        
        struct itimerspec ts;
        memset(&ts, 0, sizeof(struct itimerspec));

        struct group_list * next = NULL;
        int n, i;

        n = num_group(_host);
        if (n == 0)
            return NULL;

        struct pollfd * fds = (struct pollfd *)malloc(n * sizeof(struct pollfd));
        int tfd[n];

        for (next = _host->head, i = 0; next != NULL; next = next->next, i++)
        {
            if (-1 == (tfd[i] = timerfd_create(CLOCK_MONOTONIC, 0)))
                SYS_ERROR("timerfd_create");
	            
            ts.it_value.tv_sec = get_rand_num(igmp_hdr->code);
            next->data->id = i;

            if (timerfd_settime(tfd[i], 0, &ts, NULL) < 0) 
                SYS_ERROR("timerfd_settime");

            if (Debug)
            {
                printf(STYLE_GREEN_BOLD "\nGroup[%s] set timer = %ld sec" STYLE_RESET, 
                    parse_to_str(next->data->group), ts.it_value.tv_sec);
                fflush(stdout);
            }
            fds[i].fd = tfd[i];
            fds[i].events = POLLIN;
        }
        _host->timer_status = true;
        _host->type = general;
        
        return fds;
    }

    // General specific query
    else 
    {
        struct group_list * next = NULL;

        if (next = find_by_group(_host, igmp_hdr->group))
        {
            bool flag = true;

            struct itimerspec ts;
            memset(&ts, 0, sizeof(struct itimerspec));

            if (Debug)
            {
                printf(STYLE_GREEN_BOLD "\naccept <specific query>\n" STYLE_RESET);
                fflush(stdout);
            }

            struct pollfd * fds = (struct pollfd *)malloc(sizeof(struct pollfd));
            int tfd;
                
            if (-1 == (tfd = timerfd_create(CLOCK_MONOTONIC, 0)))
                SYS_ERROR("timerfd_create");

            ts.it_value.tv_sec = get_rand_num(igmp_hdr->code);

            if (timerfd_settime(tfd, 0, &ts, NULL) < 0) 
                SYS_ERROR("timerfd_settime");

            if (Debug)
            {
                printf(STYLE_GREEN_BOLD "\nGroup[%s] set timer = %ld sec" STYLE_RESET, 
                    parse_to_str(next->data->group), ts.it_value.tv_sec);
                fflush(stdout);
            }
            fds->fd = tfd;
            fds->events = POLLIN;  

            _host->timer_status = true;
            _host->type = specific;

            return fds;    
        }
    }
    return NULL;
}

void send_leave_group(struct host * _host, const uint32_t group)
{
    char * packet = NULL;

    struct sockaddr_in dst_addr;
    memset(&dst_addr, 0 , sizeof(struct sockaddr_in));
    
    packet = build_packet(_host->if_addr, IGMP_HOST_LEAVE_MESSAGE, group);

    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = parse_to_ip(ALLRTRS_GROUP);

    if (-1 == sendto(ssfd, packet, MIN_IP_LEN + RAOPT_LEN + MIN_IGMPV2_LEN, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr)))
        SYS_ERROR("sendto");
    
    printf(STYLE_GREEN_BOLD "\nsent <leave group> to [%s]" STYLE_RESET , ALLRTRS_GROUP);

    pop(_host, group);
    free(packet);
}