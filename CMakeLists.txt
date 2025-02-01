cmake_minimum_required(VERSION 3.10)
project(SNTOOL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-pipe -fopenmp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sn-tool/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sn-tool/lib)

set(DEFAULT_INSTALL_BIN ~/sw_home/local/ixToolbox/)
if(NOT INSTALL_TARGET)
    set(INSTALL_TARGET ${DEFAULT_INSTALL_BIN} CACHE STRING "Use the default path")
endif()

include_directories(
    $ENV{SW_HOME}/local/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/internal/include/
)

file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB COMMON_SRCS "${CMAKE_SOURCE_DIR}/internal/src/*.cpp")

set(IXSN_TOOL "ixsn-tool")
set(IXFW_TOOL_SRC "main.cpp")
add_executable(${IXSN_TOOL} ${IXFW_TOOL_SRC} ${SRCS} ${COMMON_SRCS})

install(TARGETS ixsn-tool DESTINATION ${INSTALL_TARGET})
