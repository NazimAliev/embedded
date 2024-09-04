#include "SXRUtils.hpp"

namespace sendxreceive
{
EcuMessage SXRUtils::createEcuMessage(uint16_t seqID, uint16_t SID, uint16_t CID,
                         uint32_t TransactionId, uint8_t* PayloadData_ptr,
                         size_t PayloadData_LENGTH)
{
    EcuMessage ecuMessage;
    // fill header
    ecuMessage.header = {seqID, SID, CID, TransactionId};

    // init pointer to payload
    ecuMessage.payload.assign(PayloadData_ptr,
                                PayloadData_ptr + PayloadData_LENGTH);
    return ecuMessage;
}

VbMessage SXRUtils::createVbMessage(const gateway::MessageId &id,
                         const uint32_t &transactionId,
                         const std::vector<uint8_t> &data)
{
    (void)transactionId;

    VbMessage vbMessage;

    // fill header
    vbMessage.header = {id.iid, 0, id.sid, id.cid};
    vbMessage.payload = data;
    return vbMessage;
}

}  // namespace sendxreceive