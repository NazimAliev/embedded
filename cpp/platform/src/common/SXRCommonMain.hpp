#ifndef COMMON_MAIN_HPP
#define COMMON_MAIN_HPP

static constexpr char g_appId[] = "SXR";
static constexpr char g_appName[] = "send-x-receive";
static constexpr char g_appDescription[] = "SendXReceive";
static constexpr char g_mainCtx[] = "SXR";
static constexpr char g_mainCtxDescription[] = "Main function context";
static constexpr char g_gatewayCtx[] = "CSVC";
static constexpr char g_gatewayCtxDescription[] = "SXR client";
static constexpr char g_consoleCtx[] = "UCON";
static constexpr char g_consoleCtxDescription[] = "Utility console";

#pragma once
#include <memory>
namespace sendxreceive
{
enum class InjectionId : uint32_t
{
    // injection ID from dlt viewer simulate "from VSM to SXR"
    SXRVsmInjId = 0x7050,
    // injection ID from dlt viewer simulate "from DaiVB to SXR"
    SXRDaivbInjId = 0x7051,
    // injection ID from dlt viewer simulate "from SXR to DaiVB"
    SXREcuPublishInjId = 0x7052,
    // injection ID from  dlt viewer simulate "from SXR to ECU"
    SXRDaivbPublishInjId = 0x7053
};

}  // namespace sendxreceive

#endif
