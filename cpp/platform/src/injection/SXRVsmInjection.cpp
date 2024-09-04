#include "SXRVsmInjection.hpp"
#include "SendXReceive.hpp"

namespace sendxreceive
{
SXRVsmInjection::SXRVsmInjection(SendXReceive& app)
    : SXRCommonInj(app)
{
    m_callback = app.m_vehicleSignalManager->createCallback();
}

int SXRVsmInjection::handleData(std::string&& data)
{
    /*
    using atp = asremotedoor::packet::atp::RemoteDoorRequest;
    gateway::MessageId id = {atp::CONTENT_TYPE, atp::IID, atp::SID, atp::CID};

    m_app.onMessageBackend(
        id, 0, std::vector<uint8_t>(data.begin(), data.begin() + data.size()));
        */
    handlerEcuToVb(std::move(data));
    return 0;
}

void SXRVsmInjection::handlerEcuToVb(std::string&& data)
{
    m_app.m_context.logInfo() << "Receive injection from 'ECU'";

    SXRVSMSignal signal;
    if (parseInjectionDataVSM(data, signal) != 0)
    {
        return;
    }

    std::vector<VehicleSignalsServiceClient::Signal> m_SXRSignals;
    m_SXRSignals[0].setIdentifier(signal.ID);
    m_SXRSignals[0].setValue(signal.payload);
    m_callback(m_SXRSignals);

    return;
}

assistanceservice::DltInjector::Info
SXRVsmInjection::MakeInjectionInfo()
{
    using namespace std::placeholders;
    return assistanceservice::DltInjector::Info{
        SXRVsmInjection::serviceId,
        std::bind(&SXRVsmInjection::handleData, this, _1),
        SXRVsmInjection::name, SXRVsmInjection::helpStr};
}
}  // namespace sendxreceive