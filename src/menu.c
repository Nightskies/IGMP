#include "../include/menu.h"

void main(int argc, char **argv)
{
    if (argc == 1)
        ERROR("Usage: ./igmp [OPTIONS] group1 group2 [...] interface\n");

    act_opt(argc, argv);

    struct host * head = NULL;
    srand(time(NULL));

    head = init_host(argc, argv);

    act_menu(head);

/*
    switch(fork())
    {
        case -1:
            SYS_ERROR("fork");
        
        case 0:
            while(true)
                accept_query(head);
            break;

        default:
            act_menu(head);
            break;
        
    }
*/
    close(sfd);
    exit(EXIT_SUCCESS);
}

void act_opt(int argc, char ** const argv)
{
    int key = 0;

    while (-1 != (key = getopt(argc, argv, "h")))
    {
        switch(key)
        {
            case 'h':
                printf("Usage: ./igmp [OPTIONS] group1 group2 [...] interface\n");
                printf("COMMANDS:\nadd <ip> - add multicast group to host \n");
                printf("del <ip> - delete multicast group from host \n");
                printf("print - displays the name of the network interface "
                    "and the list of multicast groups the host is subscribed to\n");
                exit(EXIT_SUCCESS);

        }
    }
}