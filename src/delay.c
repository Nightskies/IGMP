#include "../include/delay.h"

void push_fd(delay * _delay, struct pollfd_node * data)
{
    struct pollfd_list * tmp = (struct pollfd_list *)malloc(sizeof(struct pollfd_list));
    if (tmp == NULL)
        ERROR("malloc returned Null");

    tmp->data = data;
    tmp->next = NULL;

    if (_delay->fds) 
    {
        struct pollfd_list * p = _delay->fds;

        while (p->next)
            p = p->next;

        p->next = tmp;
    } 
    else
        _delay->fds = tmp;
}

void pop_fd(delay * _delay, int id)
{
    struct pollfd_list * head = NULL;
    head = _delay->fds;

    if (head->data->id == id)
    {
        head = head->next;
        free(_delay->fds->data);
        free(_delay->fds);
        _delay->fds = head;          
        return;        
    }

    while (head->next->data->id != id)
        head = head->next;

    if (head)
    {
        struct pollfd_list * p = head->next;
        head->next = p->next;
        free(p->data);
        free(p); 
    }
}

struct pollfd_list * find_fd_by_id(delay * _delay, int id)
{
    struct pollfd_list * head = NULL;
    head = _delay->fds;

	while (head && (head->data->id != id))
		head = head->next;

    if (head)
        return head;

	return NULL;
}