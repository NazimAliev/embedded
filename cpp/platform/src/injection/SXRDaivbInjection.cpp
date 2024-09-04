#include "SXRDaivbInjection.hpp"
#include <functional>
#include "SXRCommonMain.hpp"
#include "SendXReceive.hpp"

namespace sendxreceive
{
SXRDaivbInjection::SXRDaivbInjection(SendXReceive& app)
    : SXRCommonInj(app)
{
}

int SXRDaivbInjection::handleData(std::string&& data)
{
    /*
    using atp = asremotedoor::packet::atp::RemoteDoorRequest;
    gateway::MessageId id = {atp::CONTENT_TYPE, atp::IID, atp::SID, atp::CID};

    m_app.onMessageBackend(
        id, 0, std::vector<uint8_t>(data.begin(), data.begin() + data.size()));
        */
    handlerVbToEcu(std::move(data));
    return 0;
}

void SXRDaivbInjection::handlerVbToEcu(std::string&& data)
{
    m_app.m_context.logInfo() << "Receive injection from 'DaiVB'";

    SXRMessage sxrMes;
    if (parseInjectionData(data, sxrMes) != 0)
    {
        return;
    }

    gateway::MessageId msg;
    msg.contentType = gateway::ContentType::SXR;
    msg.iid = 0;
    msg.sid = sxrMes.SID;
    msg.cid = sxrMes.CID;

    m_app.onMessageBackend(msg, 0, sxrMes.payload);
    return;
}

assistanceservice::DltInjector::Info
SXRDaivbInjection::MakeInjectionInfo()
{
    using namespace std::placeholders;
    return assistanceservice::DltInjector::Info{
        SXRDaivbInjection::serviceId,
        std::bind(&SXRDaivbInjection::handleData, this, _1),
        SXRDaivbInjection::name,
        SXRDaivbInjection::helpStr};
}

}  // namespace sendxreceive