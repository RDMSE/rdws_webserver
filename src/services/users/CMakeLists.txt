cmake_minimum_required(VERSION 3.10)

project(UsersService VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add compile flags
#set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)

# Find jsoncpp via pkg-config
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
pkg_check_modules(LIBPQ REQUIRED libpq)

# Include directories
include_directories(${LIBPQXX_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(../../../third_party/valijson/include)
include_directories(/usr/include/rapidjson)
include_directories(../../shared) # Add shared directory to include path
# Optionally include libpq headers (not usually needed)
# include_directories(${LIBPQ_INCLUDE_DIRS})

# Add executable
add_executable(users_service
  main.cpp
  user_service.cpp
  ../../shared/repository/user_repository.cpp
  ../../shared/types/user.cpp
  ../../shared/types/lambda_event.cpp
  ../../shared/types/lambda_context.cpp
  ../../shared/common/utils/response_helper.cpp
  ../../shared/common/config/config.cpp
  ../../shared/validation/schema_validator.cpp
  ../../shared/common/database/postgresql_database.cpp
  ../../shared/common/utils/lambda_params_helper.cpp
)

# Link libraries
target_link_libraries(users_service
  ${LIBPQXX_LIBRARIES}
  ${LIBPQ_LIBRARIES}
  ${JSONCPP_LIBRARIES}
)

# Link with static libgcc to reduce dependencies
set_target_properties(users_service PROPERTIES
  LINK_FLAGS "-static-libgcc"
)

# Install target
install(TARGETS users_service DESTINATION bin)

# Link with static libgcc to reduce dependencies
set_target_properties(users_service PROPERTIES
  LINK_FLAGS "-static-libgcc"
)

# Install target
install(TARGETS users_service DESTINATION bin)
