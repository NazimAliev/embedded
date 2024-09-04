#include <iomanip>
#include <iterator>

#include "SXRCommonMain.hpp"
#include "SendXReceive.hpp"
#include "SXRUtils.hpp"

#include <IPowerManagementClient.hpp>
#include <IServiceAuthorization.hpp>
#include <IVehicleBackendClient.hpp>

#include "SXRDaivbInjection.hpp"
#include "SXRDaivbPublishInjection.hpp"
#include "SXRVsmInjection.hpp"
#include "SXREcuPublishInjection.hpp"

#include "SXRCommonBackend.hpp"

#include <thread>

DLT_IMPORT_CONTEXT(gateWaySvcClientCtx)
DLT_IMPORT_CONTEXT(consoleCtx)

namespace
{
const std::string authProperty =
    "sXR";  // Comes from Gateway ApplicationSettingManager.cpp
constexpr std::chrono::milliseconds gvnRequestTimer{5000};

}  // namespace

namespace sendxreceive
{
SendXReceive::SendXReceive(
    std::unique_ptr<assistanceservice::IPowerManagementClient>
        powerManagementClient,
    std::unique_ptr<sendxreceive::SXRVehicleSignalManager> vehicleSignalManager,
    std::unique_ptr<SXREngMenuAdapter> engMenuAdapter,
    assistanceservice::DltInjector &injector)
    : assistanceservice::VehicleBackendClient(injector),
      assistanceservice::ServiceAuthorization(injector),
      m_powerManagementClient(std::move(powerManagementClient)),
      m_vehicleSignalManager(std::move(vehicleSignalManager)),
      m_engMenuAdapter(std::move(engMenuAdapter)),
      m_injector(injector),
      m_context("APP", "as-sendxreceive application logs")
{
    m_injections.reserve(4);
    m_injections.push_back(
        std::move(std::make_unique<SXRDaivbInjection>(*this)));
    m_injections.push_back(std::move(
        std::make_unique<SXRDaivbPublishInjection>(*this)));
    m_injections.push_back(
        std::move(std::make_unique<SXRVsmInjection>(*this)));
    m_injections.push_back(
        std::move(std::make_unique<SXREcuPublishInjection>(*this)));
}

SendXReceive::~SendXReceive()
{
    m_context.logInfo() << "SendXReceive destroyed";
}

// init
bool SendXReceive::init(const std::string &appName)
{
    registerProperty(authProperty);

    // register callbacks in dlt injection
    for (auto &injPtr : m_injections)
        m_injector.registerInjection(injPtr->MakeInjectionInfo());

    m_context.logDebug() << __LOGHEAD__ << "Injection handlers registered";

    auto queueVbToEcu =
        std::make_unique<SXRQueue<VbMessage>>("QueueVbToEcu");
    auto queueEcuToVb =
        std::make_unique<SXRQueue<EcuMessage>>("QueueEcuToVb");

    initQueues(std::move(queueVbToEcu), std::move(queueEcuToVb));

    m_context.logDebug() << __LOGHEAD__ << "queues initialized";

    auto init_result = true;
    if (!initVehicleBackendClient(appName))
    {
        m_context.logError()
            << __LOGHEAD__ << " Failed to initialize vehicle backend client";
        init_result = false;
    }

    if (!initServiceAuthorization(appName))
    {
        m_context.logError()
            << __LOGHEAD__ << " Failed to initialize service authorization";
        init_result = false;
    }

    if (!initPowerManagement(appName))
    {
        m_context.logError()
            << __LOGHEAD__ << " Failed to initialize power management client";
        init_result = false;
    }


    if(!initVehicleSignalManager(appName))
    {
        m_context.logError()
            << __LOGHEAD__ << " Failed to initialize vehicle signal manager client";
        init_result = false;
    } 
    else 
    {
        m_context.logInfo() << __LOGHEAD__ << " Successful initialize vehicle signal manager client";

        auto callback = [this](
                uint16_t seqID, uint16_t SID,
                uint16_t CID, uint8_t *PayloadData_ptr,
                size_t PayloadData_LENGTH)
        {
            IPC_SendSXRMessageToDaiVB(seqID, SID, CID, PayloadData_ptr, PayloadData_LENGTH);
        };
        bool res = subscribeToSignalsVSM(callback);
            
        if (!res)
        {
            m_context.logError()
                << __LOGHEAD__ << "Failed to subscribe to VSM signals via vehicle signal manager client";
            init_result = false;
        }
        else
        {
            m_context.logInfo() << __LOGHEAD__ << " Successful subscribe to VSM signals via vehicle signal manager client";
        }
    }

    if (!m_engMenuAdapter->init(appName))
    {
        m_context.logError() << __LOGHEAD__ 
            << " failed to initialize EngMenu adapter";
        init_result = false;
    }
    return init_result;
}

// Just for mock in test
bool SendXReceive::subscribeToSignalsVSM(std::function<void(uint16_t, uint16_t, uint16_t,
                          uint8_t*, size_t)> sendMessageToVb)
{
    return m_vehicleSignalManager->subscribeToSignals(sendMessageToVb);
}

bool SendXReceive::initVehicleBackendClient(const std::string &appName)
{
    return VehicleBackendClient::init(appName);
}

bool SendXReceive::initVehicleSignalManager(const std::string &appName)
{
    return m_vehicleSignalManager->init(appName);
}

bool SendXReceive::initServiceAuthorization(const std::string &appName)
{
    return assistanceservice::ServiceAuthorization::init(appName);
}

bool SendXReceive::initPowerManagement(const std::string &appName)
{
    return m_powerManagementClient->init(appName);
}

bool SendXReceive::initQueues(
    std::unique_ptr<SXRQueue<VbMessage>> queueVbToEcu,
    std::unique_ptr<SXRQueue<EcuMessage>> queueEcuToVb)
{
    m_queueVbToEcu = std::move(queueVbToEcu);
    m_queueVbToEcu->setSendMessageCallback(
        [this](VbMessage msg) { SendSXRMessageFromQueueToEcu(msg); });

    m_queueEcuToVb = std::move(queueEcuToVb);
    m_queueEcuToVb->setSendMessageCallback(
        [this](EcuMessage msg) { sendMessageFromQueueToBackend(msg); });

    m_queueVbToEcu->start();
    m_queueEcuToVb->start();

    return true;
}  // init

bool SendXReceive::checkGvnWakeup(const gateway::MessageId &msgc)
{
    if (!m_powerManagementClient)
    {
        m_context.logError()
            << __LOGHEAD__ << " m_powerManagementClient is null!";
        return false;
    }

    if (!m_powerManagementClient->isGvnAvailable())
    {
        m_context.logWarning() << __LOGHEAD__ << " gvn is not awailable";
        auto wakeupProfile = getMessageReceiveProfile(msgc);
        m_context.logInfo() << __LOGHEAD__ << " wakeup profile is "
                            << static_cast<uint8_t>(wakeupProfile);
        if (wakeupProfile ==
            assistanceservice::MsgWakeupRequiredState::WAKEUP_REQUIRED)
        {
            if (!m_powerManagementClient->gvnWakeUpAndWait(gvnRequestTimer))
            {
                m_context.logWarning()
                    << __LOGHEAD__
                    << " failed to wake up gvn, Gateway message dropped";
                return false;
            }
        }
        else
        {
            m_context.logWarning()
                << __LOGHEAD__
                << " wakeup is not requested, Gateway message dropped";
            return false;
        }
    }
    return true;
}

void SendXReceive::stop()
{
    m_context.logInfo() << __LOGHEAD__ << "Stopping SXR service...";
    m_queueEcuToVb->stop();
    m_queueVbToEcu->stop();
}

// overriden as-commonlib client method(-s)

void SendXReceive::onAuthorizationStatusChanged(const std::string &property,
                                                  bool status)
{
    m_context.logInfo() << __LOGHEAD__ << "Authorization status for "
                        << property << " has been changed to " << status;
}

// VehicleBackendClient
void SendXReceive::onMessageBackend(const gateway::MessageId &msgc,
                                      uint32_t transactionId,
                                      const std::vector<uint8_t> &data)
{
    if (!checkGvnWakeup(msgc)) return;

    if (!isAuthorized(authProperty))
    {
        m_context.logError()
            << __LOGHEAD__ << " Authorization for " << authProperty
            << " is failed. Dropping ATP message.";
        return;
    }

    // send msg obj to ECU
    VbMessage mess = SXRUtils::createVbMessage(msgc, transactionId, data);
    SXRError res = m_queueVbToEcu->insert(mess);
    if (res != SXRError::SUCCESS)
    {
        m_context.logError()
            << __LOGHEAD__ << "insertMessage to m_queueVbToEcu FAIL " << mess;
    }
    else
    {
        m_context.logInfo()
            << __LOGHEAD__ << "insertMessage to m_queueVbToEcu OK " << mess;
    }
}

bool SendXReceive::sendMessageFromQueueToBackend(const EcuMessage &mess)
{
    // Construct MessageId struct and fill the 7 bytes message reference
    // Serialize the data into an std::vector<uint8_t>

    // gateway::MessageId msgId = {gateway::ContentType::ATP, 16, 0, 2};
    // std::vector<uint8_t> data {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    // 0x07, 0x08};
    gateway::MessageId msg;
    msg.contentType = gateway::ContentType::SXR;
    msg.iid = 0;

    HeaderEcuId headerEcuId = mess.header;
    msg.sid = headerEcuId.SID;
    msg.cid = headerEcuId.CID;

    bool res = sendMessage(msg, mess.payload);

    if (res)
    {
        m_context.logError()
            << __LOGHEAD__ << "send message to gateway OK " << mess;
    }
    else
    {
        m_context.logInfo()
            << __LOGHEAD__ << "send message to gateway FAIL " << mess;
    }

    return res;
}

// Bridge
bool SendXReceive::SendSXRMessageFromQueueToEcu(const VbMessage &msg)
{
    /*
    uint16_t seqID = 0;
    HeaderVbId header = msg.header;
    uint16_t SID = header.SID;
    uint16_t CID = header.CID;
    uint8_t *PayloadData_ptr = const_cast<uint8_t *>(msg.payload.data());
    size_t PayloadData_LENGTH = msg.payload.size();
    bool res = IPC_SendSXRMessageToEcu(seqID, SID, CID, PayloadData_ptr,
                                       PayloadData_LENGTH);
    */

    bool res = m_vehicleSignalManager->SendSXRMessageToVsm(msg);

    if (res)
    {
        m_context.logInfo()
            << __LOGHEAD__ << "IPC_SendSXRMessageToEcu: OK " << msg;
    }
    else
    {
        m_context.logInfo()
            << __LOGHEAD__ << "IPC_SendSXRMessageToEcu: FAIL " << msg;
    }
    return res;
}

// virtual AsyncIPC::IPC_generated_base_SXR -> BridgeInterface
// get callback msg from RPC Bridge and insert msg to queue
void SendXReceive::IPC_SendSXRMessageToDaiVB(uint16_t seqID, uint16_t SID,
                                               uint16_t CID,
                                               uint8_t *PayloadData_ptr,
                                               size_t PayloadData_LENGTH)
{
    EcuMessage msg = SXRUtils::createEcuMessage(
        seqID, SID, CID, 0, PayloadData_ptr, PayloadData_LENGTH);

    
    if (!isAuthorized(authProperty))
    {
        m_context.logError()
            << __LOGHEAD__ << " Authorization for " << authProperty
            << " is failed. Dropping ECU message. " << msg;
        return;
    }
    

    auto pduid = std::pair<uint16_t, uint16_t>(SID, CID);
    if (g_configFromTcu.count(pduid) == 1)
    {
        m_context.logInfo()
            << __LOGHEAD__ << "Registered RPC PDUID found: OK "
            << "PDUID: "
            << "{SID:" << SID << ", CID:" << CID << "} "
            << "Name:" << g_configFromTcu.at(pduid).name.c_str() << " " << msg;
    }
    else
    {
        m_context.logError()
            << __LOGHEAD__ << "Registered RPC PDUID found: FAIL "
            << "PDUID: "
            << "{SID:" << SID << ", CID:" << CID << "} " << msg;
    }
    // log for console app
    DLT_LOG(consoleCtx, DLT_LOG_INFO, DLT_INT16(SID), DLT_INT16(CID),
            DLT_RAW(PayloadData_ptr, PayloadData_LENGTH));

    // call child virtual function
    SXRError res = m_queueEcuToVb->insert(msg);
    if (res != SXRError::SUCCESS)
    {
        m_context.logError()
            << __LOGHEAD__ << "insertMessage to m_queueEcuToVb: FAIL " << msg;
    }
    else
    {
        m_context.logError()
            << __LOGHEAD__ << "insertMessage to m_queueEcuToVb: OK " << msg;
    }
}

}  // namespace sendxreceive
