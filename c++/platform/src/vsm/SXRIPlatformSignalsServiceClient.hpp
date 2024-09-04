#pragma once
#include <VehicleSignalsServiceClient.hpp>

namespace sendxreceive
{
class SXRIVehicleSignalsServiceClient
{
  public:
    SXRIVehicleSignalsServiceClient() = default;
    virtual ~SXRIVehicleSignalsServiceClient() = default;
    SXRIVehicleSignalsServiceClient(const SXRIVehicleSignalsServiceClient&) = delete;
    SXRIVehicleSignalsServiceClient& operator=(const SXRIVehicleSignalsServiceClient&) = delete;
    SXRIVehicleSignalsServiceClient(const SXRIVehicleSignalsServiceClient&&) = delete;
    SXRIVehicleSignalsServiceClient& operator=(const SXRIVehicleSignalsServiceClient&&) = delete;

    virtual bool init(const std::string& appName) = 0;
    virtual bool isAvailable() const = 0;
    virtual bool requestVehicleSignals(const std::vector<std::string>& signalsPath,
                      const std::vector<uint8_t>& signalsHistory,
                      VehicleSignalsServiceClient::VsResult& error,
                      std::vector<VehicleSignalsServiceClient::SignalIdentifier>& identifiers) = 0;

    virtual bool subscribeVehicleSignals(const std::vector<VehicleSignalsServiceClient::SignalSubscription>& signalsSubscriptionList,
                      std::vector<VehicleSignalsServiceClient::SignalState>& signalsState,
                      std::vector<VehicleSignalsServiceClient::SignalIdentifier>& identifiers) = 0;
    
    virtual bool setAndSendSignals(const std::vector<VehicleSignalsServiceClient::Signal> &signals,
                                         VehicleSignalsServiceClient::VsResult &error,
                                         std::vector<VehicleSignalsServiceClient::VsSetSendResult> &signalsResults) = 0;

    virtual bool  subscribeToVehicleSignalsSelectiveEvent(VehicleSignalsServiceClient::VehicleSignalsSelectiveEventCallback callback) = 0;
};

}