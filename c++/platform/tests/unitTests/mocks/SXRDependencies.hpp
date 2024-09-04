#pragma once
#include <gmock/gmock.h>
#include <IVehicleBackendClient.hpp>
#include <SendXReceive.hpp>
#include <engmenu/SXREngMenuAdapter.hpp>

#include <mock/PowerManagementClientMock.hpp>
#include "../mocks/client/SXRVehicleSignalsServiceClientMock.hpp"
#include "../mocks/client/SXRVehicleSignalManagerMock.hpp"


namespace sxr_unit_test
{
using namespace sendxreceive;

class SXRDependencies
{
  public:
    SXRDependencies()        
    {
        m_powerManagementClient 
            = std::make_unique<assistanceservice::unit_test::PowerManagementClientMock>();

        m_VSM = std::make_unique<SXRVehicleSignalManagerMock>(
                std::move(std::make_unique<SXRVehicleSignalsServiceClientMock>()));

        m_engMenuAdapter = std::make_unique<SXREngMenuAdapter>();

        m_powerManagementClientPont = m_powerManagementClient.get();
        m_engMenuAdapterPoint = m_engMenuAdapter.get();
        m_VSMPoint = m_VSM.get();
    }
    public:
        assistanceservice::DltInjector injector {"dlt injections"};
        assistanceservice::unit_test::PowerManagementClientMock* m_powerManagementClientPont = nullptr;
        SXRVehicleSignalManagerMock *m_VSMPoint = nullptr;
        SXREngMenuAdapter *m_engMenuAdapterPoint = nullptr;



    public:
        std::unique_ptr<assistanceservice::unit_test::PowerManagementClientMock> m_powerManagementClient;
        std::unique_ptr<SXRVehicleSignalManagerMock> m_VSM;
        std::unique_ptr<SXREngMenuAdapter> m_engMenuAdapter;
    

};
}  // namespace sxr_unit_test