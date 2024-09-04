#ifndef SXRDAIVBINJECTION
#define SXRDAIVBINJECTION

#include "SXRCommonInj.hpp"

namespace sendxreceive
{
class SendXReceive;

class SXRDaivbInjection : public SXRCommonInj
{
  public:
    explicit SXRDaivbInjection(SendXReceive& app);
    virtual ~SXRDaivbInjection() = default;

    static constexpr uint32_t serviceId =
        static_cast<uint32_t>(InjectionId::SXRDaivbInjId);
    static constexpr char const* name =
        "Dlt injection for send x receive daivb";
    static constexpr char const* helpStr =
        "Injection \"SXRDaivb\" with ID: 0x7051(hex) and 28753(decimal)\
             Injection to simulate sending Data from Backend (DaiVB) to SXR\
             Usage: First two bytes are sid\
             Second two bytes are cid and then payload";

    int handleData(std::string&& data);
    assistanceservice::DltInjector::Info MakeInjectionInfo() override;

  private:
    void handlerVbToEcu(std::string&& data);
};
}  // namespace sendxreceive

#endif /* REMOTEDOORRESPONSEINJECTION */