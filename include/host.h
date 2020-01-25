#ifndef _HOST_H_
#define _HOST_H_

#include "igmpv2.h"
#include <sys/poll.h>

enum state { NOT_SET, SET }; // timer state

typedef struct delay_send
{
    struct pollfd * fds; // contains timer descriptors
    bool timers_status; // at least one timer works
    int n; // number of reports
    int reports; // current number of reports
}delay;

struct node
{
    enum state timer_state; // status timer
    uint32_t group; // ip group
    int id;
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
    delay * _delay;
};

// put group in list
void push(struct host * head, struct node * data);

// remove group from list
void pop(struct host * head, uint32_t group);

// search for a group in a list
struct group_list * find_by_group(struct host * _host, uint32_t group);

// search for a id in a list
struct group_list * find_by_id(struct host * _host, int id);

// init host
struct host * init_host(int argc, char ** argv);

// add group to host
void set_group(uint32_t group_ip, struct host * head);

// receive socket  bind to interface by name and return ip
uint32_t if_bind(const char * name);

// wait for the timers to end and send a report
void act_timer(struct host * head);

// event handling
void act_menu(struct host * head);

// number of groups
int num_group(struct host * _host);

// parse string to ip
uint32_t parse_to_ip(const char * address);

// parse ip to string
char * parse_to_str(uint32_t ip);

#endif // _HOST_H