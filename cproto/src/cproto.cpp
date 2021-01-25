#include "cproto.h"
#include "arpa/inet.h"
#include <memory>


void
CProtoEncoder::headEncode(uint8_t* pData, CProtoMsg* pMsg)
{
    *pData = 1;
    ++pData;

    *pData = CPROTO_MAGIC;
    ++pData;

    *(uint16_t *)pData = htons(pMsg->head.server);
    pData += 2;

    *(uint32_t *)pData = htonl(pMsg->head.len);
}

uint8_t*
CProtoEncoder::encode(CProtoMsg * pMsg, uint32_t & len)
{
    uint8_t *pData = nullptr;
    Json::FastWriter fwrite{};

    std::string bodyStr = fwrite.write(pMsg->body);
    len = CPROTO_HEAD_SIZE + (uint32_t)bodyStr.size();
    pMsg->head.len = len;

    pData = new uint8_t[len];
    headEncode(pData, pMsg);
    std::memcpy(pData + CPROTO_HEAD_SIZE, bodyStr.data(),
            bodyStr.size());
    return pData;
}

void
CProtoDecoder::init()
{
    mCurParserStatus = CProtoParserStatus::ON_PARSER_INIT;
}

void
CProtoDecoder::clear()
{
    while(!mMsgQ.empty())
    {
        mMsgQ.pop();
    }
}

bool CProtoDecoder::parser(void *data, size_t len)
{
    if(len <= 0)
    {
        return false;
    }

    uint32_t curLen = 0;
    uint32_t parserLen = 0;
    uint8_t *curData = nullptr;

    curData = (uint8_t*)data;

    while(len--)
    {
        mCurReserved.push_back(*curData);
        ++curData;
    }

    curLen = mCurReserved.size();
    curData = mCurReserved.data();

    while(curLen > 0)
    {
        bool parserBreak = false;

        if(mCurParserStatus == CProtoParserStatus::ON_PARSER_INIT ||
           mCurParserStatus == CProtoParserStatus::ON_PARSER_BODY)
        {
            if(!parserHead(&curData, curLen, parserLen, parserBreak))
            {
                return false;
            }

            if(parserBreak) break;
        }

        if(mCurParserStatus == CProtoParserStatus::ON_PARSER_HEAD)
        {
            if(!parserBody(&curData, curLen, parserLen, parserBreak))
            {
                return false;
            }

            if(parserBreak) break;
        }

        if(mCurParserStatus == CProtoParserStatus::ON_PARSER_BODY)
        {
            // CProtoMsg* pMsg = nullptr;
            // pMsg = new CProtoMsg;
            // *pMsg = mCurMsg;
            // mMsgQ.emplace(pMsg);
            mMsgQ.push(std::shared_ptr<CProtoMsg>(new CProtoMsg{mCurMsg}));
        }
    }

    if(parserLen > 0)
    {
        mCurReserved.erase(mCurReserved.begin(),
                           mCurReserved.begin() + parserLen);
    }
}

std::shared_ptr<CProtoMsg> CProtoDecoder::front()
{
    return mMsgQ.front();
}

void CProtoDecoder::pop()
{
    mMsgQ.pop();
}

bool CProtoDecoder::empty() 
{
    return mMsgQ.empty();
}

bool CProtoDecoder::parserHead(uint8_t **curData, uint32_t &curLen,
                               uint32_t &parserLen, bool &parserBreak)
{
    parserBreak = false;
    if (curLen < CPROTO_HEAD_SIZE)
    {
        parserBreak = true;
        return  true;
    }

    uint8_t *pData = *curData;

    mCurMsg.head.version = *pData;
    pData++;
    mCurMsg.head.magic = *pData;
    pData++;
    if(CPROTO_MAGIC != mCurMsg.head.magic)
    {
        return false;
    }
    mCurMsg.head.server = ntohs(*(uint16_t*)pData);
    pData += 2;
    mCurMsg.head.len = ntohl(*(uint32_t*)pData);
    if(mCurMsg.head.len > CPROTO_MAX_SIZE)
    {
        return false;
    }

    (*curData) += CPROTO_HEAD_SIZE;
    curLen -= CPROTO_HEAD_SIZE;
    parserLen += CPROTO_HEAD_SIZE;
    mCurParserStatus = CProtoParserStatus::ON_PARSER_HEAD;

    return true;
}

bool CProtoDecoder::parserBody(uint8_t **curData, uint32_t &curLen,
                               uint32_t &parserLen, bool &parserBreak)
{
    parserBreak = false;
    uint32_t jsonSize = mCurMsg.head.len - CPROTO_HEAD_SIZE;
    if(curLen < jsonSize)
    {
        parserBreak = true;
        return true;
    }

    Json::Reader reader;
    if(!reader.parse((char*)(*curData), (char*)((*curData) + jsonSize),
                     mCurMsg.body, false))
    {
        return false;
    }

    (*curData) += jsonSize;
    curLen -= jsonSize;
    parserLen += jsonSize;
    mCurParserStatus = CProtoParserStatus::ON_PARSER_BODY;

    return true;
}

