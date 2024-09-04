#ifndef MESSAGE_ECU_TO_VB_HPP
#define MESSAGE_ECU_TO_VB_HPP

#include "SXRTypes.hpp"


namespace sendxreceive
{

typedef struct HeaderEcuId
{
    uint16_t seqID;
    uint16_t SID;
    uint16_t CID;
    uint32_t TransactionId;
    bool operator==(const HeaderEcuId &b) const
    {
        return TransactionId == b.TransactionId && seqID == b.seqID && SID == b.SID && CID == b.CID;
    }
} HeaderEcuId;

typedef struct EcuMessage
{
    HeaderEcuId header;
    std::vector<uint8_t> payload;

    bool operator==(const EcuMessage &b) const;
} EcuMessage;

}  // namespace sendxreceive
dlt::LogMsg& operator<<(dlt::LogMsg& os, const sendxreceive::EcuMessage& mess);
#endif
