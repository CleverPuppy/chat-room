#include "sys/types.h"
#include "sys/socket.h"
#include "unistd.h"
#include "fcntl.h"

#include "netutils.h"

int 
NetUtils::create_socket()
{
    int fd;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    return fd;
}

void
NetUtils::make_socket_unblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}