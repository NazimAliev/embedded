#===================================================================#
# This program is the confidential and proprietary product of Valeo #
# Comfort and Driving Assistance Systems. Any unauthorised use,     #
# reproduction or transfer of this program is strictly prohibited.  #
#                                                                   #
# Copyright (C) 2021 Valeo Comfort and Driving Assistance Systems.  #
# (Subject to limited distribution and restricted disclosure only.) #
#===================================================================#

# Version Requirement
cmake_minimum_required(VERSION 2.8)

if(NOT ${PROJECT_NAME} MATCHES "send-x-receive")
    message(FATAL_ERROR "\nNot cmake from here, use root CMakeLists.txt" )
endif()

# Target Name
set(TARGET_NAME "console")
set(LINK1 "/usr/bin/sxr_ecu_publish")
set(LINK2 "/usr/bin/sxr_daivb_publish")
set(LINK3 "/usr/bin/sxr_ecu_listen")
set(LINK4 "/usr/bin/sxr_daivb_listen")

add_executable(${TARGET_NAME}
    main.cpp
    parsers.cpp
)


#find common-api and dlt only for headers
find_package(PkgConfig REQUIRED)
pkg_search_module(DLT REQUIRED "automotive-dlt")

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/common
    ${DLT_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME}
    dlt
)

install(TARGETS ${TARGET_NAME} DESTINATION "/usr/bin")

install(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    /usr/bin/console \
    ${LINK1}   \
    )"
)

install(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    /usr/bin/console \
    ${LINK2}   \
    )"
)
install(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    /usr/bin/console \
    ${LINK3}   \
    )"
)
install(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    /usr/bin/console \
    ${LINK4}   \
    )"
)
