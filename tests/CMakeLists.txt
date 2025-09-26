# CMakeLists.txt for tests
cmake_minimum_required(VERSION 3.10)

# Include Google Test
include(GoogleTest)

# Unit tests executable
add_executable(unit_tests
    test_hello_server.cpp
    test_main.cpp
)

# Integration tests executable
add_executable(integration_tests
    test_integration.cpp
    test_main.cpp
)

# Link unit test executable against gtest & gtest_main libraries
target_link_libraries(unit_tests
    hello_server_lib
    ${PISTACHE_LIBRARIES}
    GTest::gtest
    GTest::gtest_main
    pthread
)

# Link integration test executable with additional CURL dependency
target_link_libraries(integration_tests
    hello_server_lib
    ${PISTACHE_LIBRARIES}
    GTest::gtest
    GTest::gtest_main
    curl
    pthread
)

# Add include directories for unit tests
target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${PISTACHE_INCLUDE_DIRS}
)

# Add include directories for integration tests
target_include_directories(integration_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${PISTACHE_INCLUDE_DIRS}
)

# Add compile flags for unit tests
target_compile_options(unit_tests PRIVATE 
    ${PISTACHE_CFLAGS_OTHER}
)

# Add compile flags for integration tests
target_compile_options(integration_tests PRIVATE 
    ${PISTACHE_CFLAGS_OTHER}
)

# Register tests with CMake
gtest_discover_tests(unit_tests)
gtest_discover_tests(integration_tests)