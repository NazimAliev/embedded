// This program is based on generated_IPC/IPC_generated_mqsetup.cpp
#include <AsyncIPC-mqsetup.h>

namespace AsyncIPC
{
enum ProcessEnum : uint16_t
{
    PROCESS_INDEX_SXR,
    PROCESS_INDEX_RPCBRIDGE,

    PROCESS_COUNT
};

static constexpr const struct Process processes[] = {
    {"SXR", "", ""},
    {"rpcBridge", "", ""},
};

static constexpr const struct Queue queues[] = {
    //{10, 66560, PROCESS_INDEX_RPCBRIDGE, PROCESS_INDEX_SXR},
    {10, 2048, PROCESS_INDEX_RPCBRIDGE, PROCESS_INDEX_SXR},
    {10, 2048, PROCESS_INDEX_SXR, PROCESS_INDEX_RPCBRIDGE},
};

} /* end namespace AsyncIPC */

int main(const int, const char* const* const, const char* const* const)
{
    // remove mqueue nodes for testing purposes
    // if node not empty
    mq_unlink("/asyncipc-SXR-rpcBridge.mq\0");
    mq_unlink("/asyncipc-rpcBridge-SXR.mq\0");

    return AsyncIPC::create_all_queues(
        &AsyncIPC::processes[0],
        sizeof(AsyncIPC::processes) / sizeof(AsyncIPC::processes[0]),
        &AsyncIPC::queues[0],
        sizeof(AsyncIPC::queues) / sizeof(AsyncIPC::queues[0]));
}
