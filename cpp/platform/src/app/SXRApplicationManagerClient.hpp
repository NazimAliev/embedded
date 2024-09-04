#ifndef REMOTEDOORAPPLICATIONMANAGERCLIENT
#define REMOTEDOORAPPLICATIONMANAGERCLIENT

#include <ApplicationManager.hpp>
#include <RpcBridgePoller.hpp>

#include "SendXReceive.hpp"

namespace sendxreceive
{
class SXRApplicationManagerClient
    : public assistanceservice::ApplicationManager
{
    using Poller = assistanceservice::RpcBridgePoller<SendXReceive>;

  public:
    explicit SXRApplicationManagerClient(Poller& poller);
    ~SXRApplicationManagerClient() override = default;

  protected:
    void onApplicationStop() override;

  private:
    Poller& m_poller;
};
}  // namespace sendxreceive

#endif /* REMOTEDOORAPPLICATIONMANAGERCLIENT */
