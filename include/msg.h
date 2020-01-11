#ifndef _MSG_H_
#define _MSG_H_

#include "host.h"

void send_membership_report(const uint32_t src, const uint32_t group, char * packet);

void accept_query(struct host * _host, char * packet);

void send_leave_group(struct host * _host, const uint32_t group, char * packet);

#endif // _MSG_H_