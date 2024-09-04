#ifndef SXRDAIVBPUBLISHINJECTION
#define SXRDAIVBPUBLISHINJECTION

#include "SXRCommonInj.hpp"

namespace sendxreceive
{
class SendXReceive;

class SXRDaivbPublishInjection : public SXRCommonInj
{
  public:
    explicit SXRDaivbPublishInjection(SendXReceive& app);
    virtual ~SXRDaivbPublishInjection() = default;

    static constexpr uint32_t serviceId =
        static_cast<uint32_t>(InjectionId::SXRDaivbPublishInjId);
    static constexpr char const* name =
        "Dlt injection for send x receive daivb publish";
    static constexpr char const* helpStr =
        "Injection \"SXRDaivbPublish\" with ID: 0x7053(hex) and 28755(decimal)\
             Injection to simulate sending Data from SXR to MCU\
             Usage: First two bytes are sid\
             Second two bytes are cid and then payload";

    int handleData(std::string&& data);

    assistanceservice::DltInjector::Info MakeInjectionInfo() override;

  private:
    void handlerDaivbPublish(std::string&& data);
};
}  // namespace sendxreceive

#endif /* REMOTEDOORRESPONSEINJECTION */