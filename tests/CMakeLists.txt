# CMakeLists.txt for tests
cmake_minimum_required(VERSION 3.10)

# Include Google Test
include(GoogleTest)

# Include Valijson for tests that need validation
include_directories(../src/third_party/valijson/include)

# Criar biblioteca de helpers comuns
add_library(test_helpers
    common/test_helpers.cpp
)

target_include_directories(test_helpers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

# Teste básico (placeholder)
add_executable(microservice_tests
    test_main.cpp
)

target_link_libraries(microservice_tests
    GTest::gtest
    GTest::gtest_main
    pthread
)

# Testes do microserviço de usuários
add_executable(users_service_tests
    users/test_users_service.cpp
    test_main.cpp
)

target_link_libraries(users_service_tests
    test_helpers
    GTest::gtest
    GTest::gtest_main
    pthread
)

# Testes do microserviço de pedidos
add_executable(orders_service_tests
    orders/test_orders_service.cpp
    test_main.cpp
)

target_link_libraries(orders_service_tests
    test_helpers
    GTest::gtest
    GTest::gtest_main
    pthread
)

# Registrar todos os testes com CTest
gtest_discover_tests(microservice_tests)
gtest_discover_tests(users_service_tests)
gtest_discover_tests(orders_service_tests)