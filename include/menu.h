#ifndef _MENU_H_
#define _MENU_H_

#include "host.h"
#include "interpreter.h"
#include <sys/wait.h>

extern char * optagr;
extern int optind, opterr, optopt;
extern int pid;

void act_opt(int argc, char ** const argv);

#endif // _MENU_H_