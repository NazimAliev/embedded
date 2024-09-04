#ifndef VEHICLESIGNALMANAGER
#define VEHICLESIGNALMANAGER

#include <dlt/dlt_user.h>
#include <dlt/dlt_user_macros.h>
#include <vsm/SXRIVehicleSignalsServiceClient.hpp>
#include <VehicleSignalsServiceClient.hpp>

DLT_IMPORT_CONTEXT(serviceContext)



#include "SXRMessageVbToEcu.hpp"

namespace sendxreceive
{
class SXRVehicleSignalManager 
{
  public:
    SXRVehicleSignalManager(std::unique_ptr<SXRIVehicleSignalsServiceClient> client);
    bool init(const std::string& appName);
    bool SendSXRMessageToVsm(const VbMessage &msg);
    bool subscribeToSignals( std::function<void(uint16_t, uint16_t, uint16_t,
                          uint8_t*, size_t)> sendMessageToVb );
    VehicleSignalsServiceClient::VehicleSignalsSelectiveEventCallback createCallback();
 private:
    void fill();
    std::vector<VehicleSignalsServiceClient::SignalSubscription> createSignalsList();

    std::string VsResult_to_string(
        const VehicleSignalsServiceClient::VsResult &vsResult) const;

    std::unique_ptr<SXRIVehicleSignalsServiceClient> m_client;
    std::map<std::pair<uint16_t, uint16_t>, VehicleSignalsServiceClient::SignalIdentifier> m_currentTxConfig;
    std::map<VehicleSignalsServiceClient::SignalIdentifier, std::pair<uint16_t, uint16_t>> m_currentRxConfig;
    
    std::vector<VehicleSignalsServiceClient::Signal> m_SXRSignals;
    std::function<void(uint16_t, uint16_t, uint16_t,
                          uint8_t*, size_t)> m_sendMessageToVb = nullptr;
    dlt::Context m_context;
};

}  // namespace sendxreceive

#endif /* VEHICLESIGNALMANAGER */
