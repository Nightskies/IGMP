#ifndef _INTERP_H_
#define _INTERP_H_

#include "host.h"
#include <sys/wait.h>
#include <signal.h>

#define BUFSIZE 512
#define TOKENSIZE 64
#define DELIM " \t\r\n\a"

// number of commands
int ncommands(void);

// add command
int com_add(char ** args, struct host * _host);

// delete command
int com_del(char ** args, struct host * _host);

// exit command
int com_exit(char ** args, struct host * _host);

// print command
int com_print(char ** args, struct host * _host);

// read line 
char * read_line(void);

// parse line to tokens
char ** parse_line(char * line);

// command execution
int exe(char ** args, struct host * _host);

// menu
void act_menu(struct host * _host);

#endif // _INTERP_H_

