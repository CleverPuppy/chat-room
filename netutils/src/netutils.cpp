#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
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

void
NetUtils::bind_and_listen(int fd, const char* ip_addr, uint16_t port)
{
    struct sockaddr addr;
    set_sockaddr(ip_addr, port, &addr);

    if(bind(fd, &addr,
         sizeof(struct sockaddr)) == -1)
    {
        handle_error("bind");
    }
    
    if(listen(fd, MAX_REQUEST) == -1)
    {
        handle_error("listen");
    }
}

void
NetUtils::handle_error(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void
NetUtils::set_sockaddr(const char* ip_addr, uint16_t port, sockaddr* _dest)
{
    sockaddr_in* in = (sockaddr_in*)_dest;
    in->sin_family = AF_INET;
    in->sin_addr.s_addr = inet_addr(ip_addr);
    in->sin_port = htons(port);
}
