#ifndef _MENU_H_
#define _MENU_H_

#include "host.h"
#include "socket.h"
#include "msg.h"
#include "interpreter.h"
#include <sys/wait.h>

extern char * optagr;
extern int optind, opterr, optopt;

void act_opt(int argc, char ** const argv);

#endif // _MENU_H_