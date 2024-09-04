#include "SXRVehicleSignalsServiceClient.hpp"

namespace sendxreceive
{
    bool SXRVehicleSignalsServiceClient::init(const std::string& appName)
    {
        return m_client.init(appName);
    }

    bool SXRVehicleSignalsServiceClient::isAvailable() const
    {
        return m_client.isAvailable();
    }

    bool SXRVehicleSignalsServiceClient::subscribeVehicleSignals(
        const std::vector<VehicleSignalsServiceClient::SignalSubscription>& signalsSubscriptionList,
                      std::vector<VehicleSignalsServiceClient::SignalState>& signalsState,
                      std::vector<VehicleSignalsServiceClient::SignalIdentifier>& identifiers)
    {
        return m_client.subscribeVehicleSignals(signalsSubscriptionList, signalsState, identifiers);
    }

    bool SXRVehicleSignalsServiceClient::requestVehicleSignals(const std::vector<std::string>& signalsPath,
                      const std::vector<uint8_t>& signalsHistory,
                      VehicleSignalsServiceClient::VsResult& error,
                      std::vector<VehicleSignalsServiceClient::SignalIdentifier>& identifiers)
    {
        return m_client.requestVehicleSignals(signalsPath, signalsHistory, error, identifiers );
    }

    bool SXRVehicleSignalsServiceClient::setAndSendSignals(const std::vector<VehicleSignalsServiceClient::Signal> &signals,
                                         VehicleSignalsServiceClient::VsResult &error,
                                         std::vector<VehicleSignalsServiceClient::VsSetSendResult> &signalsResults) 
    {
        return m_client.setAndSendSignals(signals, error, signalsResults);
    }

    bool  SXRVehicleSignalsServiceClient::subscribeToVehicleSignalsSelectiveEvent(VehicleSignalsServiceClient::VehicleSignalsSelectiveEventCallback callback) 
    {
        return m_client.subscribeToVehicleSignalsSelectiveEvent(callback);
    }
}