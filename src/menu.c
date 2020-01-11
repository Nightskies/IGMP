#include "../include/menu.h"

/*
extern char * optagr;
extern int optind, opterr, optopt;

void act_menu(int argc, char ** const argv)
{
    int key = 0;

    while (-1 != (key = getopt(argc, argv, "")))
    {
        switch(key)
        {
        }
    }
}
*/

void main(int argc, char **argv)
{
    if (argc == 1)
        ERROR("Usage: ./igmp groups interface\n");

    char * packet = (char *)malloc(BUF_SIZE);
    if (packet == NULL)
        ERROR("malloc returned Null");

    struct host * head = NULL;
    srand(time(NULL));

    head = init_host(argc, argv, packet);

    send_leave_group(head, head->head->data->group, packet);

    //accept_query(head);
/*
    switch(fork())
    {
        case -1:
            fatal("main: fork");
        
        case 0:
            while(true)
                accept_query(head);
            break;

        default:
            wait(NULL);
            break;
        
    }

*/
    close(sfd);
    exit(EXIT_SUCCESS);
}