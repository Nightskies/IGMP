#ifndef _MSG_H_
#define _MSG_H_

#define ALLRTRS_GROUP "224.0.0.2"
#define ALLHOSTS_GROUP "224.0.0.1"

#include "host.h"

// send Membership Report
void send_membership_report(const uint32_t src, const uint32_t group);

// accept Membership Query (General or Specific)
int accept_query(struct host * _host);

// send Leave Group
void send_leave_group(struct host * _host, const uint32_t group);

#endif // _MSG_H_