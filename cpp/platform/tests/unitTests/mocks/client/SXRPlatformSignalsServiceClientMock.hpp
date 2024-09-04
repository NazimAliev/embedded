#pragma once

#include <gmock/gmock.h>
#include <vsm/SXRVehicleSignalsServiceClient.hpp>

namespace sxr_unit_test
{

class SXRVehicleSignalsServiceClientMock : public sendxreceive::SXRVehicleSignalsServiceClient
{
  public:
    SXRVehicleSignalsServiceClientMock()
    {
    }

    MOCK_METHOD1(init, bool(const std::string &));
    MOCK_CONST_METHOD1(isAuthorized, bool(const std::string &));

    MOCK_METHOD4(requestVehicleSignals, bool (const std::vector<std::string>&,
                      const std::vector<uint8_t>&,
                      VehicleSignalsServiceClient::VsResult&,
                      std::vector<VehicleSignalsServiceClient::SignalIdentifier>&));
    
    MOCK_METHOD3(setAndSendSignals, bool(const std::vector<VehicleSignalsServiceClient::Signal> &,
                                         VehicleSignalsServiceClient::VsResult &,
                                         std::vector<VehicleSignalsServiceClient::VsSetSendResult> &));
                                         
    MOCK_METHOD3(subscribeVehicleSignals, bool(const std::vector<VehicleSignalsServiceClient::SignalSubscription>&,
                      std::vector<VehicleSignalsServiceClient::SignalState>&,
                      std::vector<VehicleSignalsServiceClient::SignalIdentifier>&));

    MOCK_METHOD1(subscribeToVehicleSignalsSelectiveEvent, bool(VehicleSignalsServiceClient::VehicleSignalsSelectiveEventCallback));
};
}  // namespace sxr_unit_test