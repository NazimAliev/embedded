#include "SendXReceive.hpp"
#include "SXRCommonMessage.hpp"


dlt::LogMsg& operator<<(dlt::LogMsg& os, const sendxreceive::SXRMessage& mess)
{
    const uint8_t *PayloadData_ptr = mess.payload.data();
    size_t PayloadData_LENGTH = mess.payload.size();
    
    int len;
    len = PayloadData_LENGTH > 8 ? 8 : PayloadData_LENGTH;

    auto data = static_cast<const void *>(PayloadData_ptr);
    os << "Message:"
                        << "SID:" << mess.SID << "CID:" << mess.CID
                        << "PayloadData:"
                        << dlt::Buffer(const_cast<void *>(data), len)
                        << "PayloadData_LENGTH:" << PayloadData_LENGTH;
    return os;
}


dlt::LogMsg& operator<<(dlt::LogMsg& os, const sendxreceive::SXRVSMSignal& signal)
{
    const uint8_t *PayloadData_ptr = signal.payload.data();
    size_t PayloadData_LENGTH = signal.payload.size();
    
    int len;
    len = PayloadData_LENGTH > 8 ? 8 : PayloadData_LENGTH;

    auto data = static_cast<const void *>(PayloadData_ptr);
    os << "VSM Signal:"
                        << "ID:" << signal.ID  
                        << "PayloadData:"
                        << dlt::Buffer(const_cast<void *>(data), len)
                        << "PayloadData_LENGTH:" << PayloadData_LENGTH;
    return os;
}
