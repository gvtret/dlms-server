if(DLMS_USE_SYSTEM_GTEST)
  find_package(GTest REQUIRED)
elseif(NOT TARGET GTest::gtest_main)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(dlms_server_tests
  server/test_server_context.cpp
  server/test_server_types.cpp)

target_link_libraries(dlms_server_tests
  PRIVATE
    dlms_server
    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(dlms_server_tests)
