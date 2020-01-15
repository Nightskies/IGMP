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
    char * if_name; // interface name
    struct group_list * head; // group list
    uint32_t if_addr; // interface ip
};

// put group in list
void push(struct host * head, struct node * data);

// remove group from list
void pop(struct host * head, uint32_t group);

// search for a group in a list
bool find(struct host * _host, uint32_t group);

// init host
struct host * init_host(int argc, char ** argv);

// add group to host
void set_group(uint32_t group_ip, struct host * head);

// get ip interface by name
uint32_t get_ip_if_by_name(const char * name);

// parse string to ip
uint32_t parse_to_ip(const char * address);

// parse ip to string
char * parse_to_str(uint32_t ip);

#endif // _HOST_H