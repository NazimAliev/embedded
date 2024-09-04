#include "SXRCommonInj.hpp"

#include "SXRCommonMain.hpp"
#include "SXRCommonMessage.hpp"
#include "SendXReceive.hpp"

namespace sendxreceive
{
SXRCommonInj::SXRCommonInj(SendXReceive &app)
    : m_app(app)
{
}

int SXRCommonInj::parseInjectionData(const std::string &dataSrc,
                                                SXRMessage &mess)
{
    if (dataSrc.size() < 6)
    {
        m_app.m_context.logError() << "Injection payload length < 6, FAIL";
        return 1;
    }
    void *ptr = const_cast<char *>(dataSrc.data());
    uint8_t *data = static_cast<uint8_t *>(ptr);
    mess.SID = data[1] | data[0] << 8;
    mess.CID = data[3] | data[2] << 8;

    mess.payload.assign(&data[4], &data[dataSrc.size()]);

    m_app.m_context.logInfo() << "parseInjectionData "
                              << "SeqId:" << m_app.m_seqId.load() << mess;
    return 0;
}

int SXRCommonInj::parseInjectionDataVSM(const std::string& dataSrc, SXRVSMSignal& signal)
{
    if (dataSrc.size() < 4)
    {
        m_app.m_context.logError() << "Injection payload length for VSM modulation < 4, FAIL";
        return 1;
    }
    void *ptr = const_cast<char *>(dataSrc.data());
    uint8_t *data = static_cast<uint8_t *>(ptr);
    signal.ID = data[1] | data[0] << 8;

    signal.payload.assign(&data[2], &data[dataSrc.size()]);

    m_app.m_context.logInfo() << "parseInjectionData "
                              << "SeqId:" << m_app.m_seqId.load() << signal;
    return 0;
}

}  // namespace sendxreceive