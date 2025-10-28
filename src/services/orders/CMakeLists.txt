cmake_minimum_required(VERSION 3.10)

project(OrdersService VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add compile flags
#set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Include directories
include_directories(${LIBPQXX_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(../../../third_party/valijson/include)
include_directories(/usr/include/rapidjson)
include_directories(../../shared)  # Add shared directory to include path

# Add executable
# Create standalone orders service executable (updated with new architecture)
add_executable(orders_service
  main.cpp
  order_service.cpp
  ../../shared/repository/order_repository.cpp
  ../../shared/types/order.cpp
  ../../shared/types/lambda_event.cpp
  ../../shared/types/lambda_context.cpp
  ../../shared/common/utils/response_helper.cpp
  ../../shared/common/config/config.cpp
  ../../shared/validation/schema_validator.cpp
  ../../shared/common/database/postgresql_database.cpp
)

# Link with static libgcc to reduce dependencies
set_target_properties(orders_service PROPERTIES
  LINK_FLAGS "-static-libgcc"
)

# Link libraries
target_link_libraries(orders_service
  ${JSONCPP_LIBRARIES}
  ${LIBPQXX_LIBRARIES}
)

# Install target
install(TARGETS orders_service DESTINATION bin)
