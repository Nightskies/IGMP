#include "../include/host.h"
#include "../include/msg.h"
#include "../include/socket.h"

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
            printf(STYLE_BLUE_BOLD "group[%s] is removed from the list\n" STYLE_RESET, parse_to_str(head->data->group));

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
            printf(STYLE_BLUE_BOLD "group[%s] is removed from the list\n" STYLE_RESET, parse_to_str(head->next->data->group));

        struct group_list * p = head->next;
        head->next = p->next;
        free(p->data);
        free(p); 
    }
}

bool find(struct host * _host, uint32_t group)
{
    struct group_list * head = NULL;
    head = _host->head;

	while (head && (head->data->group != group))
		head = head->next;

    if (head)
        return true;

	return false;
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

    printf(STYLE_GREEN_BOLD "INTERFACE [%s]\n" STYLE_RESET, inet_ntoa(ipaddr->sin_addr));

    struct in_addr if_addr;
    memset(&if_addr, 0, sizeof(struct in_addr));
     
    if_addr = ipaddr->sin_addr;

    return if_addr.s_addr;
}

struct host * init_host(int argc, char **argv)
{
    printf (STYLE_GREEN_BOLD "Start init host...\n" STYLE_RESET);

    struct host * _host = (struct host *)malloc(sizeof(struct host));
    if (_host == NULL)
        ERROR("malloc returned Null");

    init_sock();

    struct group_list * head = NULL;

    _host->if_addr = get_ip_if_by_name(argv[argc - 1]);

    _host->if_name = argv[argc - 1];

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

    printf (STYLE_GREEN_BOLD "Init host successful...\n" STYLE_RESET);

    return _host;
}

void set_group(uint32_t group_ip, struct host * head)
{   
    if (Debug)
        printf(STYLE_GREEN_BOLD "Set group[%s]\n" STYLE_RESET, parse_to_str(group_ip));

    struct node * data = (struct node *)malloc(sizeof(struct node));
    if (data == NULL)
        ERROR("malloc returned Null");

    data->group = group_ip;
    data->timer = 0;

    push(head, data);

    if (Debug)
        printf(STYLE_GREEN_BOLD "Added group[%s] to list\n" STYLE_RESET, parse_to_str(group_ip));
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