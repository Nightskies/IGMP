#include "../include/interpreter.h"
#include "../include/msg.h"
#include "../include/main.h"
#include "../include/socket.h"

char * commands_str[] = { "add","del","print","exit" };

int (*commands_func[]) (char **, struct host *) = { &com_add, &com_del, &com_print, &com_exit};

int ncommands(void)
{
	return sizeof(commands_str) / sizeof(char *);
}

int com_add(char ** args, struct host * _host)
{
	uint32_t group_ip;

	if (group_ip = parse_to_ip(args[1]))
	{
		if (find_by_group(_host, group_ip))
		{
			printf(STYLE_RED_BOLD "group[%s] is already on the list\n" STYLE_RESET, args[1]);
			return 1;
		}

		printf(STYLE_BLUE_BOLD "add group[%s]" STYLE_RESET, args[1]);
    	set_group(group_ip, _host);
    	send_membership_report(_host->if_addr, parse_to_ip(args[1]));
	}
	else
		printf(STYLE_RED_BOLD "This's not a multicast ip[%s]\n" STYLE_RESET, args[1]);

	return 1;
}

int com_del(char ** args, struct host * _host)
{
	struct group_list * group = NULL;

	if (_host->_delay->timers_status)
	{
		printf(STYLE_RED_BOLD "\nTo leave a group, you need to wait for all reports to be sent"  STYLE_RESET);
		return 1;
	}

	if (group = find_by_group(_host, parse_to_ip(args[1])))
    	send_leave_group(_host, parse_to_ip(args[1]));

	else
		printf(STYLE_RED_BOLD "\nYou're not subscribed to group[%s]" STYLE_RESET, args[1]);

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
	printf("list of subscribed groups");

	head = _host->head;

    while(head)
    {
        addr.s_addr = head->data->group;
        printf("\n%d - %s",i++,inet_ntoa(addr));
        head = head->next;
    }
	printf(STYLE_RESET);

	return 1;
}

int com_exit(char ** args, struct host * _host)
{
	while(_host->head)
		send_leave_group(_host, _host->head->data->group);

    free(_host->_delay);
	free(_host);
	close(ssfd);
	close(rsfd);

	printf(STYLE_BLUE_BOLD "\nExit client\n" STYLE_RESET);
    
	return 0;
}

int read_line(char * buf)
{
	int i = 0;

	char ch;

	while (true)
	{
		ssize_t nbyte = read(STDIN_FILENO, &ch, 1);

		if (ch == '\n')
		{
			buf[i] = '\0';
			return 0;
		}

		if (nbyte == 1)
			buf[i++] = ch;

		else return -1;
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

int exe(char ** args, struct host * _host)
{
	int i;

	if (args[0] == NULL)
		return 1;

	for (i = 0; i < ncommands(); i++)
		if (strcmp(args[0], commands_str[i]) == 0)
			return (*commands_func[i])(args, _host);

	printf(STYLE_BLUE_BOLD "COMMANDS:\nadd <ip> - add multicast group to host \n" STYLE_RESET);
    printf(STYLE_BLUE_BOLD "del <ip> - delete multicast group from host \n" STYLE_RESET);
    printf(STYLE_BLUE_BOLD "print - displays info about interface" STYLE_RESET);
}