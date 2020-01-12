#include "../include/interpreter.h"
#include "../include/msg.h"
#include "../include/menu.h"
#include "../include/socket.h"

char * commands_str[] = { "add","del","print","exit" };

int (*commands_func[]) (char **, struct host *) = { &com_add, &com_del, &com_print, &com_exit};

int ncommands(void)
{
	return sizeof(commands_str) / sizeof(char *);
}

int com_add(char ** args, struct host * _host)
{
	printf(STYLE_BLUE_BOLD "add group[%s] \n" STYLE_RESET, args[1]);

    set_group(args[1], _host);
    send_membership_report(_host->if_addr, parse_to_ip(args[1]));
	return 1;
}

int com_del(char ** args, struct host * _host)
{
	if (find(_host, parse_to_ip(args[1])))
	{
    	send_leave_group(_host, parse_to_ip(args[1]));
		printf(STYLE_BLUE_BOLD "leave group[%s] \n" STYLE_RESET, args[1]);
	}

	else
		printf(STYLE_RED_BOLD  "You're not subscribed to group[%s]\n" STYLE_RESET, args[1]);

	return 1;
}

int com_print(char ** args, struct host * _host)
{
	struct group_list * head = NULL;

	struct in_addr addr;
	addr.s_addr = _host->if_addr;

	int i = 1;

	printf(STYLE_BLUE_BOLD "IGMP CLIENT \n");
	printf("INTERFACE = %s\n",_host->if_name);
	printf("list of subscribed groups\n");

	head = _host->head;

    while(head)
    {
        addr.s_addr = head->data->group;
        printf("%d - %s\n",i++,inet_ntoa(addr));
        head = head->next;
    }
	printf(STYLE_RESET);

	return 1;
}

int com_exit(char ** args, struct host * _host)
{
	printf(STYLE_BLUE_BOLD "Exit client\n" STYLE_RESET);

    struct group_list * tmp = NULL;

	kill(pid, SIGINT);

	struct group_list * head = NULL;
	head = _host->head;

	while(head)
	{
		send_leave_group(_host, head->data->group);
		printf(STYLE_BLUE_BOLD "leave group[%s] \n" STYLE_RESET, parse_to_str(head->data->group));
		head = head->next;
	}

    while (_host->head)
    {
        tmp = _host->head->next;
        free(_host->head->data);
        free(_host->head);
        _host->head = tmp;
    }
    free(_host);
    
	return 0;
}

char * read_line(void)
{
	int ch;

	int cur_buf_size = BUFSIZE;

	int i = 0;

	char * buf = (char *)calloc(BUFSIZE, sizeof(char));
    if(buf == NULL)
		ERROR("calloc returned Null");

	while (true)
	{
		ch = getchar();

		if (ch == EOF || ch == '\n')
		{
			buf[i] = '\0';
			return buf;
		}
		buf[i++] = ch;

		if (i == cur_buf_size)
		{
			cur_buf_size += BUFSIZE;
			buf = (char *)realloc(buf, cur_buf_size);
            if(buf == NULL)
				ERROR("realloc returned Null");
		}
	}
}

char ** parse_line(char * line)
{
	char ** args = (char **)calloc(TOKENSIZE,sizeof(char *));
    if(args == NULL)
		ERROR("calloc returned Null");

	int i = 0;

	int cur_buf_size = TOKENSIZE;

	char * token = NULL;

	token = strtok(line, DELIM);

	while (token != NULL)
	{
		args[i++] = token;

		if (i == cur_buf_size)
		{
			cur_buf_size += TOKENSIZE;
			args = (char **)realloc(args,cur_buf_size * sizeof(char *));
            if(args = NULL)
				ERROR("realloc returned Null");

		}
		token = strtok(NULL, DELIM);
	}
    args[i] = NULL;

	return args;
}

int launch(char ** args)
{
	pid_t pid;

	int status;

	switch (pid = fork())
	{
		case -1:
			SYS_ERROR("fork");

		case 0:
			execvp(*args, args);
			exit(EXIT_SUCCESS);
	}

	do
	{
		waitpid(pid, &status, WUNTRACED);
	}
	while (!WIFEXITED(status) && !WIFSIGNALED(status));

	return 1;
}

int exe(char ** args, struct host * _host)
{
	int i;

	if (args[0] == NULL)
		return 1;

	for (i = 0; i < ncommands(); i++)
		if (strcmp(args[0], commands_str[i]) == 0)
			return (*commands_func[i])(args, _host);

	return launch(args);
}

void act_menu(struct host * _host)
{
    char * line;
	char ** args;
	int status;

	do
	{
		fputs(STYLE_YELLOW_BOLD "Enter command > " STYLE_RESET, stdout);
		line = read_line();
		args = parse_line(line);
		status = exe(args, _host);

		free(line);
		free(args);
	}
	while (status);

	close(sfd);
	exit(EXIT_SUCCESS);
}