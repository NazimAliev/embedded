#ifndef RPC_FAKE_HPP
#define RPC_FAKE_HPP

#include <stdint.h>
#include <memory>
#include "IPC_generated_base_rpcBridge.h"
#include "dlt/Logger.hpp"
#include "dlt/dlt.h"

class RpcFake : public AsyncIPC::IPC_generated_base_rpcBridge
{
  public:
    RpcFake();
    ~RpcFake();
    void sendTo(uint16_t seqID, uint16_t SID, uint16_t CID,
                const uint8_t* PayloadData, size_t PayloadData_LENGTH);
    void recvFrom();

  private:
    AsyncIPC::AsyncEventPoll m_poller;

    virtual void IPC_SendSXRMessageToEcu(uint16_t seqID, uint16_t SID,
                                         uint16_t CID, uint8_t* PayloadData_ptr,
                                         size_t PayloadData_LENGTH) override;
};

#endif
