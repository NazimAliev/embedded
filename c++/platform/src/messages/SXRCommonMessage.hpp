#ifndef SXR_Message
#define SXR_Message

#include <cstdint>
#include <dlt/LogMsg.hpp>
#include <vector>

namespace sendxreceive
{

typedef struct SXRMessage
{
    uint16_t SID;
    uint16_t CID;
    std::vector<uint8_t> payload;
} SXRMessage;

typedef struct SXRVSMSignal
{
    uint16_t ID;
    std::vector<uint8_t> payload;
} SXRVSMSignal;



}  // namespace sendxreceive
dlt::LogMsg& operator<<(dlt::LogMsg& os, const sendxreceive::SXRMessage& mess);
dlt::LogMsg& operator<<(dlt::LogMsg& os, const sendxreceive::SXRVSMSignal& signal);
#endif
