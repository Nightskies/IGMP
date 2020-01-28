#ifndef _DELAY_H_
#define _DELAY_H_

#include "igmpv2.h"
#include <sys/poll.h>

struct pollfd_node
{
    struct pollfd fd;
    int id;
};

struct pollfd_list 
{
    struct pollfd_node * data;
    struct pollfd_list * next;
};

typedef struct delay_send
{
    struct pollfd_list * fds; // contains timer descriptors
    bool timers_status; // at least one timer works
    int reports; // current number of reports
}delay;

void push_fd(delay * _delay, struct pollfd_node * data);

struct pollfd_list * find_fd_by_id(delay * _delay, int id);

void pop_fd(delay * _delay, int id);

#endif // _DELAY_H_