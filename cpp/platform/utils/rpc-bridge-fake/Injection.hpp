#ifndef INJECTION_HPP
#define INJECTION_HPP

#include "RpcFake.hpp"

typedef std::function<int(const uint32_t serviceId, const void* data,
                          const uint32_t length)>
    helper_t;

class Injection : public dlt::Context
{
  public:
    Injection(std::shared_ptr<RpcFake> rpcFake);

    ~Injection();
    static int callback(uint32_t serviceId, void* data, uint32_t length);
    int handler(const uint32_t serviceId, const uint8_t* data,
                const uint32_t length);
    static helper_t helper;

    void registration();

  private:
    std::shared_ptr<RpcFake> m_rpcFake;
};

#endif
