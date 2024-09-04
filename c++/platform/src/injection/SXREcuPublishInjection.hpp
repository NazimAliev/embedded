#ifndef SXRECUPUBLISHINJECTION
#define SXRECUPUBLISHINJECTION

#include "SXRCommonInj.hpp"

namespace sendxreceive
{
class SendXReceive;

class SXREcuPublishInjection : public SXRCommonInj
{
  public:
    SXREcuPublishInjection(SendXReceive& app);

    static constexpr uint32_t serviceId =
        static_cast<uint32_t>(InjectionId::SXREcuPublishInjId);
    static constexpr char const* name =
        "Dlt injection for send x receive ecu publish";
    static constexpr char const* helpStr =
        "Injection \"SXREcu\" with ID: 0x7052(hex) and 28754(decimal)\
            Injection to simulate sending Data from SXR to Backend (DaiVB\
            Usage: First two bytes are sid\
            Second two bytes are cid and then payload";

    int handleData(std::string&& data);
    assistanceservice::DltInjector::Info MakeInjectionInfo() override;

  private:
    void handlerEcuPublish(std::string&& data);
};
}  // namespace sendxreceive

#endif /* REMOTEDOORRESPONSEINJECTION */