#pragma once

#include "SXRIVehicleSignalsServiceClient.hpp"
#include <VehicleSignalsServiceClient.hpp>

namespace sendxreceive
{
class SXRVehicleSignalsServiceClient: public SXRIVehicleSignalsServiceClient
{
    public:
        SXRVehicleSignalsServiceClient() = default;
        virtual ~SXRVehicleSignalsServiceClient() = default;
        virtual bool init(const std::string& appName);
        virtual bool isAvailable() const;
        virtual bool subscribeVehicleSignals(const std::vector<VehicleSignalsServiceClient::SignalSubscription>& signalsSubscriptionList,
                      std::vector<VehicleSignalsServiceClient::SignalState>& signalsState,
                      std::vector<VehicleSignalsServiceClient::SignalIdentifier>& identifiers);
        virtual bool requestVehicleSignals(const std::vector<std::string>& signalsPath,
                      const std::vector<uint8_t>& signalsHistory,
                      VehicleSignalsServiceClient::VsResult& error,
                      std::vector<VehicleSignalsServiceClient::SignalIdentifier>& identifiers);
        
        virtual bool setAndSendSignals(const std::vector<VehicleSignalsServiceClient::Signal> &signals,
                                         VehicleSignalsServiceClient::VsResult &error,
                                         std::vector<VehicleSignalsServiceClient::VsSetSendResult> &signalsResults);

        virtual bool subscribeToVehicleSignalsSelectiveEvent(VehicleSignalsServiceClient::VehicleSignalsSelectiveEventCallback callback);
    private:
        VehicleSignalsServiceClient m_client;    
};
}