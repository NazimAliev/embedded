#include "SXRMessageEcuToVb.hpp"
#include "SXRTypes.hpp"

DLT_IMPORT_CONTEXT(mainCtx)

dlt::LogMsg& operator<<(dlt::LogMsg& os, const sendxreceive::EcuMessage& mess)
{
    const uint8_t *PayloadData_ptr = mess.payload.data();
    size_t PayloadData_LENGTH = mess.payload.size();
    
    int len;
    len = PayloadData_LENGTH > 8 ? 8 : PayloadData_LENGTH;

    auto data = static_cast<const void *>(PayloadData_ptr);
    os << "Message:"
        << "Header:"
        << "seqID:" << mess.header.seqID 
        << "SID:" << mess.header.SID << "CID:" << mess.header.CID
        << "TransactionId:" << mess.header.TransactionId
        << "PayloadData:"
        << dlt::Buffer(const_cast<void *>(data), len)
        << "PayloadData_LENGTH:" << PayloadData_LENGTH;
    return os;
}

namespace sendxreceive
{

bool EcuMessage::operator==(const EcuMessage &b) const
{
    return header == b.header && payload == b.payload;
}
}  // namespace sendxreceive
