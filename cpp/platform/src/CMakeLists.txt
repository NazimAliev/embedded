#===================================================================#
# This program is the confidential and proprietary product of Valeo #
# Comfort and Driving Assistance Systems. Any unauthorised use,     #
# reproduction or transfer of this program is strictly prohibited.  #
#                                                                   #
# Copyright (C) 2021 Valeo Comfort and Driving Assistance Systems.  #
# (Subject to limited distribution and restricted disclosure only.) #
#===================================================================#

# Version Requirement
cmake_minimum_required(VERSION 3.2)

# Target Name
project("send-x-receive")

set(RPC_DIR "${CMAKE_SOURCE_DIR}/../../sdk/rpc/Src/IPC" CACHE PATH "RPC Bridge includes directory location")
message(STATUS "RPC_DIR_FOR_SXR: ${RPC_DIR}")

add_executable(${PROJECT_NAME}
    main.cpp
    SendXReceive.cpp
    SXRUtils.cpp
    engmenu/SXREngMenuAdapter.cpp
    app/SXRApplicationManagerClient.cpp
    vsm/SXRVehicleSignalsServiceClient.cpp
    vsm/SXRVehicleSignalManager.cpp
    messages/SXRMessageEcuToVb.cpp
    messages/SXRMessageVbToEcu.cpp
    messages/SXRCommonMessage.cpp
    injection/SXRVsmInjection.cpp
    injection/SXRDaivbInjection.cpp
    injection/SXREcuPublishInjection.cpp
    injection/SXRDaivbPublishInjection.cpp
    injection/SXRCommonInj.cpp
	${DLT_WRAPPER_SOURCES}

    ${COMMON_DIR_ABS_PATH}/dlt/LogMsg.cpp
	${COMMON_DIR_ABS_PATH}/dlt/Logger.cpp
	${COMMON_DIR_ABS_PATH}/dlt/Context.cpp
	${COMMON_DIR_ABS_PATH}/dlt/Buffer.cpp
	${COMMON_DIR_ABS_PATH}/dlt/injector/DltInjector.cpp
)

as_find_commonapi()
pkg_search_module(GATEWAY_CLIENT REQUIRED "gateway-client")
pkg_search_module(AM_CLIENT REQUIRED "am-client")
pkg_check_modules(VS_CLIENT REQUIRED vs-client)
message(STATUS "VS_CLIENT version: ${VS_CLIENT_VERSION}")
message(STATUS "VS_CLIENT lib dirs: ${VS_CLIENT_LIBRARIES}")
message(STATUS "VS_CLIENT include dirs: ${VS_CLIENT_INCLUDE_DIRS}")

link_directories(
    ${GATEWAY_CLIENT_LIBRARY_DIRS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${COMMONAPI_INCLUDE_DIRS}
    ${GATEWAY_CLIENT_INCLUDE_DIRS}
    ${GATEWAY_VSOMEIPPROXY_INCLUDE_DIRS}
    ${AM_CLIENT_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src
    #${CMAKE_SOURCE_DIR}/ipc
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/app
    ${CMAKE_SOURCE_DIR}/src/backend
    ${CMAKE_SOURCE_DIR}/src/bridge
	${CMAKE_SOURCE_DIR}/src/queues
	${CMAKE_SOURCE_DIR}/src/messages
    ${CMAKE_SOURCE_DIR}/src/injection
     ${CMAKE_SOURCE_DIR}/src/client
    # define the default path of RPC Bridge includes

    ${RPC_DIR}
    #${CMAKE_SOURCE_DIR}/src/power
    ${VS_CLIENT_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${GATEWAY_CLIENT_LIBRARIES}
    ${GATEWAY_VSOMEIPPROXY_LIBRARIES}
    ${VS_CLIENT_LIBRARIES}
    
    asyncipc
    am-client
    dlt
    pthread
    
	apsm-client
	omm-client
	pm-client

    as-commonlib
    engmenu-client
)

install(TARGETS ${PROJECT_NAME} DESTINATION "/usr/bin")

as_setup_compile_options(${PROJECT_NAME})
