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

project("send-x-receive-unittest")
message(STATUS "%% : PROJECT_NAME ${PROJECT_NAME}")

# must build with cmake flags==> -DBUILD_TESTS=ON which will set BUILD_UNIT_TESTS flag
artemis_init_unit_tests()

message(STATUS "%% : CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}")
message(STATUS "%% : CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "%% : CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "%% : CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}")
message(STATUS "%% : COMMON_DIR ${COMMON_DIR}")
# all the test source files are added here
set(TESTS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SXRWithQueueTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SXRUtilsTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SXRWithoutQueueTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SXRInitFunctionTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SXRVehicleSignalsServiceClientTests.cpp
)

# all the real application sources files are added here
set(SXR_SOURCES
    ${CMAKE_SOURCE_DIR}/src/SendXReceive.cpp
    ${CMAKE_SOURCE_DIR}/src/SXRUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/engmenu/SXREngMenuAdapter.cpp
    ${CMAKE_SOURCE_DIR}/src/messages/SXRMessageEcuToVb.cpp
    ${CMAKE_SOURCE_DIR}/src/messages/SXRMessageVbToEcu.cpp
    ${CMAKE_SOURCE_DIR}/src/messages/SXRCommonMessage.cpp
    ${CMAKE_SOURCE_DIR}/src/injection/SXRDaivbInjection.cpp
    ${CMAKE_SOURCE_DIR}/src/injection/SXRDaivbPublishInjection.cpp
    ${CMAKE_SOURCE_DIR}/src/injection/SXRVsmInjection.cpp
    ${CMAKE_SOURCE_DIR}/src/injection/SXREcuPublishInjection.cpp
    ${CMAKE_SOURCE_DIR}/src/injection/SXRCommonInj.cpp
    ${CMAKE_SOURCE_DIR}/src/vsm/SXRVehicleSignalManager.cpp
    ${CMAKE_SOURCE_DIR}/src/vsm/SXRVehicleSignalsServiceClient.cpp
)

# do not modify this, modify the above 2 variables instead (Tests_SOURCES, MBeCall_SOURCES)
add_executable(${PROJECT_NAME}
    ${TESTS_SOURCES}
    ${SXR_SOURCES}
)

#############################################################################################################

# all the test include directories are updated here
set(TESTS_INCLUDES "")

# Dlt helpers
target_sources(${PROJECT_NAME} PRIVATE
    ${COMMON_DIR_ABS_PATH}/dlt/LogMsg.cpp
    ${COMMON_DIR_ABS_PATH}/dlt/Logger.cpp
    ${COMMON_DIR_ABS_PATH}/dlt/Context.cpp
    ${COMMON_DIR_ABS_PATH}/dlt/Buffer.cpp
    ${COMMON_DIR_ABS_PATH}/dlt/injector/DltInjector.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../../daimler-artemis-vcore/rpc-bridge/Src/RPC_NAD/generated_IPC
    ${COMMONAPI_INCLUDE_DIRS}
    ${GATEWAY_CLIENT_INCLUDE_DIRS}
    ${GATEWAY_VSOMEIPPROXY_INCLUDE_DIRS}

    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/app
    ${CMAKE_SOURCE_DIR}/src/backend
    ${CMAKE_SOURCE_DIR}/src/bridge
	${CMAKE_SOURCE_DIR}/src/queues
	${CMAKE_SOURCE_DIR}/src/messages
    ${CMAKE_SOURCE_DIR}/src/injection
    ${CMAKE_SOURCE_DIR}/src/client
)

# do not modify this, modify the above 2 variables instead (Tests_INCLUDES, MBeCall_INCLUDES)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${TESTS_INCLUDES}        # this must always be first to enable cmake to find the stubs first if the exist
    ${SXR_INCLUDES}     # this must be second to enable cmake to find the real header if we didn't make a stub
)

link_directories(
    #${Boost_LIBRARY_DIR}
    #${WRAPPERS_LIBRARY_DIRS}
)

set(LIBRARY_LIST
    ${GATEWAY_CLIENT_LIBRARIES}
    ${GATEWAY_VSOMEIPPROXY_LIBRARIES}
    ${VS_CLIENT_LIBRARIES}
    as-commonlib
    am-client
    asyncipc
    dlt
    pthread
    gtest
    gmock
    apsm-client
    omm-client
    pm-client
    engmenu-client
)

if(COVERAGE_ENABLE)
	list(APPEND LIBRARY_LIST gcov)

	set(TEST_EXECUTABLE ${CMAKE_BINARY_DIR}/tests/unitTests/${PROJECT_NAME})
	set(TEST_EXECUTABLE_ARGS ) #required empty variable to pass

	if(COVERAGE_USE_QEMU)
		if(NOT COVERAGE_SDK_SYSROOT_PATH)
			message(FATAL_ERROR "COVERAGE_SDK_SYSROOT_PATH required for qemu-arm execution")
		endif()

		set(TEST_EXECUTABLE qemu-arm)
		set(TEST_EXECUTABLE_ARGS -L ${COVERAGE_SDK_SYSROOT_PATH} ${CMAKE_BINARY_DIR}/tests/unitTests/${PROJECT_NAME})
	endif()

	if(COVERAGE_TEST_ARGS)
		list(APPEND TEST_EXECUTABLE_ARGS ${COVERAGE_TEST_ARGS})
	endif()

	if(COVERAGE_ENABLE_XML)
		setup_target_for_coverage_gcovr_xml(NAME ${PROJECT_NAME}-coverage-xml
				DEPENDENCIES ${PROJECT_NAME}
				EXECUTABLE ${TEST_EXECUTABLE}
				EXECUTABLE_ARGS ${TEST_EXECUTABLE_ARGS}
				BASE_DIRECTORY ${CMAKE_SOURCE_DIR}
				EXCLUDE ${CMAKE_SOURCE_DIR}/tests/* ${CMAKE_BINARY_DIR}/* /usr/include/*
				OUTPUT_DIRECTORY ${COVERAGE_XML_OUTPUT_DIRECTORY}
				OUTPUT_NAME ${COVERAGE_XML_OUTPUT_NAME}
				PACKAGE_PREFIX src:${TARGET_NAME}
				KEEP_GCOV_FILES ${KEEP_GCOV_FILES})
	endif()

	if(COVERAGE_ENABLE_HTML)
		setup_target_for_coverage_gcovr_html(NAME ${PROJECT_NAME}-coverage-html
				DEPENDENCIES ${PROJECT_NAME}
				EXECUTABLE ${TEST_EXECUTABLE}
				EXECUTABLE_ARGS ${TEST_EXECUTABLE_ARGS}
				EXCLUDE ${CMAKE_SOURCE_DIR}/tests/* ${CMAKE_BINARY_DIR}/* /usr/include/*
				BASE_DIRECTORY ${CMAKE_SOURCE_DIR}
				OUTPUT_DIRECTORY ${COVERAGE_HTML_OUTPUT_DIRECTORY}
				OUTPUT_NAME ${COVERAGE_HTML_OUTPUT_NAME}
				KEEP_GCOV_FILES ${KEEP_GCOV_FILES})
	endif()
endif()

target_link_libraries (${PROJECT_NAME} ${LIBRARY_LIST})

target_compile_options(${PROJECT_NAME} PRIVATE
	-Wall -Wextra -Werror  -Wformat -Wpedantic
)

set_target_properties(${PROJECT_NAME} PROPERTIES 
	CMAKE_CXX_STANDARD_REQUIRED TRUE
    CXX_STANDARD 14 
)