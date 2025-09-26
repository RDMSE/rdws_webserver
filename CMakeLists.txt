cmake_minimum_required(VERSION 3.10)

# Set the project name
project(CppRestServer VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable debug symbols for development
set(CMAKE_BUILD_TYPE Debug)

# Add compile flags for better debugging and warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

# Enable testing
enable_testing()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find Pistache library
pkg_check_modules(PISTACHE REQUIRED libpistache)

# Find Google Test
find_package(GTest REQUIRED)
# GMock is usually included with GTest in modern installations

# Create library from server code (for easier testing)
add_library(hello_server_lib
    src/hello_server.cpp
)

# Link libraries to the library
target_link_libraries(hello_server_lib 
    ${PISTACHE_LIBRARIES}
    pthread
)

# Add include directories for Pistache to the library
target_include_directories(hello_server_lib PRIVATE 
    ${PISTACHE_INCLUDE_DIRS}
)

# Add compile flags for Pistache to the library
target_compile_options(hello_server_lib PRIVATE 
    ${PISTACHE_CFLAGS_OTHER}
)

# Add executable
add_executable(rest_server
    src/main.cpp
)

# Link the server library to the executable
target_link_libraries(rest_server 
    hello_server_lib
)

# Add tests subdirectory
add_subdirectory(tests)

# Install target (optional)
install(TARGETS rest_server DESTINATION bin)