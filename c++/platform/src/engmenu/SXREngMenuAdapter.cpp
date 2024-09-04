#include "SXREngMenuAdapter.hpp"

namespace sendxreceive
{

SXREngMenuAdapter::SXREngMenuAdapter() :
    dlt::Context("MENU", "EngMenu adapter")
{
}

bool SXREngMenuAdapter::init(const std::string& appName)
{
    if (!m_engMenuClient.initAsync(appName))
    {
        LOG(dlt::LogLevel::ERROR) << __LOGHEAD__ 
            << "Failed to initialize EngMenu client.";
        return false;
    } 
/*
    if (!m_engMenuClient.subscribeToWriteSXRData([this](EngMenuClient::Direction_t direction, uint16_t message_id){
        (void) direction;
        (void) message_id;
    }))
    {

    }
    */
    
    return true;

}
/*
Callback_WriteSXRData SXREngMenuAdapter::createWriteCallback()
{
    return [this](Direction_t direction , uint16_t message_id){
        return;
    };
}
*/
} // namespace r