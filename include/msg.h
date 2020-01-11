#ifndef _MSG_H_
#define _MSG_H_

#define ALLRTRS_GROUP "224.0.0.2"
#define ALLHOSTS_GROUP "224.0.0.1"

#include "host.h"

void send_membership_report(const uint32_t src, const uint32_t group);

void accept_query(struct host * _host);

void send_leave_group(struct host * _host, const uint32_t group);

#endif // _MSG_H_