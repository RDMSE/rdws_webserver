# CMakeLists.txt for tests
cmake_minimum_required(VERSION 3.10)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Include Google Test
include(GoogleTest)

# Include Valijson for tests that need validation
include_directories(../src/third_party/valijson/include)

# Basic test (placeholder)
add_executable(microservice_tests
  test_main.cpp
)

target_link_libraries(microservice_tests
  GTest::gtest
  GTest::gtest_main
  pthread
)



# Testes unitários do UserService (com MockDatabase)
add_executable(users_service_unit_tests
  users/test_user_service_unit.cpp
  test_main.cpp
  mocks/mock_database.cpp
  ../src/services/users/user_service.cpp
  ../src/shared/repository/user_repository.cpp
  ../src/shared/types/user.cpp
  ../src/shared/types/lambda_event.cpp
  ../src/shared/types/lambda_context.cpp
  ../src/shared/common/config/config.cpp
  ../src/shared/validation/schema_validator.cpp
  ../src/shared/common/utils/response_helper.cpp
)

target_include_directories(users_service_unit_tests PRIVATE
  ../src/third_party/valijson/include
  /usr/include/rapidjson
  ${JSONCPP_INCLUDE_DIRS}
)

target_link_libraries(users_service_unit_tests
  GTest::gmock
  GTest::gtest
  GTest::gtest_main
  ${JSONCPP_LIBRARIES}
  pthread
)

# Set working directory for tests to find schemas
set_target_properties(users_service_unit_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)



# Order Service Unit Tests (with MockDatabase)
add_executable(orders_service_unit_tests
  orders/test_order_service_unit.cpp
  test_main.cpp
  mocks/mock_database.cpp
  ../src/services/orders/order_service.cpp
  ../src/services/orders/order_repository.cpp
  ../src/shared/types/order.cpp
  ../src/shared/types/lambda_event.cpp
  ../src/shared/types/lambda_context.cpp
  ../src/shared/common/config/config.cpp
  ../src/shared/validation/schema_validator.cpp
  ../src/shared/common/utils/response_helper.cpp
)

target_include_directories(orders_service_unit_tests PRIVATE
  ../src/third_party/valijson/include
  /usr/include/rapidjson
  ${JSONCPP_INCLUDE_DIRS}
)

target_link_libraries(orders_service_unit_tests
  GTest::gmock
  GTest::gtest
  GTest::gtest_main
  ${JSONCPP_LIBRARIES}
  pthread
)

# Set working directory for tests to find schemas
set_target_properties(orders_service_unit_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)



# Registrar testes unitários com CTest
gtest_discover_tests(microservice_tests)
gtest_discover_tests(users_service_unit_tests
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
gtest_discover_tests(orders_service_unit_tests
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
