
#include "SXRVehicleSignalManager.hpp"
#include "SXRCommonBackend.hpp"

namespace sendxreceive
{

SXRVehicleSignalManager::SXRVehicleSignalManager(std::unique_ptr<SXRIVehicleSignalsServiceClient> client)
	 : 	m_client(std::move(client)),
     m_context("APP", "as-sendxreceive application logs")
{
}

bool SXRVehicleSignalManager::init(const std::string& appName)
{
    if (m_client->init(appName) != true)
    {
        m_context.logError() << __LOGHEAD__ << "failed to init localization manager client";
        return false;
    }

    std::vector<std::string> pathes(VsmConfig.size());
    for (unsigned int i = 0; i < VsmConfig.size(); i++) 
    {
        pathes[i] = VsmConfig[i].path;
    }

    VehicleSignalsServiceClient::VsResult error = VehicleSignalsServiceClient::VsResult::VS_E_UNKNOWN;
    std::vector<VehicleSignalsServiceClient::SignalIdentifier> signalIds;


    if (m_client->requestVehicleSignals(pathes, {}, error, signalIds))
    {
        for (unsigned int i = 0; i < signalIds.size(); i++) 
        {
            VehicleSignalsServiceClient::SignalIdentifier id = signalIds[i];
            std::pair<uint16_t, uint16_t> pduid = VsmConfig[i].pduid;
            if (VsmConfig[i].type == VSMSignalType::Tx)
            {
                m_currentTxConfig.insert({pduid, id});
            }
            else
            {
                 m_currentRxConfig.insert({id, pduid});
            }
            m_context.logInfo() << "For message with pduid {SID:" << pduid.first << ", CID:" 
                                << pduid.second << "} " << "found signal id:" << id;
        }
    }
    else
    {
        m_context.logError() << __LOGHEAD__ << "ERROR: Failed to request vehicle signals, error:"
            << VsResult_to_string(error);
        return false;
    }
    
    fill();

    return true;
}

void SXRVehicleSignalManager::fill()
{
    VehicleSignalsServiceClient::Signal tempSignal;
    tempSignal.setIdentifier(0);
    tempSignal.setValue(0000U);
    tempSignal.setType(
        VehicleSignalsServiceClient::TypeOfSignal::BYTEBUFFER);

    tempSignal.setState(
        VehicleSignalsServiceClient::SignalState::VS_ENABLE);
    tempSignal.setTimestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock().now().time_since_epoch())
            .count());

    m_SXRSignals.emplace_back(tempSignal);
}

bool SXRVehicleSignalManager::SendSXRMessageToVsm(const VbMessage &msg)
{
    uint16_t SID = msg.header.SID;
    uint16_t CID = msg.header.CID;

    auto pduid = std::pair<uint16_t, uint16_t>( SID, CID);
    if (!m_currentTxConfig.count(pduid))
    {
        m_context.logError()
            << __LOGHEAD__ << "PDUID not found "
            << "PDUID: "
            << "{SID:" << SID << ", CID:" << CID << "} "
            << "Msg:" << " " << msg;
            return false;
    }

    VehicleSignalsServiceClient::VsResult vsResult;
    std::vector<VehicleSignalsServiceClient::VsSetSendResult> vsSetSendResult;

    m_SXRSignals[0].setIdentifier(m_currentTxConfig.at(pduid));
    m_SXRSignals[0].setValue(msg.payload);

    if (!m_client->setAndSendSignals(m_SXRSignals, vsResult,
                                    vsSetSendResult))
    {
        m_context.logError()
            << __LOGHEAD__ << "setAndSendSignals failed. vsResult: "
            << VsResult_to_string(vsResult);
        return false;
    }

    return true;
}

inline std::string SXRVehicleSignalManager::VsResult_to_string(
    const VehicleSignalsServiceClient::VsResult &vsResult) const
{
    switch (vsResult)
    {
        case VehicleSignalsServiceClient::VsResult::VS_E_OK:
            return "VS_E_OK";
        case VehicleSignalsServiceClient::VsResult::VS_E_FAILED:
            return "VS_E_FAILED";
        case VehicleSignalsServiceClient::VsResult::VS_E_UNCONSISTENT_INPUTS:
            return "VS_E_UNCONSISTENT_INPUTS";
        case VehicleSignalsServiceClient::VsResult::VS_E_UNKNOWN:
        default:
            return "VS_E_UNKNOWN";
    }
}
// Receiving signals from VSM
bool SXRVehicleSignalManager::subscribeToSignals( std::function<void(uint16_t, uint16_t, uint16_t,
                          uint8_t*, size_t)> sendMessageToVb )
{
    std::vector<VehicleSignalsServiceClient::SignalIdentifier> signalIds;
	std::vector<VehicleSignalsServiceClient::SignalState> signalStates;

    std::vector<VehicleSignalsServiceClient::SignalSubscription> signalsList = createSignalsList();
	bool subscribeResult = m_client->subscribeVehicleSignals(signalsList, signalStates, signalIds);
	if (!subscribeResult)
	{
        m_context.logError() <<  __LOGHEAD__ << "Subscription failed for some of the signals in the list:";
		for(auto signal : signalsList) 
		{
			DLT_LOG(serviceContext, DLT_LOG_ERROR, DLT_CSTRING(signal.getSignalPath().c_str()));
		}
		return false;
	}
	
    auto callback = createCallback();
	bool regCallbackResult = m_client->subscribeToVehicleSignalsSelectiveEvent(callback);
	if(!regCallbackResult)
	{
        m_context.logError() <<  __LOGHEAD__ << "Callback registration failed";
		return false;
	}
	return true;

    m_sendMessageToVb = sendMessageToVb;
    return true;

}

std::vector<VehicleSignalsServiceClient::SignalSubscription> SXRVehicleSignalManager::createSignalsList()
{
    std::vector<VehicleSignalsServiceClient::SignalSubscription> signalsList;

    VehicleSignalsServiceClient::SignalSubscription gpsTrackingSubscription;
    gpsTrackingSubscription.setSignalPath("12822//Event/EI_MeCall_MaintOffset_Rs_ST3/EI_MeCall_MaintOffset_Rs_ST3");
    gpsTrackingSubscription.setPeriod(0);  // report on change
    signalsList.push_back(gpsTrackingSubscription);
    return signalsList;

}

VehicleSignalsServiceClient::VehicleSignalsSelectiveEventCallback SXRVehicleSignalManager::createCallback()
{
    return [this](const std::vector<VehicleSignalsServiceClient::Signal>& signals)
        {
            for (const auto& signal : signals)
            {
                VehicleSignalsServiceClient::SignalType signalValue;

                if (!signal.getValue(signalValue))
                {
                    m_context.logError() <<  __LOGHEAD__ << 
                    "Failed to fetch vehicle signal value, id: " <<
                        signal.getIdentifier();
                    continue;
                }
                if (m_currentRxConfig.count(signal.getIdentifier()))
                {
                    m_context.logError() <<  __LOGHEAD__ << 
                    "Can not find PUID for signal id " <<
                        signal.getIdentifier();
                    continue;
                }
                std::pair<uint16_t, uint16_t> pduid = m_currentRxConfig.at(signal.getIdentifier());
                
                std::string strValue;
                signal.getValueToString(strValue);
                
                void *ptr = const_cast<char *>(strValue.data());
                uint8_t *PayloadData_ptr = static_cast<uint8_t *>(ptr);

                size_t PayloadData_LENGTH = strValue.size();

                m_sendMessageToVb(-1, pduid.first, pduid.second, PayloadData_ptr, PayloadData_LENGTH);
            
            }
        };
}
} // namespace sendxreceive