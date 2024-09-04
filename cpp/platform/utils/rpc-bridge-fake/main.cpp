#include <memory>
#include "IPC_generated_base_rpcBridge.h"
#include "Injection.hpp"
#include "RpcFake.hpp"

static constexpr char g_appId[] = "RPCF";
static constexpr char g_appDescription[] = "RPC Bridge Fake";
static constexpr char g_mainCtx[] = "SXR";
static constexpr char g_mainCtxDescription[] = "Main function context";

DLT_DECLARE_CONTEXT(mainCtx);

int main(int argc, char* argv[])
{
    DLT_REGISTER_APP(g_appId, g_appDescription);
    DLT_REGISTER_CONTEXT(mainCtx, g_mainCtx, g_mainCtxDescription);

    DLT_LOG(mainCtx, DLT_LOG_INFO, DLT_CSTRING("RPC Bridge Fake main"));

    auto rpcFake = std::make_shared<RpcFake>();
    auto injection = std::make_shared<Injection>(rpcFake);

    injection->registration();
    rpcFake->recvFrom();
}
