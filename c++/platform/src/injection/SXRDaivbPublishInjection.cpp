#include "SXRDaivbPublishInjection.hpp"
#include "SXRCommonMain.hpp"
#include "SendXReceive.hpp"

namespace sendxreceive
{
SXRDaivbPublishInjection::SXRDaivbPublishInjection(
    SendXReceive& app)
    : SXRCommonInj(app)
{
}

int SXRDaivbPublishInjection::handleData(std::string&& data)
{
    /*
    using atp = asremotedoor::packet::atp::RemoteDoorRequest;
    gateway::MessageId id = {atp::CONTENT_TYPE, atp::IID, atp::SID, atp::CID};

    m_app.onMessageBackend(
        id, 0, std::vector<uint8_t>(data.begin(), data.begin() + data.size()));
        */
    handlerDaivbPublish(std::move(data));
    return 0;
}

// injection from console app sxr_daivb_publish simulate "from SXR to ECU"
void SXRDaivbPublishInjection::handlerDaivbPublish(
    std::string&& data)
{
    m_app.m_context.logInfo() << "Receive DaivbPublish injection";

    SXRMessage sxrMes;
    if (parseInjectionData(data, sxrMes) != 0)
    {
        return;
    }

    HeaderVbId header = {0, m_app.m_seqId++, sxrMes.SID, sxrMes.CID};

    bool res = m_app.SendSXRMessageFromQueueToEcu({header, sxrMes.payload});

    if (res)
    {
        m_app.m_context.logInfo()
            << "handlerDaivbPublish::SendSXRMessageToEcu OK"
            << "SeqId:" << m_app.m_seqId.load() << sxrMes;
    }
    else
    {
        m_app.m_context.logError()
            << "handlerDaivbPublish::SendSXRMessageToEcu FAIL" << sxrMes;
    }
}

assistanceservice::DltInjector::Info
SXRDaivbPublishInjection::MakeInjectionInfo()
{
    using namespace std::placeholders;
    return assistanceservice::DltInjector::Info{
        SXRDaivbPublishInjection::serviceId,
        std::bind(&SXRDaivbPublishInjection::handleData, this, _1),
        SXRDaivbPublishInjection::name,
        SXRDaivbPublishInjection::helpStr};
}
}  // namespace sendxreceive