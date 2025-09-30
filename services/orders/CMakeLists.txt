cmake_minimum_required(VERSION 3.10)

project(OrdersService VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add compile flags
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra")

# Add executable
add_executable(orders_service main.cpp)

# Install target
install(TARGETS orders_service DESTINATION bin)
