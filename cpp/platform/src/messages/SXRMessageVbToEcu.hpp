#ifndef MESSAGE_VB_TO_ECU_HPP
#define MESSAGE_VB_TO_ECU_HPP

#include "SXRTypes.hpp"

namespace sendxreceive
{
typedef struct HeaderVbId
{
    uint16_t iid;
    uint16_t seqID;
    uint16_t SID;
    uint16_t CID;

    bool operator==(const HeaderVbId &b) const
    {
        return iid == b.iid && seqID == b.seqID && SID == b.SID && CID == b.CID;
    }
} HeaderVbId;

typedef struct VbMessage
{
    HeaderVbId header;
    std::vector<uint8_t> payload;
    bool operator==(const VbMessage &b) const;

} VbMessage;
}  // namespace sendxreceive
dlt::LogMsg& operator<<(dlt::LogMsg& os, const sendxreceive::VbMessage& mess);
#endif
