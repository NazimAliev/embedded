#===================================================================#
# This program is the confidential and proprietary product of Valeo #
# Comfort and Driving Assistance Systems. Any unauthorised use,     #
# reproduction or transfer of this program is strictly prohibited.  #
#                                                                   #
# Copyright (C) 2018 Valeo Comfort and Driving Assistance Systems.  #
# (Subject to limited distribution and restricted disclosure only.) #
#===================================================================#

# Version Requirement
cmake_minimum_required(VERSION 2.8)

message(STATUS ${PROJECT_NAME})

if(NOT ${PROJECT_NAME} MATCHES "send-x-receive")
    message(FATAL_ERROR "\nNot cmake from here, use utils/CMakeLists.txt" )
endif()

# Target Name
set(TARGET_NAME "create-queue")

add_executable(${TARGET_NAME}
    main.cpp
    ${DLT_WRAPPER_SOURCES}
)

#find common-api header
find_package(PkgConfig REQUIRED)
pkg_search_module(COMMONAPI REQUIRED "CommonAPI")

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/common
    ${COMMONAPI_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/ipc
)

target_link_libraries(${TARGET_NAME}
    asyncipc
    dlt
    rt
)

install(TARGETS ${TARGET_NAME} DESTINATION "/usr/bin")
