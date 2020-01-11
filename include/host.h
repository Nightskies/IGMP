#ifndef _HOST_H_
#define _HOST_H_

#include "igmpv2.h"

struct node
{
    uint32_t group;
    int timer;
};

struct group_list 
{
    struct node * data;
    struct group_list * next;
};

struct host 
{
    uint32_t if_addr;
    struct group_list * head;
};

// push group 
void push(struct host * head, struct node * data);

// pop group
void pop(struct host * head, uint32_t group);

// init host
struct host * init_host(int argc, char ** argv);

// init group
void set_group(const char * group_ip, struct host * head);

// get ip interface by name
uint32_t get_ip_if_by_name(const char * name);

#endif // _HOST_H