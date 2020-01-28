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

uint32_t if_bind(const char * name)
{
    struct ifreq ifr;
    size_t length = strlen(name);

    struct sockaddr_ll addr;

    if (length > sizeof(ifr.ifr_name))
        ERROR("Interface name is too big");

    memcpy(ifr.ifr_name, name, length);
    ifr.ifr_name[length] = '\0';
    
    if (-1 == ioctl(ssfd, SIOCGIFADDR, &ifr)) 
        SYS_ERROR("ioctl");

    if (-1 == ioctl(rsfd, SIOCGIFINDEX, &ifr))
        SYS_ERROR("ioctl");

    addr.sll_family = PF_PACKET;
    addr.sll_ifindex = ifr.ifr_ifindex;

    if (-1 == bind(rsfd, (struct sockaddr*)&addr, sizeof(addr)))
        SYS_ERROR("bind");
    
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

struct host * init_host(int argc, char ** argv)
{
    printf (STYLE_GREEN_BOLD "Start init host..." STYLE_RESET);

    struct host * _host = (struct host *)malloc(sizeof(struct host));
    if (_host == NULL)
        ERROR("malloc returned Null");

    memset(_host, 0, sizeof(struct host));

    init_sock();

    struct group_list * head = NULL;

    _host->if_addr = if_bind(argv[argc - 1]);

    _host->if_name = argv[argc - 1];

    _host->_delay = (delay *)malloc(sizeof(delay));
    if (_host->_delay == NULL)
        ERROR("malloc returned Null");

    memset(_host->_delay, 0, sizeof(delay));

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
    static int id = 0;

    if (Debug)
    {
        printf(STYLE_GREEN_BOLD "\nSet group[%s]" STYLE_RESET, parse_to_str(group_ip));
        fflush(stdout);
    }

    struct node * data = (struct node *)malloc(sizeof(struct node));
    if (data == NULL)
        ERROR("malloc returned Null");

    memset(data, 0, sizeof(struct node));

    data->group = group_ip;
    data->id = id;

    push(head, data);

    struct pollfd_node * node = (struct pollfd_node *)malloc(sizeof(struct pollfd_node));
    if (node == NULL)
        ERROR("malloc returned Null");

    memset(node, 0, sizeof(struct pollfd_node));
    node->id = id++;

    push_fd(head->_delay, node);

    if (Debug)
    {
        printf(STYLE_GREEN_BOLD "\nAdded group[%s] to list" STYLE_RESET, parse_to_str(group_ip));
        fflush(stdout);
    }
}

void act_timer(struct host * _host)
{
    struct pollfd_list * next = NULL;
    for (next = _host->_delay->fds; next != NULL; next = next->next)
    {  
        switch (poll(&next->data->fd, 1, 0))
        {
            case -1:
                SYS_ERROR("poll");

            case 0:
                break;
    
            default:
                if (next->data->fd.revents & POLLIN)
                {
                    read(next->data->fd.fd, NULL, 8);
                    next->data->fd.revents = 0;

                    struct group_list * group = NULL;
                    group = find_by_id(_host, next->data->id);

                    send_membership_report(_host->if_addr, group->data->group);
                    _host->_delay->reports--;
                    group->data->timer_state = NOT_SET;

                    if (_host->_delay->reports == 0)
                    {
                        _host->_delay->n = 0;
                        _host->_delay->timers_status = false;
                        printf(STYLE_YELLOW_BOLD "\nEnter command > " STYLE_RESET);
                        fflush(stdout);
                    }
                }
                break;
        }
    }
}

void act_menu(struct host * _host)
{
    printf(STYLE_YELLOW_BOLD "\nEnter command > " STYLE_RESET);
    fflush(stdout);

    char ** args;

    char line[BUFSIZE];

    int n;
    
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
                            free(args);
                            exit(EXIT_SUCCESS);
                        }
                    }
                }

                if (fds[1].revents & POLLIN)
                {
                    fds[1].revents = 0;

                    if (accept_query(_host))
                       act_timer(_host);
                        
                    else if (_host->_delay->timers_status)
                        act_timer(_host);
                              
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