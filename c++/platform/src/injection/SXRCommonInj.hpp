#ifndef SEND_X_RECEIVE_COMMON_INJ
#define SEND_X_RECEIVE_COMMON_INJ

#include <iomanip>

#include <DltInjector.hpp>
#include "SXRCommonMain.hpp"
#include "SXRCommonMessage.hpp"
#include "SXRCommonInj.hpp"

namespace sendxreceive
{
class SendXReceive;

class SXRCommonInj
{
  public:
    SXRCommonInj(SendXReceive& app);

    int parseInjectionData(const std::string& data, SXRMessage& mess);
    int parseInjectionDataVSM(const std::string& data, SXRVSMSignal& signal);
    virtual assistanceservice::DltInjector::Info MakeInjectionInfo() = 0;

  protected:
    SendXReceive& m_app;
};

}  // namespace sendxreceive

#endif
