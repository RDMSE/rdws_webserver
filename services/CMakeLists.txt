cmake_minimum_required(VERSION 3.10)

# Services CMakeLists.txt
# Each service is a standalone executable

# Add users service
add_subdirectory(users)

# Add orders service
add_subdirectory(orders)

# Future services can be added here:
# add_subdirectory(products)
# add_subdirectory(payments)
# add_subdirectory(notifications)
