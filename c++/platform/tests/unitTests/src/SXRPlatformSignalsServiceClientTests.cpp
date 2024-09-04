#include <gtest/gtest.h>

#include "vsm/SXRVehicleSignalManager.hpp"
#include "../mocks/client/SXRVehicleSignalsServiceClientMock.hpp"

namespace sxr_unit_test
{
using namespace sendxreceive;

TEST(SXRVehicleSignalsServiceClientTests, init)
{
    using testing::_;
    using ::testing::Return;
    using ::testing::DoAll;

    std::unique_ptr<SXRVehicleSignalsServiceClientMock> client = 
                                        std::make_unique<SXRVehicleSignalsServiceClientMock>();
    SXRVehicleSignalsServiceClientMock *c = client.get(); 
    SXRVehicleSignalManager vsm(std::move(client));

    std::string appName("send-x-receive");

    EXPECT_CALL(*c, init(appName)).WillOnce(Return(false));
    EXPECT_FALSE(vsm.init(appName));

    //================================================
    EXPECT_CALL(*c, init(appName)).WillOnce(Return(true));
    EXPECT_CALL(*c, requestVehicleSignals(_,_,_,_)).WillOnce(Return(false));
    EXPECT_FALSE(vsm.init(appName));

    //=================================================================
    EXPECT_CALL(*c, init(appName)).WillOnce(Return(true));
    std::vector<VehicleSignalsServiceClient::SignalIdentifier> ids = {0,1};
    EXPECT_CALL(*c, requestVehicleSignals(_,_,_,_))
                        .WillOnce(DoAll(testing::SetArgReferee<3>(ids), Return(true)));
    EXPECT_TRUE(vsm.init(appName));
}

TEST(SXRVehicleSignalsServiceClientTests,SendSXRMessageToVsm)
{
    using testing::_;
    using ::testing::Return;
    using ::testing::DoAll;

    std::unique_ptr<SXRVehicleSignalsServiceClientMock> client = 
                                        std::make_unique<SXRVehicleSignalsServiceClientMock>();
    SXRVehicleSignalsServiceClientMock *c = client.get(); 
    SXRVehicleSignalManager vsm(std::move(client));

    std::string appName("send-x-receive");

    EXPECT_CALL(*c, init(appName)).WillOnce(Return(true)); 
    std::vector<VehicleSignalsServiceClient::SignalIdentifier> ids = {0,1};
    EXPECT_CALL(*c, requestVehicleSignals(_,_,_,_))
                        .WillOnce(DoAll(testing::SetArgReferee<3>(ids), Return(true)));
    EXPECT_TRUE(vsm.init(appName));

    
    VbMessage msg;
    msg.header.CID = 0x8002;
    msg.header.SID = 0x0000;
    EXPECT_FALSE(vsm.SendSXRMessageToVsm(msg));

    //=================================
    msg.header.SID = 0x310A;
    EXPECT_CALL(*c, setAndSendSignals(_,_,_)).WillOnce(Return(false));
    EXPECT_FALSE(vsm.SendSXRMessageToVsm(msg));

    //=================================
    EXPECT_CALL(*c, setAndSendSignals(_,_,_)).WillOnce(Return(true));
    EXPECT_TRUE(vsm.SendSXRMessageToVsm(msg));

}

TEST(SXRVehicleSignalsServiceClientTests, subscribeToSignals)
{
    using testing::_;
    using ::testing::Return;
    using ::testing::DoAll;

    std::unique_ptr<SXRVehicleSignalsServiceClientMock> client = 
                                        std::make_unique<SXRVehicleSignalsServiceClientMock>();
    SXRVehicleSignalsServiceClientMock *c = client.get(); 
    SXRVehicleSignalManager vsm(std::move(client));

    std::string appName("send-x-receive");

    EXPECT_CALL(*c, init(appName)).WillOnce(Return(true)); 
    std::vector<VehicleSignalsServiceClient::SignalIdentifier> ids = {0,1};
    EXPECT_CALL(*c, requestVehicleSignals(_,_,_,_))
                        .WillOnce(DoAll(testing::SetArgReferee<3>(ids), Return(true)));
    EXPECT_TRUE(vsm.init(appName));

      auto callback = [this](
                uint16_t seqID, uint16_t SID,
                uint16_t CID, uint8_t *PayloadData_ptr,
                size_t PayloadData_LENGTH)
        { (void)seqID; (void)SID; (void)CID; (void)PayloadData_ptr; (void)PayloadData_LENGTH;    };

    //===============
    EXPECT_CALL(*c, subscribeVehicleSignals(_,_,_)).WillOnce(Return(false));
    EXPECT_FALSE(vsm.subscribeToSignals(callback));

    //====================================
    EXPECT_CALL(*c, subscribeVehicleSignals(_,_,_)).WillOnce(Return(true));
    EXPECT_CALL(*c, subscribeToVehicleSignalsSelectiveEvent(_)).WillOnce(Return(false));
    EXPECT_FALSE(vsm.subscribeToSignals(callback));

    //====================================
    EXPECT_CALL(*c, subscribeVehicleSignals(_,_,_)).WillOnce(Return(true));
    EXPECT_CALL(*c, subscribeToVehicleSignalsSelectiveEvent(_)).WillOnce(Return(true));
    EXPECT_TRUE(vsm.subscribeToSignals(callback));
}

}  // namespace sxr_unit_test

//---------------------------------------------------------------------------
// End of File
//---------------------------------------------------------------------------
