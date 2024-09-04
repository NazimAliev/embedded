#ifndef SXR_UTILS_HPP
#define SXR_UTILS_HPP

#include <SXRMessageEcuToVb.hpp>
#include <SXRMessageVbToEcu.hpp>


namespace sendxreceive
{
class SXRUtils
{
public:
     static EcuMessage createEcuMessage(uint16_t seqID, uint16_t SID, uint16_t CID,
                         uint32_t TransactionId, uint8_t* PayloadData_ptr,
                         size_t PayloadData_LENGTH);
     static VbMessage createVbMessage(const gateway::MessageId &id,
                         const uint32_t &transactionId,
                         const std::vector<uint8_t> &data);

};
}  // namespace sendxreceive

#endif
