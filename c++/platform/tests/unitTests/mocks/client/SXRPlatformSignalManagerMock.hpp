#pragma once
#include <gmock/gmock.h>
#include <vsm/SXRVehicleSignalManager.hpp>

namespace sxr_unit_test
{
using namespace sendxreceive;
class SXRVehicleSignalManagerMock : public SXRVehicleSignalManager
{

public:
    SXRVehicleSignalManagerMock(std::unique_ptr<SXRIVehicleSignalsServiceClient> client)
    : SXRVehicleSignalManager(std::move(client)){};
    
    MOCK_METHOD1(init, bool(const std::string &));
    MOCK_METHOD1(SendSXRMessageToVsm, bool (const VbMessage &));
    MOCK_METHOD1(subscribeToSignals, bool ( std::function<void(uint16_t, uint16_t, uint16_t,
                          uint8_t*, size_t)> ));
    MOCK_METHOD0(createCallback, VehicleSignalsServiceClient::VehicleSignalsSelectiveEventCallback());
};

}  // namespace sxr_unit_test