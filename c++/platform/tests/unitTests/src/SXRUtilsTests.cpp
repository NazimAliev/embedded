#include <gtest/gtest.h>
#include "dlt/Context.hpp"
#include "dlt/injector/DltInjector.hpp"


#include <SXRUtils.hpp>

#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

namespace sxr_unit_test
{
using namespace sendxreceive;

TEST(SXRSXRUtilsTests, EcuMessage)
{
    // Test
    std::vector<uint8_t> data{0x00, 0x01, 0x02, 0x03, 0x04,
                              0x05, 0x06, 0x07, 0x08};

    EcuMessage msg = SXRUtils::createEcuMessage(0, 1, 2, 3, data.data(), data.size());

    HeaderEcuId header = msg.header;
    std::vector<uint8_t> payload = msg.payload;

    EXPECT_EQ(header.seqID, 0);
    EXPECT_EQ(header.SID, 1);
    EXPECT_EQ(header.CID, 2);
    EXPECT_EQ(header.TransactionId, 3);
    EXPECT_EQ(payload, data);
}

TEST(SXRSXRUtilsTests, VbMessage)
{
    std::vector<uint8_t> data{0x00, 0x01, 0x02, 0x03, 0x04,
                              0x05, 0x06, 0x07, 0x08};
    gateway::MessageId msg;
    msg.iid = 55;
    msg.sid = 44;
    msg.cid = 33;
    msg.contentType = gateway::ContentType::SXR;

    VbMessage converted_msg = SXRUtils::createVbMessage(msg, 0, data);
    HeaderVbId header = converted_msg.header;
    auto payload = converted_msg.payload;
    EXPECT_EQ(header.iid, 55);
    EXPECT_EQ(header.seqID, 0);
    EXPECT_EQ(header.SID, 44);
    EXPECT_EQ(header.CID, 33);
    EXPECT_EQ(payload, data);
}
}  // namespace sxr_unit_test

//---------------------------------------------------------------------------
// End of File
//---------------------------------------------------------------------------
