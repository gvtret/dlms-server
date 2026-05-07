cmake_minimum_required(VERSION 3.16)

project(dlms_server
  VERSION 0.1.0
  LANGUAGES CXX)

option(DLMS_BUILD_TESTS "Build GoogleTest tests" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)

add_library(dlms_server
  src/server/dlms_server.cpp
  src/server/server_context.cpp
  src/server/service_dispatcher.cpp
  src/server/server_status.cpp
  src/server/server_types.cpp
  src/server/xdlms_server_adapter.cpp)

if(NOT TARGET dlms_cosem AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../dlms-cosem/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../dlms-cosem" "${CMAKE_CURRENT_BINARY_DIR}/_deps/dlms-cosem")
endif()

if(NOT TARGET dlms_xdlms AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../dlms-xdlms/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../dlms-xdlms" "${CMAKE_CURRENT_BINARY_DIR}/_deps/dlms-xdlms")
endif()

target_compile_features(dlms_server PUBLIC cxx_std_11)

target_include_directories(dlms_server
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(dlms_server
  PUBLIC
    dlms_cosem
    dlms_xdlms)

set(DLMS_SERVER_HAS_DISPATCH_TYPES_API ON CACHE INTERNAL
  "dlms-server exposes server dispatch context and service types")

if(DLMS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
