#pragma once
#include <list>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "people.h"

constexpr UserId MIN_USER_ID = 10000;
using PeoplePtr = std::shared_ptr<People>;

class UserManager
{
public:
    UserManager() = default;
    UserManager& operator=(const UserManager&) = delete;
    UserManager(const UserManager&) = delete;

    void registerAndLoginUser(int fd, const std::string& username, const std::string& password);
    void loginUser(int fd,  const std::string& username, const std::string& password);
    bool verify(const UserToken& token) const;
    bool verify(const UserId& id) const;
    UserId getUserIdFromToken(const UserToken& token) const;
    UserId verifyTokenAndGetUserId(const CProtoMsg& msg, int fd);
    PeoplePtr getUser(const UserId& id);
private:
    mutable UserId minUserId = MIN_USER_ID + 1;
    std::vector<PeoplePtr> mUsers;
    std::unordered_map<UserId, PeoplePtr> userIdMap;
    std::unordered_map<UserName, UserId> userNameMap;
    std::unordered_map<UserToken, UserId> tokenMap;

    bool checkUserNameValidity(const UserName& username, std::string& info);
    UserId generateUserId() const;
    UserToken generateUserToken(UserId ID);
};