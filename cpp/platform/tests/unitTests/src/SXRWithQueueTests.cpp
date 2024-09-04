#include <gtest/gtest.h>
#include "dlt/Context.hpp"
#include "dlt/injector/DltInjector.hpp"

#include <PowerManagementClient.hpp>
#include <ServiceAuthorization.hpp>
#include <SXRUtils.hpp>
#include <VehicleBackendClient.hpp>

#include <common/SXRTypes.hpp>
#include <mock/PowerManagementClientMock.hpp>
#include "../mocks/SXRWithQueueMock.hpp"
#include "../mocks/SXRQueueMock.hpp"

#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

DLT_DECLARE_CONTEXT(mainCtx)
DLT_DECLARE_CONTEXT(gateWaySvcClientCtx)
DLT_DECLARE_CONTEXT(consoleCtx)
DLT_DECLARE_CONTEXT(serviceContext)

namespace sxr_unit_test
{
using namespace sendxreceive;
using namespace assistanceservice;

TEST(SXRWithQueueTest, SendSXRMessageToEcu)
{
    using testing::_;
    using ::testing::Return;
    
    SXRDependencies dp;
    SXRWithQueueMock service(dp);
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

    auto queueVbToEcu = std::make_unique<SXRQueue<VbMessage>>("QueueVbToEcu");
    auto queueEcuToVb = std::make_unique<SXRQueue<EcuMessage>>("QueueEcuToVb");

    service.initQueues(std::move(queueVbToEcu), std::move(queueEcuToVb));

    VbMessage converted_msg =  SXRUtils::createVbMessage(msg, 0, data);
    EXPECT_CALL(service, SendSXRMessageFromQueueToEcu(converted_msg)).Times(1);

    EXPECT_CALL(service, isAuthorized("sXR"))
        .WillRepeatedly(testing::Return(true));

    service.onMessageBackend(msg, 0, data);
    std::this_thread::sleep_for(
        std::chrono::seconds(2));  // sleep for queues to work
    service.stop();
}

TEST(SXRWithQueueTest, IPC_SendSXRMessageToDaiVB)
{
    using testing::_;
    using ::testing::Return;

    SXRDependencies dp;
    SXRWithQueueMock service(dp);
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

    HeaderEcuId header = {0, 0x3216, 0x8002, 0};
    EcuMessage msg = {header, data};
    EXPECT_CALL(service, sendMessageFromQueueToBackend(msg)).Times(1);

    service.init("Sample");
    
    auto queueVbToEcu = std::make_unique<SXRQueue<VbMessage>>("QueueVbToEcu");
    auto queueEcuToVb = std::make_unique<SXRQueue<EcuMessage>>("QueueEcuToVb");

    service.initQueues(std::move(queueVbToEcu), std::move(queueEcuToVb));

    service.IPC_SendSXRMessageToDaiVB(0, 0x3216, 0x8002, data.data(),
                                      data.size());
    std::this_thread::sleep_for(
        std::chrono::seconds(2));  // sleep for queues to work
    service.stop();
}

TEST(SXRWithQueueTest, SendSXRMessageToEcu_authFail)
{
    using testing::_;
    using ::testing::Return;

    SXRDependencies dp;
    SXRWithQueueMock service(dp);
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
    
    auto queueVbToEcu = std::make_unique<SXRQueue<VbMessage>>("QueueVbToEcu");
    auto queueEcuToVb = std::make_unique<SXRQueue<EcuMessage>>("QueueEcuToVb");

    service.initQueues(std::move(queueVbToEcu), std::move(queueEcuToVb));


    EXPECT_CALL(service, isAuthorized("sXR"))
        .WillRepeatedly(testing::Return(false));  // auth fail

    VbMessage converted_msg = SXRUtils::createVbMessage(msg, 0, data);
    EXPECT_CALL(service, SendSXRMessageFromQueueToEcu(converted_msg)).Times(0);

    service.onMessageBackend(msg, 0, data);
    std::this_thread::sleep_for(
        std::chrono::seconds(2));  // sleep for queues to work
    service.stop();
}

TEST(SXRWithQueueTest, SendSXRMessageToEcu_GVNFail)
{
    using testing::_;
    using ::testing::Return;

    SXRDependencies dp;
    SXRWithQueueMock service(dp);
    auto powerManagementClient = dp.m_powerManagementClientPont;

    EXPECT_CALL(*powerManagementClient, isGvnAvailable())
        .WillRepeatedly(Return(false)); // // GVN wakeup fail
    EXPECT_CALL(*powerManagementClient, gvnWakeUpAndWait(_)).Times(1);

    EXPECT_CALL(service, init(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(service, getMessageReceiveProfile(_))
        .WillRepeatedly(Return(MsgWakeupRequiredState::WAKEUP_REQUIRED));

    std::vector<uint8_t> data{0x00, 0x01, 0x02, 0x03, 0x04,
                              0x05, 0x06, 0x07, 0x08};
    gateway::MessageId msg;
    msg.iid = 0;
    msg.contentType = gateway::ContentType::SXR;

    service.init("Sample");
    
    auto queueVbToEcu = std::make_unique<SXRQueue<VbMessage>>("QueueVbToEcu");
    auto queueEcuToVb = std::make_unique<SXRQueue<EcuMessage>>("QueueEcuToVb");

    service.initQueues(std::move(queueVbToEcu), std::move(queueEcuToVb));

    EXPECT_CALL(service, isAuthorized("sXR"))
        .WillRepeatedly(testing::Return(true));

    VbMessage converted_msg = SXRUtils::createVbMessage(msg, 0, data);
    EXPECT_CALL(service, SendSXRMessageFromQueueToEcu(converted_msg)).Times(0);

    service.onMessageBackend(msg, 0, data);
    std::this_thread::sleep_for(
        std::chrono::seconds(2));  // sleep for queues to work
    service.stop();
}

}  // namespace sxr_unit_test

//---------------------------------------------------------------------------
// End of File
//---------------------------------------------------------------------------
