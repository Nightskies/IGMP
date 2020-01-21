#ifndef _MENU_H_
#define _MENU_H_

#include <stdbool.h>

extern char * optagr;
extern int optind, opterr, optopt;
extern bool Debug;

// handling options
void act_opt(int argc, char ** const argv);

#endif // _MENU_H_