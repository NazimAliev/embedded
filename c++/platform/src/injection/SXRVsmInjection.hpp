#ifndef SXRECUINJECTION
#define SXRECUINJECTION

#include "SXRCommonInj.hpp"
#include <VehicleSignalsServiceClient.hpp>
namespace sendxreceive
{
class SendXReceive;

class SXRVsmInjection : public SXRCommonInj
{
  public:
    explicit SXRVsmInjection(SendXReceive& app);
    virtual ~SXRVsmInjection() = default;

    static constexpr uint16_t serviceId =
        static_cast<uint16_t>(InjectionId::SXRVsmInjId);
    static constexpr char const* name =
        "Dlt injection for send x receive ecu";
    static constexpr char const* helpStr =
        "Injection \"SXREcu\" with ID: 0x7050(hex) and 28752(decimal)\
            Injection to simulate sending Data from MCU to SXR\
            Usage: First two bytes are VSM signal ID and then payload";

    int handleData(std::string&& data);
    assistanceservice::DltInjector::Info MakeInjectionInfo() override;

  private:
    void handlerEcuToVb(std::string&& data);
    VehicleSignalsServiceClient::VehicleSignalsSelectiveEventCallback m_callback = nullptr;
};
}  // namespace sendxreceive

#endif /* REMOTEDOORRESPONSEINJECTION */