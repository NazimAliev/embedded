#pragma once
#include <gmock/gmock.h>
#include "SXRQueue.hpp"

namespace sxr_unit_test
{

template<class queueType> 
class SXRQueueMock : public SXRQueue<queueType>
{

public:
    SXRQueueMock(const std::string &queueName) 
        : SXRQueue<queueType>(queueName) {} 

    MOCK_METHOD0(start, SXRError());
    MOCK_METHOD0(stop, SXRError());
    MOCK_METHOD0(clear, SXRError());

    virtual SXRError insert(queueType msg) override
    {
        SXRQueue<queueType>::sendMessage(msg);
        return SXRError::SUCCESS;
    }
protected:
    MOCK_METHOD0(processQueue, void());
};
}  // namespace sxr_unit_test