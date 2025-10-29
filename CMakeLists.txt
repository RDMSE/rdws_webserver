cmake_minimum_required(VERSION 3.10)

# Set the project name
project(CppMicroservices VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# identify the environment
if(DEFINED ENV{RDWS_ENVIRONMENT})
  set(ENVIRONMENT $ENV{RDWS_ENVIRONMENT})
else()
  set(ENVIRONMENT "development")
endif()

message(STATUS "We are on '${ENVIRONMENT}' environment")

# define .env paths
set(SOURCE_ENV_FILE "${CMAKE_SOURCE_DIR}/.env.${ENVIRONMENT}")
set(DEST_ENV_DIR "${CMAKE_BINARY_DIR}/src/services")
set(DEST_ENV_FILE "${DEST_ENV_DIR}/.env")

# verify if file exist
if(EXISTS ${SOURCE_ENV_FILE})
  message(STATUS "Using env file: ${SOURCE_ENV_FILE}")
else()
  message(FATAL_ERROR "Env file not found for environment '${ENVIRONMENT}': ${SOURCE_ENV_FILE}")
endif()

# make sure destination directory exists
file(MAKE_DIRECTORY "${DEST_ENV_DIR}")

# copy env file
add_custom_target(copy_env ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${SOURCE_ENV_FILE} ${DEST_ENV_FILE}
  COMMENT "Copying ${SOURCE_ENV_FILE} to ${DEST_ENV_FILE}"
)


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
add_subdirectory(src/third_party/valijson)

# Make dotenv include available globally
include_directories(src/third_party/dotenv-cpp/include/laserpants/dotenv)

# Add Valijson header-only library
include_directories(src/third_party/valijson/include)

# Add rapidjson include directory
include_directories(src/third_party/valijson/thirdparty/rapidjson/include)

# Add services subdirectory (microservices executables)
add_subdirectory(src/services)

# Add tests subdirectory
add_subdirectory(tests)
