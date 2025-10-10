# C++ Microservices with PostgreSQL

This project is a C++ microservices architecture with PostgreSQL database integration. It provides independent HTTP services with JSON APIs and modern configuration management.

## Project Structure
- `src/` - Source code files
- `include/` - Header files  
- `CMakeLists.txt` - CMake build configuration
- `README.md` - Project documentation

## Dependencies
- libpqxx (PostgreSQL C++ library)
- dotenv-cpp (environment configuration)
- CMake (build system)
- C++17 compiler (gcc/clang)

## Build Instructions
```bash
mkdir build && cd build
cmake ..
make
```

## Development Guidelines
- Follow modern C++ practices (C++17+)
- Use CMake for build management
- Keep microservices independent and well-documented
- Handle database connections gracefully with proper error handling
- Use environment-based configuration (.env files)