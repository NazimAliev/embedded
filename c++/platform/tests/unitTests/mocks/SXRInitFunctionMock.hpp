#pragma once
#include <gmock/gmock.h>
#include "SXRHelperMock.hpp"
namespace sxr_unit_test
{
using namespace sendxreceive;

class SXRInitFunctionMock: public SXRHelperMock
{
  public:
    SXRInitFunctionMock(SXRDependencies& dp)
      : SXRHelperMock(dp)
    {
    }

    MOCK_METHOD1(initVehicleBackendClient, bool(const std::string &));
    MOCK_METHOD1(initServiceAuthorization, bool(const std::string &));
    MOCK_METHOD1(initPowerManagement, bool(const std::string &));
    MOCK_METHOD1(initVehicleSignalManager, bool(const std::string &));

    MOCK_METHOD1(subscribeToSignalsVSM, bool(std::function<void(uint16_t, uint16_t, uint16_t,
                          uint8_t*, size_t)>));

    MOCK_CONST_METHOD1(isAuthorized, bool(const std::string &));
    MOCK_METHOD1(getMessageReceiveProfile,
                 assistanceservice::MsgWakeupRequiredState(
                     const gateway::MessageId &id));
};
}  // namespace sxr_unit_test