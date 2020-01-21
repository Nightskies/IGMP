#ifndef _HOST_H_
#define _HOST_H_

#include "igmpv2.h"
#include <sys/poll.h>

struct node
{
    uint32_t group;
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
    bool timer_status;
    type_query type;
};

typedef struct delayed_send
{
    struct pollfd * fds; // contains timer descriptors
    int reports; // current number of reports
    int n; // number of reports
}delay;

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

// get ip interface by name
uint32_t get_ip_if_by_name(const char * name);

// wait for the timers to end and send a report
void act_time(struct host * head, delay * _delay);

// event handling
void act_menu(struct host * head);

// number of groups
int num_group(struct host * _host);

// parse string to ip
uint32_t parse_to_ip(const char * address);

// parse ip to string
char * parse_to_str(uint32_t ip);

#endif // _HOST_H