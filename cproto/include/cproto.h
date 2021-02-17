#pragma once
#include "json/json.h"
#include <cstdint>
#include <cassert>
#include <queue>
#include <memory>

constexpr uint8_t CPROTO_VERSION = 1;
constexpr uint8_t CPROTO_RESPONSE_MAGIC = 81;
constexpr uint8_t CPROTO_REQUEST_MAGIC = 82;
constexpr uint8_t CPROTO_MESSAGE_MAGIC = 83;
constexpr uint16_t CPROTO_SERVER = 0; 
constexpr uint32_t CPROTO_MAX_SIZE = 10 * 1024 * 1024;
constexpr uint32_t CPROTO_HEAD_SIZE = 8;

enum class ResponseStatus : uint8_t
{
    SUCCESS = 200,
    INTERNAL_ERROR = 110,
};

struct CProtoHead
{
    uint8_t version;
    uint8_t magic;
    uint16_t server;
    uint32_t len;
};

struct CProtoMsg
{
    CProtoHead head;
    Json::Value body;
};

class CProtoEncoder
{
public:
    static std::shared_ptr<uint8_t> encode(CProtoMsg* pMsg, uint32_t& len);
private:
    static void headEncode(uint8_t* pData, CProtoMsg* pMsg);
};

enum class CProtoParserStatus
{
    ON_PARSER_INIT = 0,
    ON_PARSER_HEAD = 1,
    ON_PARSER_BODY = 2,
};

class CProtoDecoder
{
public:
    void init();
    void clear();
    bool parser(void* data, size_t len);
    bool empty();
    std::shared_ptr<CProtoMsg> front();
    void pop();
private:
    bool parserHead(uint8_t **curData, uint32_t& curLen,
                    uint32_t& parserLen, bool& parserBreak);
    bool parserBody(uint8_t** curData, uint32_t& curLen,
                    uint32_t& parserLen, bool& parserBreak);

    CProtoMsg mCurMsg;
    std::queue<std::shared_ptr<CProtoMsg>> mMsgQ;
    std::vector<uint8_t> mCurReserved;
    CProtoParserStatus mCurParserStatus;
};

static bool isValidMagic(uint8_t magic)
{
    return magic == CPROTO_MESSAGE_MAGIC || magic == CPROTO_REQUEST_MAGIC || magic == CPROTO_RESPONSE_MAGIC;
}