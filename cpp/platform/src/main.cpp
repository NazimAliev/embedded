#include "SXRCommonMain.hpp"
#include "SendXReceive.hpp"
#include "SXRApplicationManagerClient.hpp"
#include "vsm/SXRVehicleSignalManager.hpp"
#include "vsm/SXRVehicleSignalsServiceClient.hpp"
#include "engmenu/SXREngMenuAdapter.hpp"
#include "version.h"

#include <PowerManagementClient.hpp>
#include <RpcBridgePoller.hpp>


DLT_DECLARE_CONTEXT(mainCtx)
DLT_DECLARE_CONTEXT(gateWaySvcClientCtx)
DLT_DECLARE_CONTEXT(consoleCtx)
DLT_DECLARE_CONTEXT(serviceContext)

void signalHandler(int sig)
{
    if (sig == SIGPOLL)
    {
        DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING("SIGPOLL received"));
    }
}

int main(int argc, char* argv[])
{
    using namespace sendxreceive;

    int retval = EXIT_SUCCESS;
    if (argc == 2 && argv[1] == std::string("-v"))
    {
        std::cout << rcsid << std::endl;
        return retval;
    }

    DLT_REGISTER_APP(g_appId, g_appDescription);
    DLT_REGISTER_CONTEXT(mainCtx, g_mainCtx, g_mainCtxDescription);
    DLT_REGISTER_CONTEXT(gateWaySvcClientCtx, g_gatewayCtx,
                         g_gatewayCtxDescription);
    DLT_REGISTER_CONTEXT(consoleCtx, g_consoleCtx, g_consoleCtxDescription);
    DLT_REGISTER_CONTEXT(serviceContext, "SRV", "Service");

    DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING(rcsid));
    DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING("SXR main"));
    DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING("DLT injection rules:"));
    DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING("AppId: "), DLT_STRING(g_appId),
            DLT_CSTRING("Context: "), DLT_STRING(g_mainCtx));
    DLT_LOG(
        mainCtx, DLT_LOG_INFO, DLT_CSTRING("ServiceId: "),
        DLT_HEX16(static_cast<uint32_t>(InjectionId::SXRVsmInjId)),
        DLT_CSTRING(": run IPC_SendSXRMessageToDaiVB"),
        DLT_CSTRING("TextData: SID CID Payload, App::CBMC example: 32 16 80 02 "
                    "09 08 07 06 05 04 03 02"));
    DLT_LOG(
        mainCtx, DLT_LOG_INFO, DLT_CSTRING("ServiceId: "),
        DLT_HEX16(static_cast<uint32_t>(InjectionId::SXRDaivbInjId)),
        DLT_CSTRING(": run GateWay RecvMsg callback"),
        DLT_CSTRING("TextData: SID CID Payload, App::CBMC example: 31 0A 80 02 "
                    "09 08 07 06 05 04 03 02"));
    uint8_t buf[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    DLT_LOG(consoleCtx, DLT_LOG_INFO, DLT_INT16(258), DLT_INT16(515),
            DLT_RAW(buf, sizeof(buf)));

    signal(SIGPOLL, signalHandler);
    // block to see dlt log when app is finished
    {
        assistanceservice::DltInjector injector{"dlt injections"};
        auto powerManagementClient =
            std::make_unique<assistanceservice::PowerManagementClient>(
                injector);

        auto vehicleSignalManager = std::make_unique<SXRVehicleSignalManager>(
        std::move(std::make_unique<SXRVehicleSignalsServiceClient>()));

        auto engMenuAdapter = std::make_unique<SXREngMenuAdapter>();

        SendXReceive service(std::move(powerManagementClient),
         std::move(vehicleSignalManager),  
         std::move(engMenuAdapter),
         injector);
        assistanceservice::RpcBridgePoller<SendXReceive> poller(service);
        SXRApplicationManagerClient am(poller);

        if (am.init(g_appName))
        {
            DLT_LOG(mainCtx, DLT_LOG_INFO,
                    DLT_CSTRING(" application manager client initialized, "
                                "start heartbeating..."));
            am.startHeartbeating();
            if (!service.init(g_appName))
            {
                DLT_LOG(mainCtx, DLT_LOG_ERROR,
                        DLT_CSTRING(" failed to initialize service"));
                retval = EXIT_FAILURE;
            }
            if (!poller.init())
            {
                DLT_LOG(mainCtx, DLT_LOG_ERROR,
                        DLT_CSTRING(" failed to initialize rpc bridge poller"));
                retval = EXIT_FAILURE;
            }
            DLT_LOG(mainCtx, DLT_LOG_INFO,
                    DLT_CSTRING(" starting main loop..."));
            poller.poll();
        }
        else
        {
            DLT_LOG(mainCtx, DLT_LOG_ERROR,
                    DLT_CSTRING(
                        " failed to initialize application manager client"));
            retval = EXIT_FAILURE;
        }
    }

    DLT_UNREGISTER_CONTEXT(mainCtx);
    DLT_UNREGISTER_CONTEXT(gateWaySvcClientCtx);
    DLT_UNREGISTER_CONTEXT(consoleCtx);
    DLT_UNREGISTER_CONTEXT(serviceContext);
    DLT_UNREGISTER_APP();
    return retval;
}
