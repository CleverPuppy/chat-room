#include "login.h"

#include "sys/socket.h"
#include "sys/types.h"


bool Login::login(const std::string& username, const std::string& password, int fd) 
{
    // TODO check parameters validity
    auto encryptedPassword = crptPassword(password);
    auto msg = CProtoMsgManager::genCmdRequest(LOGIN_CMD, username, encryptedPassword);
    uint32_t msgLen;
    auto encodedMsg = CProtoEncoder::encode(&msg, msgLen);

    if(send(fd, encodedMsg.get(), msgLen, MSG_CONFIRM) == -1)
    {
        fprintf(stderr, "send ERROR :%s\n", strerror(errno));
        return false;
    };

    printf("Logining ... Waiting for server...\n");
    return true;
}

bool Login::registerAndLogin(const std::string& username, const std::string& password, int fd) 
{
    auto encryptedPassword = crptPassword(password);
    auto msg = CProtoMsgManager::genCmdRequest(REGISTER_CMD, username, encryptedPassword);
    uint32_t msgLen;
    auto encodedMsg = CProtoEncoder::encode(&msg, msgLen);
    if(send(fd, encodedMsg.get(), msgLen, MSG_CONFIRM) == -1)
    {
        fprintf(stderr, "send ERROR:%s\n", strerror(errno));
        return false;
    }
    printf("Registering ... Waiting for server...\n");
    return true;
}


std::string Login::crptPassword(const std::string& password) 
{
    std::string ret;
    // TODO encrpt password
    ret = password;
    return ret;
}
