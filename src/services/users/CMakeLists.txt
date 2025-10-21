cmake_minimum_required(VERSION 3.10)

project(UsersService VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add compile flags
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)

# Find jsoncpp
find_package(jsoncpp REQUIRED)

# Include directories
include_directories(${LIBPQXX_INCLUDE_DIRS})

# Add executable
add_executable(users_service 
    main.cpp
    ../../shared/common/database/postgresql_database.cpp
    ../../shared/repository/user_repository.cpp
    ../../shared/types/user.cpp
    ../../shared/common/config/config.cpp
    ../../shared/validation/schema_validator.cpp
)

# Link libraries
target_link_libraries(users_service 
    ${LIBPQXX_LIBRARIES}
    jsoncpp_lib
)

# Link with static libgcc to reduce dependencies
set_target_properties(users_service PROPERTIES
    LINK_FLAGS "-static-libgcc"
)

# Install target
install(TARGETS users_service DESTINATION bin)
