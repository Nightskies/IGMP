#include "../include/host.h"
#include "../include/msg.h"
#include "../include/socket.h"
#include "../include/interpreter.h"

void push(struct host * _host, struct node * data)
{
    struct group_list * tmp = (struct group_list *)malloc(sizeof(struct group_list));
    if (tmp == NULL)
        ERROR("malloc returned Null");

    tmp->data = data;
    tmp->next = NULL;

    if (_host->head) 
    {
        struct group_list * p = _host->head;

        while (p->next)
            p = p->next;

        p->next = tmp;
    } 
    else
        _host->head = tmp;
}

void pop(struct host * _host, uint32_t group)
{
    struct group_list * head = NULL;
    head = _host->head;

    if (head->data->group == group)
    {
        if (Debug)
        {
            printf(STYLE_BLUE_BOLD "\ngroup[%s] is removed from the list" STYLE_RESET, parse_to_str(head->data->group));
            fflush(stdout);
        }

        head = head->next;
        free(_host->head->data);
        free(_host->head);
        _host->head = head;          
        return;        
    }

    while (head->next->data->group != group)
        head = head->next;

    if (head)
    {
        if (Debug)
            printf(STYLE_BLUE_BOLD "\ngroup[%s] is removed from the list" STYLE_RESET, parse_to_str(head->next->data->group));

        struct group_list * p = head->next;
        head->next = p->next;
        free(p->data);
        free(p); 
    }
}

struct group_list * find_by_group(struct host * _host, uint32_t group)
{
    struct group_list * head = NULL;
    head = _host->head;

	while (head && (head->data->group != group))
		head = head->next;

    if (head)
        return head;

	return NULL;
}

struct group_list * find_by_id(struct host * _host, int id)
{
    struct group_list * head = NULL;
    head = _host->head;

	while (head && (head->data->id != id))
		head = head->next;

    if (head)
        return head;

	return NULL;
}

uint32_t get_ip_if_by_name(const char * name)
{
    struct ifreq ifr;
    size_t length = strlen(name);

    if (length > sizeof(ifr.ifr_name))
        ERROR("Interface name is too big");

    memcpy(ifr.ifr_name, name, length);
    ifr.ifr_name[length] = '\0';
    
    if (-1 == ioctl(ssfd, SIOCGIFADDR, &ifr)) 
        SYS_ERROR("ioctl");
    
    struct sockaddr_in * ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;

    printf(STYLE_GREEN_BOLD "\nINTERFACE [%s]" STYLE_RESET, inet_ntoa(ipaddr->sin_addr));

    struct in_addr if_addr;
    memset(&if_addr, 0, sizeof(struct in_addr));
     
    if_addr = ipaddr->sin_addr;

    return if_addr.s_addr;
}

int num_group(struct host * _host)
{
    struct group_list * head = NULL;
    int num = 0;

    head = _host->head;

    while(head)
    {
        num++;
        head = head->next;
    }

    return num;
}

struct host * init_host(int argc, char **argv)
{
    printf (STYLE_GREEN_BOLD "Start init host..." STYLE_RESET);

    struct host * _host = (struct host *)malloc(sizeof(struct host));
    if (_host == NULL)
        ERROR("malloc returned Null");

    init_sock();

    struct group_list * head = NULL;

    _host->if_addr = get_ip_if_by_name(argv[argc - 1]);

    _host->if_name = argv[argc - 1];

    _host->timer_status = false;

    struct in_addr addr;
    uint32_t group_ip;
    int i = 1;
    
    if (Debug)
        i = 2;

    for (; i < argc - 1; i++)
        if (group_ip = parse_to_ip(argv[i]))
            set_group(group_ip, _host);
        else
        {
            printf(STYLE_RED_BOLD "This's not a multicast ip[%s]\n" STYLE_RESET, argv[i]);
            exit(EXIT_FAILURE);
        }

    head = _host->head;

    while(head)
    {
        addr.s_addr = head->data->group;
        send_membership_report(_host->if_addr, head->data->group);
        head = head->next;
    }

    printf (STYLE_GREEN_BOLD "\nInit host successful..." STYLE_RESET);

    return _host;
}

void set_group(uint32_t group_ip, struct host * head)
{   
    if (Debug)
    {
        printf(STYLE_GREEN_BOLD "\nSet group[%s]" STYLE_RESET, parse_to_str(group_ip));
        fflush(stdout);
    }

    struct node * data = (struct node *)malloc(sizeof(struct node));
    if (data == NULL)
        ERROR("malloc returned Null");

    data->group = group_ip;

    push(head, data);

    if (Debug)
    {
        printf(STYLE_GREEN_BOLD "\nAdded group[%s] to list" STYLE_RESET, parse_to_str(group_ip));
        fflush(stdout);
    }
}

void act_time(struct host * _host, delay * _delay)
{
    switch (poll(_delay->fds, _delay->n, 0))
    {
        case -1:
            SYS_ERROR("poll");

        case 0:
            break;
    
        default:
            for (int i = 0; i < _delay->n; i++)
            {
                if ( _delay->fds[i].revents & POLLIN)
                {
                    read(_delay->fds[i].fd, NULL, 8);
                     _delay->fds[i].revents = 0;
                    struct group_list * group = NULL;
                    group = find_by_id(_host, i);

                    send_membership_report(_host->if_addr, group->data->group);
                     _delay->reports--;

                    if ( _delay->reports == 0)
                    {
                        _host->timer_status = false;
                        printf(STYLE_YELLOW_BOLD "\nEnter command > " STYLE_RESET);
                        fflush(stdout);
                    }
                }
            }
            break;
    }
}

void act_menu(struct host * _host)
{
    printf(STYLE_YELLOW_BOLD "\nEnter command > " STYLE_RESET);
    fflush(stdout);

    char ** args;

    delay _delay;

    int i;

    struct pollfd * tfd = NULL;

    char line[BUFSIZE];
    
    struct pollfd fds[2];

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
 
    fds[1].fd = rsfd;
    fds[1].events = POLLIN;

    while(true)
    {
        switch (poll(fds, 2, 0))
        {
            case -1:
                SYS_ERROR("poll");

            case 0:
                break;
     
            default:
                if (fds[0].revents & POLLIN)
                {   
                    fds[0].revents = 0;
                    if (read_line(line) == 0)
                    {
		                args = parse_line(line);

                        if (exe(args, _host))
                        {   
                            printf(STYLE_YELLOW_BOLD "\nEnter command > " STYLE_RESET);
                            fflush(stdout);
                        }
                        else
                        {
                            free(_delay.fds);
                            free(args);
                            exit(EXIT_SUCCESS);
                        }
                    }
                }

                if (fds[1].revents & POLLIN)
                {
                    fds[1].revents = 0;

                    if (_host->timer_status)
                        act_time(_host, &_delay);

                    else if (tfd = accept_query(_host))
                    {
                        _delay.fds = tfd;
                        if (_host->type == general)
                        {
                            _delay.reports = num_group(_host);
                            _delay.n = _delay.reports;
                        }
                        else
                        {
                            _delay.reports = 1;
                            _delay.n = _delay.reports;
                        }
                        act_time(_host, &_delay);
                    }
                }
        } 
    }
}

uint32_t parse_to_ip(const char * address)
{
	uint32_t s_addr;

	if (inet_pton(AF_INET, address, &s_addr) < 0)
        SYS_ERROR("inet_pton");

    if (IN_MULTICAST(ntohl(s_addr)))
	    return s_addr;
    
    else
        return 0;  
}

char * parse_to_str(uint32_t ip)
{
    struct in_addr addr;
    addr.s_addr = ip;

    return inet_ntoa(addr);
}