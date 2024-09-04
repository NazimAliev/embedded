#include <gtest/gtest.h>
#include "dlt/Context.hpp"
#include "dlt/injector/DltInjector.hpp"

#include <PowerManagementClient.hpp>
#include <ServiceAuthorization.hpp>
#include <VehicleBackendClient.hpp>
#include <SXRUtils.hpp>


#include <common/SXRTypes.hpp>
#include <mock/PowerManagementClientMock.hpp>
#include "../mocks/SXRWithoutQueueMock.hpp"
#include "../mocks/SXRQueueMock.hpp"

#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>
#include <iostream>



namespace sxr_unit_test
{
using namespace sendxreceive;
using namespace assistanceservice;

TEST(SXRWithoutQueueTest, SendSXRMessageToEcu)
{
    using testing::_;
    using ::testing::Return;
    
    SXRDependencies dp;
    SXRWithoutQueueMock service(dp);
    auto powerManagementClient = dp.m_powerManagementClientPont;

    EXPECT_CALL(*powerManagementClient, isGvnAvailable())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*powerManagementClient, gvnWakeUpAndWait(_)).Times(0);


    EXPECT_CALL(service, init(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(service, getMessageReceiveProfile(_))
        .WillRepeatedly(Return(MsgWakeupRequiredState::WAKEUP_REQUIRED));

    std::vector<uint8_t> data{0x00, 0x01, 0x02, 0x03, 0x04,
                              0x05, 0x06, 0x07, 0x08};
    gateway::MessageId msg;
    msg.iid = 0;
    msg.contentType = gateway::ContentType::SXR;

    service.init("Sample");

    auto queueVbToEcu = std::make_unique<SXRQueueMock<VbMessage>>("QueueVbToEcu");
    auto queueEcuToVb = std::make_unique<SXRQueueMock<EcuMessage>>("QueueEcuToVb");

    service.initQueues(std::move(queueVbToEcu), std::move(queueEcuToVb));

    VbMessage converted_msg = SXRUtils::createVbMessage(msg, 0, data);
    EXPECT_CALL(service, SendSXRMessageFromQueueToEcu(converted_msg))
        .WillRepeatedly(
            testing::Invoke([&](const VbMessage &msg){
                bool result = service.SendXReceive::SendSXRMessageFromQueueToEcu(msg);
                EXPECT_FALSE(result);
                return result;
            }));
        
    EXPECT_CALL(service, isAuthorized("sXR"))
        .WillRepeatedly(testing::Return(true));

    service.onMessageBackend(msg, 0, data);
    std::this_thread::sleep_for(
        std::chrono::seconds(2));  // sleep for queues to work
    service.stop();
}

TEST(SXRWithoutQueueTest, IPC_SendSXRMessageToDaiVB)
{
    using testing::_;
    using ::testing::Return;

        
    SXRDependencies dp;
    SXRWithoutQueueMock service(dp);
    auto powerManagementClient = dp.m_powerManagementClientPont;

    EXPECT_CALL(*powerManagementClient, isGvnAvailable())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*powerManagementClient, gvnWakeUpAndWait(_)).Times(0);


    EXPECT_CALL(service, isAuthorized("sXR"))
        .WillRepeatedly(testing::Return(true));  // auth fail
    EXPECT_CALL(service, init(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(service, getMessageReceiveProfile(_))
        .WillRepeatedly(Return(MsgWakeupRequiredState::WAKEUP_REQUIRED));

    std::vector<uint8_t> data{0x00, 0x01, 0x02, 0x03, 0x04,
                              0x05, 0x06, 0x07, 0x08};

    EXPECT_CALL(service, sendMessage(_, data)).Times(1);

    service.init("Sample");
    
    auto queueVbToEcu = std::make_unique<SXRQueueMock<VbMessage>>("QueueVbToEcu");
    auto queueEcuToVb = std::make_unique<SXRQueueMock<EcuMessage>>("QueueEcuToVb");

    service.initQueues(std::move(queueVbToEcu), std::move(queueEcuToVb));

    service.IPC_SendSXRMessageToDaiVB(0, 0x3216, 0x8002, data.data(),
                                      data.size());
    service.stop();
}

// Just for better coverage
TEST(SXRWithoutQueueTest, IPC_SendSXRMessageToDaiVB_unregistered)
{
    using testing::_;
    using ::testing::Return;

    SXRDependencies dp;
    SXRWithoutQueueMock service(dp);
    auto powerManagementClient = dp.m_powerManagementClientPont;

    EXPECT_CALL(*powerManagementClient, isGvnAvailable())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*powerManagementClient, gvnWakeUpAndWait(_)).Times(0);

    EXPECT_CALL(service, isAuthorized("sXR"))
        .WillRepeatedly(testing::Return(true));  // auth fail
    EXPECT_CALL(service, init(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(service, getMessageReceiveProfile(_))
        .WillRepeatedly(Return(MsgWakeupRequiredState::WAKEUP_REQUIRED));

    std::vector<uint8_t> data{0x00, 0x01, 0x02, 0x03, 0x04,
                              0x05, 0x06, 0x07, 0x08};

    EXPECT_CALL(service, sendMessage(_, data)).Times(1);

    service.init("Sample");
    
    auto queueVbToEcu = std::make_unique<SXRQueueMock<VbMessage>>("QueueVbToEcu");
    auto queueEcuToVb = std::make_unique<SXRQueueMock<EcuMessage>>("QueueEcuToVb");

    service.initQueues(std::move(queueVbToEcu), std::move(queueEcuToVb));

    service.IPC_SendSXRMessageToDaiVB(0, 0x5901, 0x5901, data.data(),
                                      data.size());
    service.stop();
}

}  // namespace sxr_unit_test


//---------------------------------------------------------------------------
// End of File
//---------------------------------------------------------------------------
