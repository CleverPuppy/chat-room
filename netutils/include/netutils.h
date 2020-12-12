#pragma once
#include "sys/types.h"

class NetUtils
{
public:
    NetUtils() = delete;
    
    static int create_socket();
    static void make_socket_unblock(int fd);
    static void bind_and_listen(int fd, const char* ip_addr, uint16_t port);
    static void handle_error(const char* msg);


    static int const MAX_REQUEST = 100;
};