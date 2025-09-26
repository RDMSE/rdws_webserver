# C++ REST Server with Pistache

This project is a C++ REST API server using the Pistache framework. It provides a simple HTTP server with basic endpoints.

## Project Structure
- `src/` - Source code files
- `include/` - Header files  
- `CMakeLists.txt` - CMake build configuration
- `README.md` - Project documentation

## Dependencies
- Pistache HTTP library
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
- Keep REST endpoints simple and well-documented
- Handle errors gracefully with proper HTTP status codes