#pragma once
#include <gmock/gmock.h>
#include <IVehicleBackendClient.hpp>
#include <SendXReceive.hpp>
#include "../mocks/client/SXRVehicleSignalsServiceClientMock.hpp"
#include "../mocks/client/SXRVehicleSignalManagerMock.hpp"

#include "SXRHelperMock.hpp"

namespace sxr_unit_test
{
using namespace sendxreceive;

class SXRWithQueueMock : public SXRHelperMock
{
  public:
    SXRWithQueueMock(SXRDependencies& dp)
      : SXRHelperMock(dp)
    {
    }

    MOCK_METHOD1(init, bool(const std::string &));
    MOCK_CONST_METHOD1(isAuthorized, bool(const std::string &));
    MOCK_METHOD1(sendMessageFromQueueToBackend, bool(const EcuMessage &msg));
    MOCK_METHOD1(getMessageReceiveProfile,
                 assistanceservice::MsgWakeupRequiredState(
                     const gateway::MessageId &id));
    MOCK_METHOD1(SendSXRMessageFromQueueToEcu,
                 bool(const VbMessage &msg));
};
}  // namespace sxr_unit_test