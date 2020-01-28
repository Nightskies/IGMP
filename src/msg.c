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

int accept_query(struct host * _host)
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
        int n, tfd;

        if ((n = num_group(_host)) == 0)
            return false;

        if (Debug)
        {
            printf(STYLE_GREEN_BOLD "\naccept <general query>" STYLE_RESET);
            fflush(stdout);
        }
        
        struct itimerspec ts;
        memset(&ts, 0, sizeof(struct itimerspec));

        if (!_host->_delay->timers_status)
            _host->_delay->timers_status = true;

        struct group_list * next = NULL;

        struct pollfd_list * nextfd = _host->_delay->fds;

        for (next = _host->head; next != NULL; next = next->next, nextfd = nextfd->next)
        {
            if (next->data->timer_state == NOT_SET)
            {
                if (-1 == (tfd = timerfd_create(CLOCK_MONOTONIC, 0)))
                    SYS_ERROR("timerfd_create");
	            
                ts.it_value.tv_sec = get_rand_num(igmp_hdr->code);
                _host->_delay->reports++;

                if (timerfd_settime(tfd, 0, &ts, NULL) < 0)
                    SYS_ERROR("timerfd_settime");

                if (Debug)
                {
                    printf(STYLE_GREEN_BOLD "\nGroup[%s] set timer = %ld sec" STYLE_RESET, 
                        parse_to_str(next->data->group), ts.it_value.tv_sec);
                    fflush(stdout);
                }

                nextfd->data->fd.fd = tfd;
                nextfd->data->fd.events = POLLIN;

                next->data->timer_state = SET;
            }
            else
            {
                int max_resp_time = igmp_hdr->code / 10; // in sec
                struct pollfd_list * elem = NULL;
                elem = find_fd_by_id(_host->_delay, next->data->id);
                tfd = elem->data->fd.fd;

                if (-1 == timerfd_gettime(tfd, &ts))
                    SYS_ERROR("timerfd_gettime");

                if (max_resp_time < ts.it_value.tv_sec)
                {
                    ts.it_value.tv_sec = get_rand_num(igmp_hdr->code);
                    if (-1 == timerfd_settime(tfd, 0, &ts, NULL)) 
                        SYS_ERROR("timerfd_settime");
                }
            }        
        }
         return true;
        
    }

    // General specific query
    else 
    {
        struct group_list * next = NULL;

        if (next = find_by_group(_host, ip_hdr->daddr))
        {
            if (Debug)
            {
                printf(STYLE_GREEN_BOLD "\naccept <specific query>\n" STYLE_RESET);
                fflush(stdout);
            }

            struct itimerspec ts;
            memset(&ts, 0, sizeof(struct itimerspec));

            if (!_host->_delay->timers_status)
                _host->_delay->timers_status = true;

            struct pollfd_list * nextfd = _host->_delay->fds;

            int tfd;

            if (next->data->timer_state == NOT_SET)
            {
                if (-1 == (tfd = timerfd_create(CLOCK_MONOTONIC, 0)))
                    SYS_ERROR("timerfd_create");

                ts.it_value.tv_sec = get_rand_num(igmp_hdr->code);
                _host->_delay->reports++;

                if (-1 == timerfd_settime(tfd, 0, &ts, NULL)) 
                    SYS_ERROR("timerfd_settime");

                if (Debug)
                {
                    printf(STYLE_GREEN_BOLD "\nGroup[%s] set timer = %ld sec" STYLE_RESET, 
                        parse_to_str(next->data->group), ts.it_value.tv_sec);
                    fflush(stdout);
                }
                
                nextfd->data->fd.fd = tfd;
                nextfd->data->fd.events = POLLIN;

                next->data->timer_state = SET;
            }
            else
            {   
                int max_resp_time = igmp_hdr->code / 10; // in sec
                struct pollfd_list * elem = NULL;
                elem = find_fd_by_id(_host->_delay, next->data->id);
                tfd = elem->data->fd.fd;

                if (-1 == timerfd_gettime(tfd, &ts))
                    SYS_ERROR("timerfd_gettime");

                if (max_resp_time < ts.it_value.tv_sec)
                {
                    ts.it_value.tv_sec = get_rand_num(igmp_hdr->code);
                    if (-1 == timerfd_settime(tfd, 0, &ts, NULL)) 
                        SYS_ERROR("timerfd_settime");
                }
            }
            return true;
        }
    }
    return false;
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

    struct group_list * elem = NULL;
    elem = find_by_group(_host, group);
    pop_fd(_host->_delay, elem->data->id);

    pop(_host, group);
    free(packet);
}