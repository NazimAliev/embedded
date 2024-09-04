#include "RpcFake.hpp"

//${DLT_WRAPPER_SOURCES}

DLT_IMPORT_CONTEXT(mainCtx);

static constexpr char g_appName[] = "RPCF";
static constexpr char g_appDescription[] = "RPC Bridge Fake";
static constexpr char g_mainCtx[] = "MAIN";
static constexpr char g_mainCtxDescription[] = "Main function context";

RpcFake::RpcFake() { open_message_queues(m_poller); }

RpcFake::~RpcFake() { close_message_queues(); }

void RpcFake::IPC_SendSXRMessageToEcu(uint16_t seqID, uint16_t SID,
                                      uint16_t CID, uint8_t* PayloadData_ptr,
                                      size_t PayloadData_LENGTH)
{
    DLT_LOG(mainCtx, DLT_LOG_INFO,
            DLT_CSTRING("Receive IPC_SendSXRMessageToEcu"),
            DLT_STRING("seqID: "), DLT_INT(seqID), DLT_STRING("SID: "),
            DLT_INT(SID), DLT_STRING("CID: "), DLT_INT(CID),
            DLT_STRING("PayloadData_LENGTH: "), DLT_INT(PayloadData_LENGTH));
}

void RpcFake::sendTo(uint16_t seqID, uint16_t SID, uint16_t CID,
                     const uint8_t* PayloadData, size_t PayloadData_LENGTH)
{
    // bool IPC_SendSXRMessageToDaiVB(uint16_t seqID, uint16_t SID, uint16_t
    // CID, uint32_t TransactionId, uint8_t* PayloadData, size_t
    // PayloadData_LENGTH)
    bool res;
    res = IPC_SendSXRMessageToDaiVB(
        seqID, SID, CID, const_cast<uint8_t*>(PayloadData), PayloadData_LENGTH);
    DLT_LOG(mainCtx, DLT_LOG_INFO,
            DLT_CSTRING("Send IPC_SendSXRMessageToDaiVB"),
            DLT_STRING("Result: "), DLT_INT(res));
}

void RpcFake::recvFrom()
{
    for (int i = 0; i < 10; ++i)
    {
        m_poller.poll_events(1, -1);
        DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING("Receive poll event"));
    }
}
