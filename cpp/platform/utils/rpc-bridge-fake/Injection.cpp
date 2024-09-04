#include "Injection.hpp"
#include "CommonMain.hpp"

DLT_IMPORT_CONTEXT(mainCtx);

Injection::Injection(std::shared_ptr<RpcFake> rpcFake) : m_rpcFake(rpcFake) {}

Injection::~Injection() {}

int Injection::callback(uint32_t serviceId, void* data, uint32_t length)
{
    helper(serviceId, data, length);
    return 0;
}

void Injection::registration()
{
    dlt::Context::registerInjectionCallback(g_ecuInjId, &callback);
    helper = [this](const uint32_t serviceId, const void* data,
                    const uint32_t length) -> int {
        handler(serviceId, static_cast<const uint8_t*>(data), length);
        return 0;
    };
    DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING("registerInjectionCallback"));
}

helper_t Injection::helper = nullptr;

int Injection::handler(const uint32_t serviceId, const uint8_t* data,
                       const uint32_t length)
{
    uint16_t seqid;
    uint16_t sid;
    uint16_t cid;

    DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING("Get injection, serviceId: "),
            DLT_INT(serviceId));
    if (length < 6)
    {
        DLT_LOG(mainCtx, DLT_LOG_ERROR,
                DLT_CSTRING("Wrong injection format, len: "), DLT_INT(length));
        return 0;
    }
    seqid = data[1] | data[0] << 8;
    sid = data[3] | data[2] << 8;
    cid = data[5] | data[4] << 8;

    DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING("seqID: "), DLT_INT(seqid),
            DLT_CSTRING("sid: "), DLT_INT(sid), DLT_CSTRING("cid: "),
            DLT_INT(cid);
            DLT_CSTRING("payload length: "), DLT_INT(length - 6));

    // uint16_t seqID, uint16_t SID, uint16_t CID, uint8_t* PayloadData, size_t
    // PayloadData_LENGTH
    m_rpcFake->sendTo(seqid, sid, cid, data + 6, length - 6);
    return 0;
}
