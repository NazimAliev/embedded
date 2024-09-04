#include "SXREcuPublishInjection.hpp"
#include "SendXReceive.hpp"

namespace sendxreceive
{
SXREcuPublishInjection::SXREcuPublishInjection(
    SendXReceive& app)
    : SXRCommonInj(app)
{
}

int SXREcuPublishInjection::handleData(std::string&& data)
{
    /*
    using atp = asremotedoor::packet::atp::RemoteDoorRequest;
    gateway::MessageId id = {atp::CONTENT_TYPE, atp::IID, atp::SID, atp::CID};

    m_app.onMessageBackend(
        id, 0, std::vector<uint8_t>(data.begin(), data.begin() + data.size()));
        */
    handlerEcuPublish(std::move(data));
    return 0;
}

// injection from console app sxr_ecu_publish simulate "from SXR to DaiVB"
void SXREcuPublishInjection::handlerEcuPublish(std::string&& data)
{
    m_app.m_context.logInfo() << "Receive EcuPublish injection";

    SXRMessage sxrMes;
    if (parseInjectionData(data, sxrMes) != 0)
    {
        return;
    }

    HeaderEcuId header;
    header.SID = sxrMes.SID;
    header.CID = sxrMes.CID;

    bool res = m_app.sendMessageFromQueueToBackend({header, sxrMes.payload});
    if (res)
    {
        m_app.m_context.logInfo()
            << "handlerEcuPublish::sendMessageBackend OK " << sxrMes;
    }
    else
    {
        m_app.m_context.logError()
            << "handlerEcuPublish::sendMessageBackend FAIL " << sxrMes;
    }
}

assistanceservice::DltInjector::Info
SXREcuPublishInjection::MakeInjectionInfo()
{
    using namespace std::placeholders;
    return assistanceservice::DltInjector::Info{
        SXREcuPublishInjection::serviceId,
        std::bind(&SXREcuPublishInjection::handleData, this, _1),
        SXREcuPublishInjection::name,
        SXREcuPublishInjection::helpStr};
}
}  // namespace sendxreceive