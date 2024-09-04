#pragma once
#include <gmock/gmock.h>
#include <IVehicleBackendClient.hpp>
#include <SendXReceive.hpp>

#include <mock/PowerManagementClientMock.hpp>
#include "../mocks/client/SXRVehicleSignalsServiceClientMock.hpp"
#include "../mocks/client/SXRVehicleSignalManagerMock.hpp"
#include "SXRDependencies.hpp"


namespace sxr_unit_test
{
using namespace sendxreceive;

class SXRHelperMock : public SendXReceive
{
  public:
    SXRHelperMock(SXRDependencies& dp) : sendxreceive::SendXReceive(
        std::move(dp.m_powerManagementClient),
        std::move(dp.m_VSM),
        std::move(dp.m_engMenuAdapter),
        dp.injector)
    {
    }
};
}  // namespace sxr_unit_test