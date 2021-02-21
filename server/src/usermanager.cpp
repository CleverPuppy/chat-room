#include "usermanager.h"
#include "cprotomsg.h"
#include "uuid/uuid.h"

void UserManager::registerAndLoginUser(int fd, const std::string& username, const std::string& password) 
{
    // check username validy
    std::string info;
    if(!checkUserNameValidity(username, info))
    {
        auto msg =  CProtoMsgManager::genInfoResponse(ResponseStatus::INTERNAL_ERROR, info);
        if(CProtoMsgManager::encodeAndSendMsg(msg, fd) == -1)
        {
            fprintf(stderr, "registerAndLoginUser failed\n");
        };
        return;
    }

    // TODO REGISTER SUCCESSFUL
    PeoplePtr people = std::make_shared<People>(username, password, generateUserId());
    assert(userIdMap.count(people->getUserID()) == 0);
    assert(userNameMap.count(people->getUserName()) == 0);
    mUsers.emplace_back(people);
    userIdMap[people->getUserID()] = people;
    userNameMap[people->getUserName()] = people->getUserID();
    // LOGIN
    loginUser(fd, username, password);
}

void UserManager::loginUser(int fd,  const std::string& username, const std::string& password) 
{
    if(userNameMap.count(username) != 0)
    {
        auto ID = userNameMap[username];
        if(userIdMap[ID]->getUserPassword() == password)
        {
            auto token = generateUserToken(ID);
            auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::SUCCESS, token);
            if(CProtoMsgManager::encodeAndSendMsg(msg, fd) == -1)
            {
                fprintf(stderr, "UserManager::loginUser send Msg error\n");
            }
            return;
        }
    }

    auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::INTERNAL_ERROR, "Wrong username or password");
    if(CProtoMsgManager::encodeAndSendMsg(msg, fd) == -1)
    {
        fprintf(stderr, "UserManager::loginUser send Msg error\n");
    }
    
}

bool UserManager::checkUserNameValidity(const UserName& username, std::string& info) 
{
    if(userNameMap.count(username) != 0)
    {
        info = "Username " + username + " Already Registered!\n";
        return false;
    }
    return true;
}

UserId UserManager::generateUserId() const
{
    return ++minUserId;
}

UserToken UserManager::generateUserToken(UserId ID) 
{
    // TODO token generator
    auto newToken = userIdMap[ID]->getUserName();
    userIdMap[ID]->updateToken(newToken);
    tokenMap[newToken] = ID;
    return newToken;
}

UserId UserManager::getUserIdFromToken(const UserToken& token) const
{
    if(tokenMap.count(token) == 0)
    {
        return 0;
    }
    return tokenMap.at(token);
}


bool UserManager::verify(const UserToken& token) const
{
    return bool(tokenMap.count(token) == 1);
}

bool UserManager::verify(const UserId& id) const
{
    return bool(userIdMap.count(id) == 1);
}

PeoplePtr UserManager::getUser(const UserId& id) 
{
    return userIdMap.at(id);
}
UserId UserManager::verifyTokenAndGetUserId(const CProtoMsg& msg, int fd) 
{
    auto token = CProtoMsgManager::getToken(msg);
    auto userId = getUserIdFromToken(token);
    // invalid token
    if(!userId)
    {
        std::string info = "Invalid Token";
        auto msg = CProtoMsgManager::genInfoResponse(ResponseStatus::INTERNAL_ERROR, info);
        CProtoMsgManager::encodeAndSendMsg(msg, fd);
    }
    return userId;
}