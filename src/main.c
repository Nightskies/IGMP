#include "../include/main.h"
#include "../include/msg.h"
#include "../include/host.h"
#include "../include/socket.h"

bool Debug = false;

void main(int argc, char **argv)
{
    if (argc == 1)
        ERROR("Usage: ./igmp [OPTIONS] group1 group2 [...] interface\n");

    act_opt(argc, argv);

    struct host * head = NULL;
    srand(time(NULL));

    head = init_host(argc, argv);

    act_menu(head);
}

void act_opt(int argc, char ** const argv)
{
    int key = 0;

    while (-1 != (key = getopt(argc, argv, "hd")))
    {
        switch(key)
        {
            case 'h':
                printf("Usage: ./igmp [OPTIONS] group1 group2 [...] interface\n");
                printf("OPTIONS:\n-h <help> information about program\n");
                printf("-d <debug> additional debugging information\n");
                printf("COMMANDS:\nadd <ip> - add multicast group to host \n");
                printf("del <ip> - delete multicast group from host \n");
                printf("print - displays info about interface \n");
                exit(EXIT_SUCCESS);
            
            case 'd':
                Debug = true;
                break;
        }
    }
}