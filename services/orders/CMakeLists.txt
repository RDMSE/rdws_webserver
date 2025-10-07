cmake_minimum_required(VERSION 3.10)

project(OrdersService VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add compile flags
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra")

# Add executable
# Create standalone orders service executable
add_executable(orders_service main.cpp)

# Link with static libgcc to reduce dependencies
set_target_properties(orders_service PROPERTIES
    LINK_FLAGS "-static-libgcc"
)

# Install target
install(TARGETS orders_service DESTINATION bin)
