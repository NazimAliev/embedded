#include <gtest/gtest.h>
#include "dlt/Context.hpp"

#include <PowerManagementClient.hpp>
#include <ServiceAuthorization.hpp>
#include <VehicleBackendClient.hpp>

#include <common/SXRTypes.hpp>
#include <mock/PowerManagementClientMock.hpp>
#include "../mocks/SXRInitFunctionMock.hpp"

#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>



namespace sxr_unit_test
{
using namespace sendxreceive;
using namespace assistanceservice;

TEST(SXRInitFunctionTests, initVehicleBackendClient_fail)
{
    using testing::_;
    using ::testing::Return;

    SXRDependencies dp;
    SXRInitFunctionMock service(dp);

    EXPECT_CALL(service, initVehicleBackendClient("send-x-receive")).WillRepeatedly(Return(false));

    EXPECT_FALSE(service.init("send-x-receive"));
    service.stop();
}

TEST(SXRInitFunctionTests, initServiceAuthorization_fail)
{
    using testing::_;
    using ::testing::Return;

    SXRDependencies dp;
    SXRInitFunctionMock service(dp);

    EXPECT_CALL(service, initVehicleBackendClient("send-x-receive")).WillRepeatedly(Return(true));
    EXPECT_CALL(service, initServiceAuthorization("send-x-receive")).WillRepeatedly(Return(false));

    EXPECT_FALSE(service.init("send-x-receive"));
    service.stop();
}

TEST(SXRInitFunctionTests, initPowerManagement_fail)
{
    using testing::_;
    using ::testing::Return;

    SXRDependencies dp;
    SXRInitFunctionMock service(dp);

    EXPECT_CALL(service, initVehicleBackendClient("send-x-receive")).WillRepeatedly(Return(true));
    EXPECT_CALL(service, initServiceAuthorization("send-x-receive")).WillRepeatedly(Return(true));
    EXPECT_CALL(service, initPowerManagement("send-x-receive")).WillRepeatedly(Return(false));

    EXPECT_FALSE(service.init("send-x-receive"));
    service.stop();
}

TEST(SXRInitFunctionTests, init)
{
    using testing::_;
    using ::testing::Return;


    SXRDependencies dp;
    SXRInitFunctionMock service(dp);

    EXPECT_CALL(service, initVehicleBackendClient("send-x-receive")).WillRepeatedly(Return(true));
    EXPECT_CALL(service, initServiceAuthorization("send-x-receive")).WillRepeatedly(Return(true));
    EXPECT_CALL(service, initPowerManagement("send-x-receive")).WillRepeatedly(Return(true));
    EXPECT_CALL(service, initVehicleSignalManager("send-x-receive")).WillRepeatedly(Return(true));

    EXPECT_CALL(service, subscribeToSignalsVSM(_)).WillRepeatedly(Return(true));

    EXPECT_TRUE(service.init("send-x-receive"));
    service.stop();
}

}  // namespace sxr_unit_test


//---------------------------------------------------------------------------
// End of File
//---------------------------------------------------------------------------
