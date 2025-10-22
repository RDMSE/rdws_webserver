cmake_minimum_required(VERSION 3.10)

project(OrdersService VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add compile flags
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra")

# Include directories
include_directories(../../../third_party/valijson/include)
include_directories(/usr/include/rapidjson)

# Add executable
# Create standalone orders service executable (updated with new architecture)
add_executable(orders_service 
    main.cpp
    order_service.cpp
    order_repository.cpp
    ../../shared/types/order.cpp
    ../../shared/common/utils/response_helper.cpp
    ../../shared/common/config/config.cpp
    ../../shared/validation/schema_validator.cpp
)

# Find required libraries
find_package(jsoncpp REQUIRED)

# Link with static libgcc to reduce dependencies
set_target_properties(orders_service PROPERTIES
  LINK_FLAGS "-static-libgcc"
)

# Link libraries
target_link_libraries(orders_service
    jsoncpp_lib
)

# Install target
install(TARGETS orders_service DESTINATION bin)
