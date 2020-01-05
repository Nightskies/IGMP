#include "../include/interpreter.h"
#include "../include/msg.h"

char * commands_str[] = { "add","del" };

int (*commands_func[]) (char **, struct host *) = { &com_add, &com_del, &com_exit};

int ncommands(void)
{
	return sizeof(commands_str) / sizeof(char *);
}

int com_add(char ** args, struct host * _host)
{
    set_group(args[1], _host);
    printf("added group[%s] \n", args[1]);

    send_membership_report(_host->if_addr, _host->head->data->group);

	return 1;
}

int com_del(char ** args, struct host * _host)
{
    send_leave_group(_host, parse_to_ip(args[1]));
    printf("leave group[%s] \n", args[1]);
	return 1;
}

int com_exit(char ** args, struct host * _host)
{
    struct group_list * tmp = NULL;

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
    {
        fprintf(stderr, "read_line: Error calloc \n");
        exit(EXIT_FAILURE);
    }

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
			{
				fprintf(stderr, "read_line: Error realloc \n");
                exit(EXIT_FAILURE);
			}
		}
	}
}

char ** parse_line(char * line)
{
	char ** args = (char **)calloc(TOKENSIZE,sizeof(char *));
    if(args == NULL)
    {
        fprintf(stderr, "parse_line: Error calloc \n");
        exit(EXIT_FAILURE);
    }

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
			{
				fprintf(stderr, "parse_line: Error realloc \n");
                exit(EXIT_FAILURE);
			}

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
			fatal("launch: fork");

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
		fputs("Enter command > ", stdout);
		line = read_line();
		args = parse_line(line);
		status = exe(args, _host);

		free(line);
		free(args);
	}
	while (status);
}