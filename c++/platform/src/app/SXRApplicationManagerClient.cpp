#include "SXRApplicationManagerClient.hpp"
#include <RpcBridgePoller.hpp>

namespace sendxreceive
{
SXRApplicationManagerClient::SXRApplicationManagerClient(
    Poller& poller)
    : m_poller(poller)
{
}

void SXRApplicationManagerClient::onApplicationStop()
{
    m_poller.stop();
}

}  // namespace sendxreceive
