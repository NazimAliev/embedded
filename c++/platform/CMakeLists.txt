#===================================================================#
# This program is the confidential and proprietary product of Valeo #
# Comfort and Driving Assistance Systems. Any unauthorised use,     #
# reproduction or transfer of this program is strictly prohibited.  #
#                                                                   #
# Copyright (C) 2021 Valeo Comfort and Driving Assistance Systems.  #
# (Subject to limited distribution and restricted disclosure only.) #
#===================================================================#

cmake_minimum_required(VERSION 3.2)

# Project Name
project(send-x-receive)

# define the default path of common in Artemis
set(COMMON_DIR "../common" CACHE PATH "Common directory location")
message(STATUS "COMMON_DIR_FOR_SXR: ${COMMON_DIR}")

# add common directory path to cmake
if(IS_ABSOLUTE ${COMMON_DIR})
    add_subdirectory(${COMMON_DIR} ${CMAKE_CURRENT_BINARY_DIR}/common)
    message(STATUS "IS_ABSOLUTE: ${COMMON_DIR}")
    message(STATUS "IS ABSOLUTE: ${CMAKE_CURRENT_BINARY_DIR}/common")
else()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${COMMON_DIR} ${CMAKE_CURRENT_BINARY_DIR}/common)
    message(STATUS "NO_ABSOLUTE: ${CMAKE_CURRENT_SOURCE_DIR}/${COMMON_DIR}")
    message(STATUS "NO ABSOLUTE: ${CMAKE_CURRENT_BINARY_DIR}/common")
endif()

# include all the cmake functions in common
include(${COMMON_DIR}/cmake/common.cmake)

# as-commonlib
add_subdirectory(../as-commonlib ${CMAKE_CURRENT_BINARY_DIR}/as-commonlib)

# Set component version
artemis_set_component_version(
    "${SXR_VERSION_MAJOR}"
    "${SXR_VERSION_MINOR}"
    "${SXR_VERSION_PATCH}"
)

as_setup_coverage()

# Create service and unit tests
artemis_configure_component()

add_subdirectory(utils/console)
#add_subdirectory(utils/rpc-bridge-fake)
