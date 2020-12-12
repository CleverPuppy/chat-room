#pragma once
#include "sys/types.h"

class NetUtils
{
public:
    NetUtils() = delete;
    
    static int create_socket();
    static void make_socket_unblock(int fd);
};