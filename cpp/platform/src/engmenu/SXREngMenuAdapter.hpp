#ifndef SXR_ENGMENU_ADAPTER_HPP
#define SXR_ENGMENU_ADAPTER_HPP

#include <vector>
#include "dlt/Logger.hpp"
#include "EngMenuClient.hpp"

namespace sendxreceive
{
class SXREngMenuAdapter : public dlt::Context
{
public:
    SXREngMenuAdapter();

    bool init(const std::string& appName);
private:
    //Callback_WriteSXRData createWriteCallback();
    EngMenuClient m_engMenuClient;
};

} // namespace sendxreceive

#endif