cmake_minimum_required(VERSION 3.10)

# Set the project name
project(CppMicroservices VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Add compile flags for better debugging and warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Wextra")

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest REQUIRED)

# Find libpqxx for PostgreSQL support
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Make libpqxx available to all subdirectories
set(PQXX_LIBS ${PQXX_LIBRARIES})
set(PQXX_INCLUDES ${PQXX_INCLUDE_DIRS})

# Add dotenv-cpp third-party library
add_subdirectory(src/third_party/dotenv-cpp)

# Make dotenv include available globally
include_directories(src/third_party/dotenv-cpp/include/laserpants/dotenv)

# Add Valijson header-only library
include_directories(src/third_party/valijson/include)

# Add services subdirectory (microservices executables)
add_subdirectory(src/services)

# Add tests subdirectory
add_subdirectory(tests)
