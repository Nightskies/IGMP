#ifndef _INTERP_H_
#define _INTERP_H_

#include "host.h"
#include <sys/wait.h>
#include <signal.h>

#define BUFSIZE 512
#define TOKENSIZE 64
#define DELIM " \t\r\n\a"

int ncommands(void);

int com_add(char ** args, struct host * _host);

int com_del(char ** args, struct host * _host);

int com_exit(char ** args, struct host * _host);

int com_print(char ** args, struct host * _host);

char * read_line(void);

char ** parse_line(char * line);

int launch(char ** args);

int exe(char ** args, struct host * _host);

void act_menu(struct host * _host);

#endif // _INTERP_H_

