#include <iomanip>
#include <ios>
#include <iostream>
#include <string>
#include <vector>
#include "SXRCommonMain.hpp"
#include "dlt_client.h"
//#include "injections/InjectionTypes.hpp"

int argvToBuffer(const std::string argvMessageId, const std::string argvData,
                 std::vector<uint8_t>& out);
int bufferToMsg(const uint8_t* buffer, const size_t bufferSize, uint16_t& sid,
                uint16_t& cid, std::vector<uint8_t>& data);
/*
    Channel-ID/Message Type/Event ID/CID
    */

DltClient client;
DltFilter filter;
DltReturnValue res;
char* appid = const_cast<char*>(g_appId);
char* ctx = const_cast<char*>(g_mainCtx);

uint16_t msgId[2];

/***** INIT *****/

DltReturnValue init(DltClient* client, char* servIp)
{
    DltReturnValue res;

    res = dlt_client_init(client, false);
    if (res != DLT_RETURN_OK)
    {
        std::cout << "Init fail" << res << std::endl;
        return res;
    }
    dlt_client_set_mode(client, DLT_CLIENT_MODE_TCP);
    dlt_client_set_server_ip(client, servIp);
    res = dlt_client_connect(client, false);
    if (res != DLT_RETURN_OK)
    {
        std::cout << "Connect to DLT daemon fail" << std::endl;
        return res;
    }
    return res;
}

/***** PUBLISH *****/

void publish(int injectionId, std::string argvMessageId, std::string argvData)
{
    std::vector<uint8_t> out;
    argvToBuffer(argvMessageId, argvData, out);

    res = dlt_client_send_inject_msg(&client, appid, ctx, injectionId,
                                     out.data(), out.size());
    if (res != DLT_RETURN_OK)
    {
        std::cout << "Send injection fail" << std::endl;
    }
    dlt_client_cleanup(&client, false);
}

/***** CALLBACK *****/

// catch all log messages
int callback(DltMessage* message, void* data)
{
    uint16_t* p = static_cast<uint16_t*>(data);

    DltReturnValue ret = dlt_message_filter_check(message, &filter, false);
    if (ret == 0)
    {
        return 0;
    }

    std::string ecu = std::string(message->headerextra.ecu, DLT_ID_SIZE);
    std::string apid = std::string(message->extendedheader->apid, DLT_ID_SIZE);
    std::string ctid = std::string(message->extendedheader->ctid, DLT_ID_SIZE);
    std::cerr << "Got message AppId: " << apid << " ContextId: " << ctid
              << " ECU: " << ecu << std::endl;
    // std::cout << "headersize: " << message->headersize << " datasize: " <<
    // message->datasize << std::endl;

    uint16_t sid;
    uint16_t cid;
    std::vector<uint8_t> v;
    bufferToMsg(message->databuffer, message->datasize, sid, cid, v);
    if (sid == p[0] && cid == p[1])
    {
        std::cerr << "Catch SID: " << sid << " CID: " << cid
                  << ", payload size: " << v.size() << std::endl;
        std::cerr << "Payload:" << std::endl;
        for (unsigned char x : v)
        {
            if (x < 16) std::cout << "0";

            std::cout << std::hex << std::uppercase << static_cast<int>(x);
        }
        std::cerr << std::endl;

        dlt_filter_free(&filter, false);
        dlt_message_free(message, false);
        dlt_client_cleanup(&client, false);
    }
    else
    {
        std::cerr << "Catch SID:CID " << sid << ":" << cid
                  << " doesn't match, skip" << std::endl;
        std::cerr << "Should be " << p[0] << ":" << p[1] << std::endl;
    }

    return 0;
}

/***** LISTEN *****/

void listen(std::string argvMessageId)
{
    dlt_filter_init(&filter, false);
    dlt_filter_add(&filter, g_appId, g_consoleCtx, false);
    dlt_client_register_message_callback(callback);

    try
    {
        // getting sid, cid from command line
        auto pos = argvMessageId.find(":");
        msgId[0] = std::stoul(argvMessageId.substr(0, pos), nullptr, 16);
        msgId[1] = std::stoul(
            argvMessageId.substr(pos + 1, argvMessageId.size()), nullptr, 16);
    }
    catch (...)
    {
        std::cerr << "Uncorrect data" << std::endl;
        return;
    }
    dlt_client_main_loop(&client, msgId, false);
    std::cerr << "End listen loop " << std::endl;
}

/***** USAGE *****/

void usage()
{
    std::cout << "Usage:\n" << std::endl;
    std::cout << "sxr_ecu_publish SID:CID data ipaddr" << std::endl;
    std::cout << "example: sxr_ecu_publish 000F:00A7 A3125F68 127.0.0.1\n"
              << std::endl;

    std::cout << "sxr_ecu_listen SID:CID" << std::endl;
    std::cout << "example: sxr_ecu_listen 000F:00A7\n" << std::endl;

    std::cout << "sxr_daivb_publish SID:CID data ipaddr" << std::endl;
    std::cout << "example: sxr_daivb_publish 000F:00A7 A3125F68 127.0.0.1\n"
              << std::endl;

    std::cout << "sxr_daivb_listen SID:CID" << std::endl;
    std::cout << "example: sxr_daivb_listen 000F:00A7\n" << std::endl;
}

/***** MAIN *****/
/***** **** *****/

int main(int argc, char* argv[])
{
    const int offset = 10;
    char local[] = "127.0.0.1";
    auto argv0 = std::string(argv[0]);
    if (offset >= argv0.size())
    {
        usage();
        return 0;
    }
    auto command = argv0.substr(argv0.size() - offset);
    if (command == "cu_publish" && argc == 4)
    {
        init(&client, argv[3]);
        uint32_t ecuPublishInjId = static_cast<uint32_t>(
            sendxreceive::InjectionId::SXREcuPublishInjId);

        publish(ecuPublishInjId, argv[1], argv[2]);
    }
    else if (command == "ecu_listen" && argc == 2)
    {
        init(&client, local);
        listen(argv[1]);
    }
    else if (command == "vb_publish" && argc == 4)
    {
        init(&client, argv[3]);
        uint32_t daivbPublishInjId = static_cast<uint32_t>(
            sendxreceive::InjectionId::SXRDaivbPublishInjId);
        publish(daivbPublishInjId, argv[1], argv[2]);
    }
    else if (command == "ivb_listen" && argc == 2)
    {
        init(&client, local);
        listen(argv[1]);
    }
    else
    {
        usage();
    }
    return 0;
}
