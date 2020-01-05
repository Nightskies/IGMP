#include "../include/error.h"
#include "../include/socket.h"

void fatal(const char * msg)
{
    perror(msg);

    if (sfd != -1)
        close(sfd);
        
    exit(EXIT_FAILURE);
}