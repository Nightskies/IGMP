#include "../include/host.h"
#include "../include/msg.h"
#include "../include/socket.h"

void push(struct host * _host, struct node * data)
{
    struct group_list * tmp = (struct group_list *)malloc(sizeof(struct group_list));
    if (tmp == NULL)
    {
        fprintf(stderr, "push: Error malloc \n");
        exit(EXIT_FAILURE);
    }

    tmp->data = data;
    tmp->next = NULL;

    if (_host->head) 
    {
        struct group_list * p = _host->head;

        while (p->next != NULL)
            p = p->next;

        p->next = tmp;
    } 
    else
        _host->head = tmp;
}

void pop(struct host * _host, uint32_t group)
{
  while (_host->head->next != NULL && _host->head->next->data->group != group)
    _host->head = _host->head->next;

  if (_host->head->next) 
  {
    struct group_list * p = _host->head->next;
    _host->head->next = p->next;
    free(p->data);
    free(p);
  }
}

uint32_t get_ip_if_by_name(const char * name)
{
    struct ifreq ifr;
    size_t length = strlen(name);

    if(length > sizeof(ifr.ifr_name))
    {
        printf("Error get_ip_if_by_name \n");
        exit(EXIT_FAILURE);
    }

    memcpy(ifr.ifr_name, name, length);
    ifr.ifr_name[length] = '\0';
    
    if (-1 == ioctl(sfd, SIOCGIFADDR, &ifr)) 
        fatal("get_ip_if_by_name: ioctl");
    
    struct sockaddr_in * ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;

    printf("IP address: %s\n",inet_ntoa(ipaddr->sin_addr));

    struct in_addr if_addr;
    memset(&if_addr, 0, sizeof(struct in_addr));
     
    if_addr = ipaddr->sin_addr;

    return if_addr.s_addr;
}

struct host * init_host(int argc, char **argv)
{
    struct host * _host = (struct host *)malloc(sizeof(struct host));
    if (_host == NULL)
    {
        fprintf(stderr, "init_host: Error malloc \n");
        exit(EXIT_FAILURE);
    }

    init_sock(_host);
    
    _host->if_addr = get_ip_if_by_name(argv[argc - 1]);

    for (int i = 1; i < argc - 1; i++)
    {
        set_group(argv[i], _host);
        send_membership_report(_host->if_addr, _host->head->data->group);
        _host->head = _host->head->next;
    }

    return _host;
}

void set_group(const char * group_ip, struct host * head)
{
    struct node * data = (struct node *)malloc(sizeof(struct node));
    if (data == NULL)
    {
        fprintf(stderr, "init_group: Error malloc\n");
        exit(EXIT_FAILURE);
    }

    data->group = parse_to_ip(group_ip);
    data->timer = 0;

    push(head, data);
}