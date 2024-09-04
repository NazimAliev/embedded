#ifndef SEND_X_RECEIVE_REMOTE_HPP
#define SEND_X_RECEIVE_REMOTE_HPP

#include <DltInjector.hpp>
#include <ServiceAuthorization.hpp>
#include <VehicleBackendClient.hpp>
#include "IPC_generated_base_SXR.h"

#include <SXRMessageEcuToVb.hpp>
#include <SXRMessageVbToEcu.hpp>
#include <SXRQueue.hpp>

#include "SXRCommonInj.hpp"
#include "vsm/SXRVehicleSignalManager.hpp"
#include "engmenu/SXREngMenuAdapter.hpp"

namespace assistanceservice
{
class IPowerManagementClient;
template <class BridgeInterface>
class RpcBridgePoller;

}  // namespace assistanceservice

namespace sendxreceive
{
class SendXReceive : protected assistanceservice::VehicleBackendClient,
                       protected assistanceservice::ServiceAuthorization,
                       public AsyncIPC::IPC_generated_base_SXR
{
    friend class assistanceservice::RpcBridgePoller<SendXReceive>;

    friend class SXRCommonInj;
    friend class SXRDaivbInjection;
    friend class SXRDaivbPublishInjection;
    friend class SXRVsmInjection;
    friend class SXREcuPublishInjection;

  public:
    SendXReceive(std::unique_ptr<assistanceservice::IPowerManagementClient>
                       powerManagementClient,
                    std::unique_ptr<sendxreceive::SXRVehicleSignalManager> vehicleSignalManager,
                    std::unique_ptr<SXREngMenuAdapter> engMenuAdapter,
                   assistanceservice::DltInjector &injector);
    ~SendXReceive();

    bool init(const std::string &appName);
    void stop();

    // Bridge
    virtual bool SendSXRMessageFromQueueToEcu(const VbMessage &msg);

    // callback from RPC Bridge
    virtual void IPC_SendSXRMessageToDaiVB(uint16_t seqID, uint16_t SID,
                                           uint16_t CID,
                                           uint8_t *PayloadData_ptr,
                                           size_t PayloadData_LENGTH) override;

    // VehicleBackendClient
    bool initQueues(
        std::unique_ptr<SXRQueue<VbMessage>> queueVbToEcu,
        std::unique_ptr<SXRQueue<EcuMessage>> queueEcuToVb);

    virtual bool sendMessageFromQueueToBackend(const EcuMessage &mess);

    // overriden as-commonlib client method(-s)
    void onAuthorizationStatusChanged(const std::string &, bool) override;
    void onMessageBackend(const gateway::MessageId &msgc,
                          uint32_t transactionId,
                          const std::vector<uint8_t> &data) override;

    virtual bool initVehicleBackendClient(const std::string &appName);
    virtual bool initServiceAuthorization(const std::string &appName);
    virtual bool initPowerManagement(const std::string &appName);
    virtual bool initVehicleSignalManager(const std::string &appName);
    virtual bool subscribeToSignalsVSM(std::function<void(uint16_t, uint16_t, uint16_t,
                          uint8_t*, size_t)> sendMessageToVb);

  private:
    bool checkGvnWakeup(const gateway::MessageId &msgc);

    // keep this sequence for correct call dtors
    std::atomic<uint16_t> m_seqId{0};
    std::unique_ptr<assistanceservice::IPowerManagementClient>
        m_powerManagementClient;
    std::unique_ptr<sendxreceive::SXRVehicleSignalManager> m_vehicleSignalManager;
    std::unique_ptr<SXREngMenuAdapter> m_engMenuAdapter;
    
    assistanceservice::DltInjector &m_injector;
    dlt::Context m_context;

    std::unique_ptr<SXRQueue<VbMessage>> m_queueVbToEcu;
    std::unique_ptr<SXRQueue<EcuMessage>> m_queueEcuToVb;



    std::vector<std::unique_ptr<SXRCommonInj>> m_injections;
};
}  // namespace sendxreceive

#endif
