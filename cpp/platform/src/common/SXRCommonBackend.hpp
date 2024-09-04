#ifndef COMMON_BACKEND_HPP
#define COMMON_BACKEND_HPP

#include "GateWayClient.hpp"

namespace sendxreceive
{
typedef struct Config_S
{
    gateway::App app;
    std::string name;

} Config_S;

typedef const std::map<std::pair<uint16_t, uint16_t>, Config_S> Config;

// serviceId, msgType, appId

static Config g_configFromTcu = {
    {{0x3216, 0x8002}, {gateway::App::CBMC, "CBMC"}},
    {{0x5088, 0x8001}, {gateway::App::CROWDDATAIDC, "Crowd Data IDC"}},
    {{0x0505, 0x8003}, {gateway::App::FBS4ME, "FBS to me"}},
    {{0x5087, 0x8001}, {gateway::App::MAP, "MAP"}},
    {{0x3929, 0x8006}, {gateway::App::OFFBOARDCALC, "Off Board Calc"}},
    {{0x469D, 0x8001}, {gateway::App::PARKAVP, "Park AVP"}},
    {{0x469D, 0x8002}, {gateway::App::PARKRPA, "Park RPA"}},
    {{0x3908, 0x8009}, {gateway::App::SXRPNC, "SXRPNC"}},
    {{0x3939, 0x8001}, {gateway::App::SXRPNC, "SXRPNC"}},

    {{0x2591, 0x8002}, {gateway::App::CROWDDATALRR, "Crowd Data LRR"}},
    {{0x25CE, 0x0002}, {gateway::App::POS_BURST, "POS Burst"}},
   // {{ 0x25CE, 0x0001 }, { gateway::App::POSGCD, "POS GCD" }},
    {{0x3901, 0x8001}, {gateway::App::TMPT, "TMPT"}}
};

enum class VSMSignalType {Tx,  Rx};
struct VSMSignalConfig
{
    std::string path;
    std::pair<uint16_t, uint16_t> pduid;
    VSMSignalType type;
};

const std::vector<VSMSignalConfig> VsmConfig = {
    {"/12554/event/EI_MeCall_MaintOffset_Rq_ST3/EI_MeCall_MaintOffset_Rq_ST3", {0x310A,0x8002}, VSMSignalType::Tx},
    {"/12822/event/EI_MeCall_MaintOffset_Rs_ST3/EI_MeCall_MaintOffset_Rs_ST3", {0x3216, 0x8002}, VSMSignalType::Rx},
    
    { "/9682/event/UDC_IDC_M_PersonalData_Download_ST35/UDC_IDC_M_PersonalData_Download_ST35", {0x25D2, 0x8001}, VSMSignalType::Tx},
    {"/20616/event/UDC_IDC_M_PersonalData_Upload_ST35/UDC_IDC_M_PersonalData_Upload_ST35", {0x5088, 0x8001}, VSMSignalType::Rx},

    {"/268/event/CU_MAU_FBS4me_ST3/CU_MAU_FBS4me_ST3", {0x010C, 0x8005},  VSMSignalType::Tx},
    {"/1285/event/MAU_CU_FBS4me_ST3/MAU_CU_FBS4me_ST3", {0x0505, 0x8003},  VSMSignalType::Rx},

    {"/9701/event/Backend_MapData_Tile_ST35/DAP_Backend_MapData_Tile_ST35", {0x25E5, 0x8001}, VSMSignalType::Tx},
    {"/20615/event/Backend_MapData_Actv_ST35/DAP_Backend_MapData_Actv_ST35", {0x5087, 0x8001}, VSMSignalType::Rx},
 
    {"/14634/event/PT4_VEPM_RemoteFinalization_TA_Rs_ST3/PT4_VEPM_RemoteFinalization_TA_Rs_ST3", {0x392A, 0x8004}, VSMSignalType::Tx},
    {"/14633/event/PT4_VEPM_RemoteFinalization_TA_Rq_ST3/PT4_VEPM_RemoteFinalization_TA_Rq_ST3", {0x3929, 0x8006}, VSMSignalType::Rx},


    {"/18076/event/RdyRem_CU_PARK_AVP_ST35/RdyREM_CU_PARK_AVP_ST35", {0x469C, 0x8001}, VSMSignalType::Tx},
    {"/18077/event/RdyRem_PARK_CU_AVP_ST35/RdyREM_PARK_CU_AVP_ST35", {0x469D, 0x8001}, VSMSignalType::Rx},

    {"/18076/event/RdyRem_CU_PARK_RPA_ST35/RdyREM_CU_PARK_RPA_ST35", {0x469C, 0x8002}, VSMSignalType::Tx},
    {"/18077/event/RdyRem_PARK_CU_RPA_ST35/RdyREM_PARK_CU_RPA_ST35", {0x469D, 0x8002}, VSMSignalType::Rx},

    {"/14634/event/PT4_CMM_PlugAndChrg_TA_Rq_ST3/PT4_CMM_PlugAndChrg_TA_Rq_ST3", {0x392A, 0x8003}, VSMSignalType::Tx},
    {"/14600/event/PT4_CMM_PlugAndChrg_TA_Rs_ST3/PT4_CMM_PlugAndChrg_TA_Rs_ST3", {0x3908, 0x8009}, VSMSignalType::Rx},


    {"/14648/event/CIC_CU_SCC_ST35/CIC_CU_SCC_ST35", {0x3938, 0x8001}, VSMSignalType::Tx},
    {"/14649/event/CU_CIC_SCC_ST35/CU_CIC_SCC_ST35", {0x3939, 0x8001}, VSMSignalType::Rx},

};

}  // namespace sendxreceive

#endif
