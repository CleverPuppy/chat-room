#include "client.h"
#include "netutils.h"
#include <cstdio>
#include <string>
#include <cerrno>
#include <cstring>
#include "unistd.h"

constexpr const char* SERVER_ADDRESS = "127.0.0.1";
constexpr uint16_t PORT = 8080;

Client::Client() 
    :status(ClientStatus::START), fd(-1)
{
    connectToServer();
    status = ClientStatus::WAITING_FOR_LOGING;
}

Client::~Client() 
{
    if(fd > 0)
    {
        close(fd);
    }
}

void Client::connectToServer() 
{
    if(status != ClientStatus::START && status != ClientStatus::FAILED_CONNECT_TO_SERVER)
    {
        return;
    }

    printf("connecting to server %s@%d...\n",SERVER_ADDRESS, PORT);
    fd = NetUtils::create_socket();
    if (fd <= 0)
    {
        NetUtils::handle_error("Counld't create_socket!\n");
    }
    sockaddr addr;
    NetUtils::set_sockaddr(SERVER_ADDRESS, PORT, &addr);
    int conFlag = connect(fd, &addr, sizeof(addr));
    if(conFlag == 0)
    {
        printf("Connected!\n");
    }else if(conFlag == -1)
    {
        printf("Connect To Server Failed!\n%s\n", strerror(errno));
        status = ClientStatus::FAILED_CONNECT_TO_SERVER;
    }
    NetUtils::make_socket_unblock(fd);
}


int main()
{
    Client client{};
    // login to server
    // waiting for cmd
    return 0;
}
