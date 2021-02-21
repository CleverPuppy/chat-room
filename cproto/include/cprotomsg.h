#pragma once
#include <unordered_map>
#include "cproto.h"
#include "types.h"

enum class ResponseStatus : uint8_t
{
    SUCCESS,
    INTERNAL_ERROR,
    
    CHAT_LIST,
    ROOM_LIST,
    ROOM_ID,
    TOKEN,
    TOKEN_FAILED
};

class CProtoMsgManager
{
using DecoderPtr = std::shared_ptr<CProtoDecoder>;
public:
    CProtoMsgManager() = default;
    CProtoMsgManager(const CProtoMsgManager&) = delete;
    CProtoMsgManager& operator=(const CProtoMsgManager&) = delete;

    static CProtoMsg genChatMsg(const UserToken& token,
                                const RoomIDType& roomID,
                                const std::string& info);
    static CProtoMsg genInfoResponse(ResponseStatus status, const std::string& info)
    {
        CProtoMsg msg;
        setDefaultHead(msg, CPROTO_RESPONSE_MAGIC);
        msg.body["status"] = int(status);
        msg.body["info"] = info;
        return msg;
    }

    template<typename... Args>
    static CProtoMsg genTokenCmdRequest(const std::string& cmdName, const UserToken& token, Args... args)
    {
        std::pair<std::string, std::string> p = {"token", token};
        auto msg = genCmdRequest(cmdName, p, args...);
        // msg.body["token"] = token;
        return msg;
    }

    template<typename... Args>
    static CProtoMsg genCmdRequest(const std::string& cmdName, Args... args)
    {
        CProtoMsg msg;
        setDefaultHead(msg, CPROTO_REQUEST_MAGIC);
        msg.body["cmd"] = cmdName;
        appendArgs(msg, args...);
        return msg;
    }

    static bool isMsgSuccess(const CProtoMsg& msg)
    {
        if(msg.body["status"] != Json::nullValue 
            && msg.body["status"].asInt() == int(ResponseStatus::SUCCESS))
        {
            return true;
        }
        return false;
    }
    static UserToken getToken(const CProtoMsg& msg)
    {
        return msg.body["token"].asString();
    }
    static int encodeAndSendMsg(CProtoMsg& msg, int fd);

    void establishNewConnection(int fd);
    void releaseConnection(int fd);
    void readData(int fd);
    std::shared_ptr<CProtoMsg> getMsg(int fd);
private:
    std::unordered_map<int, DecoderPtr> fdMap;

    static void setDefaultHead(CProtoMsg& msg, uint8_t magic_number)
    {
        msg.head.version = CPROTO_VERSION;
        msg.head.magic = magic_number;
        msg.head.server = CPROTO_SERVER;
    }


    template<typename T, typename... Ts>
    static void appendArgs(CProtoMsg& msg, const T& arg, Ts... args)
    {
        msg.body["args"].append(arg);
        appendArgs(msg, args...);
    }

    template<typename T, typename... Ts>
    static void appendArgs(CProtoMsg& msg, const std::pair<std::string, T>& arg, Ts... args)
    {
        msg.body[arg.first] = arg.second;
        appendArgs(msg, args...);
    }

    static void appendArgs(CProtoMsg& msg)
    {
        // break callstack
    }
};