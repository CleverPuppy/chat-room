#include "client.h"
#include "netutils.h"
#include "login.h"
#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <cassert>
#include "unistd.h"
#include "sys/time.h"
#include "sys/types.h"

constexpr const char* SERVER_ADDRESS = "127.0.0.1";
constexpr uint16_t PORT = 8080;

Client::Client() 
    :status(ClientStatus::START), fd(-1)
{
    connectToServer();
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
        msgManager.establishNewConnection(fd);
        status = ClientStatus::WAITING_FOR_LOGING;
    }else if(conFlag == -1)
    {
        printf("Connect To Server Failed!\n%s\n", strerror(errno));
        status = ClientStatus::FAILED_CONNECT_TO_SERVER;
    }
    NetUtils::make_socket_unblock(fd);
}

void Client::waitingForLogin() 
{
    while (status == ClientStatus::WAITING_FOR_LOGING)
    {
        waitingForLoginHint();
        std::string line;
        std::vector<std::string> cmds;
        while (std::getline(std::cin, line) && status == ClientStatus::WAITING_FOR_LOGING)
        {
            cmds.clear();
            std::stringstream ss{line};
            std::string tmp;
            while(ss >> tmp)
            {
                cmds.push_back(tmp);
            }
            if (isCmdValid(cmds))
            {
                if(cmds.front() == LOGIN_CMD)
                {
                    Login::login(cmds[1], cmds[2], fd);
                }
                if(cmds.front() == REGISTER_CMD)
                {
                    Login::registerAndLogin(cmds[1],cmds[2], fd);
                }

                fd_set rfds;
                timeval tv{5, 0};
                FD_ZERO(&rfds);
                FD_SET(fd, &rfds);
                int select_ret = select(fd + 1, &rfds, NULL, NULL, &tv);
                if(select_ret == -1)
                {
                    fprintf(stderr, "select() error\n");
                }else if(select_ret)
                {
                    msgManager.readData(fd);
                    if(auto msgPt = msgManager.getMsg(fd))
                    {
                        std::cout << msgPt->body << std::endl;
                        if(CProtoMsgManager::isMsgSuccess(*msgPt))
                        {
                            token = msgPt->body["info"].asString();
                            printf("Login Successful!\n");
                            status = ClientStatus::WAITING_FOR_CMD;
                        }else{
                            auto info = msgPt->body["info"].asString();
                            printf("Login Failed, error %s\n", info.c_str());
                        }
                    }
                }else{
                    fprintf(stderr, "timeout\n");
                }

            }else break;
        }
    }
}

void Client::waitingForLoginHint() 
{
    printf("\t\t\t\t\tLogin or Register\n\n\n\n"
            "Cmds:\n"
            "Login:   \t\tlogin [username] [password]\n"
            "Register:\t\tregister [username] [password]\n");
}

bool Client::isCmdValid(const std::vector<std::string>& cmds) 
{
    assert(!cmds.empty());
    constexpr const char* registerCmd = "register";
    constexpr const char* loginCmd = "login";
    switch (status)
    {
    case ClientStatus::WAITING_FOR_LOGING:
        if(cmds.front() != registerCmd && cmds.front() != loginCmd)
        {
            wrongCmdNameHint(cmds.front());
            return false;
        }
        if(cmds.size() != 3)
        {
            wrongCmdSizeHint(cmds.front(), cmds.size() - 1, 3 - 1);
            return false;
        }
        break;
    
    default:
        return false;
    }
    return true;
}


void Client::wrongCmdSizeHint(const std::string& cmd, int real, int target) 
{
    fprintf(stderr, "comman [%s] must have %d argv[s] given, but %d was given\n", 
            cmd.c_str(), target, real);
}

void Client::wrongCmdNameHint(const std::string& cmd) 
{
    fprintf(stderr, "invalid cmd [%s]\n", cmd.c_str());
}

void Client::mainLoop() 
{
    printf("Main loop\n");
    while(true)
    {
        statusTransfrom();
    }
}

void Client::statusTransfrom() 
{
    switch (status)
    {
    case ClientStatus::FAILED_CONNECT_TO_SERVER:
        if(++retryTimes > MAX_RETRY_TIMES)
        {
            fprintf(stderr, "Reached max retry times still failed to connect to server\n");
            status = ClientStatus::EXIT;
        }else
        {
            printf("Retry ...%d\n", retryTimes);
            connectToServer();
        }
        break;
    case ClientStatus::WAITING_FOR_LOGING:
        waitingForLogin();
        break;
    case ClientStatus::EXIT:
        exit(0);
        break;
    default:
        printf("Exit!\n");
        break;
    }
}
