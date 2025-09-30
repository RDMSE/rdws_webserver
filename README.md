# C++ REST Server with Serverless Architecture

A hybrid REST server in C++ using Pistache framework with serverless functions support, built for Fedora Server.

> **Local Development + Remote Deploy**: See [DEVELOPMENT.md](DEVELOPMENT.md) for complete guide on how to develop locally (Linux Mint) and deploy to server.

## 🏗️ Architecture

### **Hybrid Serverless Model**
- **Main Server**: C++ Pistache server on port 9080 (routing/proxy layer)
- **Serverless Functions**: Node.js containers on port 8082+ (business logic)
- **Auto-Deployment**: GitHub Actions with both C++ server and function deployment

### **Request Flow**
```
Client Request → C++ Server (9080) → Proxy → Serverless Function (8082) → Response
                     ↓ (if function unavailable)
                 Fallback Response
```

## Features

- **Intelligent Proxy**: `/hello` endpoint proxies to serverless function with fallback
- **Serverless Functions**: Node.js functions deployed as Docker containers
- **Auto-Deployment**: CI/CD pipeline deploys both server and functions
- **Fallback Support**: C++ server provides fallback if serverless function unavailable
- **Real-time Health Checks**: Tests both main server and serverless functions
- Basic HTTP REST server on http://localhost:9080
- Graceful shutdown with Ctrl+C

## 🚀 Quick Start

### Configure Environment Variables

1. **Copy environment template:**
```bash
cp .env.example .env
```

2. **Edit configuration:**
```bash
# Edit .env file with your settings
SERVERLESS_FUNCTION_URL=http://localhost:8082/
```

### Test the Serverless Architecture

```bash
# Test main server (fallback)
curl http://fedora-server.local:9080/

# Test serverless proxy endpoint
curl http://fedora-server.local:9080/hello

# Test serverless function directly
curl http://fedora-server.local:8082/
```

The `/hello` endpoint now returns JSON from the serverless function:
```json
{
  "timestamp": "2025-09-30T07:20:53.210Z",
  "originalEndpoint": "/hello",
  "serverless": true
}
```

## ⚙️ Configuration

The server supports configuration via environment variables and `.env` file:

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `SERVERLESS_FUNCTION_URL` | `http://localhost:8082/` | URL of the serverless function |

### Configuration Precedence

1. **System environment variables** (highest priority)
2. **`.env` file** (loaded automatically)
3. **Default values** (fallback)

### Example .env file:
```bash
# Serverless Configuration
SERVERLESS_FUNCTION_URL=http://localhost:8082/

# Future configurations
# SERVER_PORT=9080
# SERVER_THREADS=2
```

## Prerequisites

### Fedora Server

Install the required dependencies:

```bash
# Update system
sudo dnf update -y

# Install essential development tools
sudo dnf groupinstall -y "Development Tools" "Development Libraries"

# Install CMake
sudo dnf install -y cmake

# Install C++ compiler and tools
sudo dnf install -y gcc-c++ make

# Install Pistache dependencies
sudo dnf install -y pistache-devel

# If pistache-devel is not available, install dependencies to compile from source
sudo dnf install -y curl-devel rapidjson-devel
```

### If Pistache is not available in repositories

If Pistache is not available via DNF, you can compile it from source:

```bash
# Clone and compile Pistache
git clone https://github.com/pistacheio/pistache.git
cd pistache
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
sudo ldconfig
cd ../..
```

## Project Structure

```
server/
├── src/
│   ├── main.cpp           # Main file with server initialization
│   └── hello_server.cpp   # HelloServer class with serverless proxy
├── include/
│   └── hello_server.h     # HelloServer class header with proxy methods
├── functions/             # Serverless functions directory
│   ├── hello-simple/      # Node.js serverless function
│   │   ├── handler.js     # Function implementation
│   │   └── package.json   # Node.js dependencies
│   ├── stack.yaml         # OpenFaaS function configuration
│   └── README.md          # Serverless functions documentation
├── .github/workflows/     # GitHub Actions CI/CD
│   ├── ci.yml            # Continuous Integration
│   └── deploy.yml        # Deployment (C++ + Serverless)
├── CMakeLists.txt        # CMake configuration
└── README.md             # This documentation
```

## Building

1. Create a build directory:
```bash
mkdir build && cd build
```

2. Configure the project with CMake:
```bash
cmake ..
```

3. Compile:
```bash
make
```

## Running

### Method 1: Direct Execution
After compilation, run the server:

```bash
# From the build directory
./rest_server
```

### Method 2: As System Service (Recommended)
Install and run as a systemd service:

```bash
# Install service
sudo ./scripts/service_manager.sh install

# Start service
sudo ./scripts/service_manager.sh start

# Check status
sudo ./scripts/service_manager.sh status

# View logs
sudo ./scripts/service_manager.sh logs
```

The server will start and be available at:
- http://localhost:9080/hello
- http://localhost:9080/

### Service Management Commands:
```bash
sudo ./scripts/service_manager.sh start     # Start service
sudo ./scripts/service_manager.sh stop      # Stop service
sudo ./scripts/service_manager.sh restart   # Restart service
sudo ./scripts/service_manager.sh status    # Check status
sudo ./scripts/service_manager.sh logs      # View logs
sudo ./scripts/service_manager.sh uninstall # Remove service
```

## Testing

You can test the endpoints using curl:

```bash
# Test the /hello endpoint
curl http://localhost:9080/hello

# Test the root endpoint
curl http://localhost:9080/

# Both should return: "Hello World from C++ REST Server!"
```

## Configuration

The server is configured for:
- **Port**: 9080 (configurable in main.cpp)
- **Threads**: 2 (configurable in main.cpp)
- **IP**: 0.0.0.0 (accepts connections from any IP)

To change these settings, edit the `src/main.cpp` file.

## Testing

The project includes two types of tests:

### **Unit Tests** (Google Test)
Test class logic without external dependencies.

### **Integration Tests**
Test real HTTP endpoints with complete requests.

### Running Tests

```bash
# Compile and run only unit tests
cd build
PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig cmake ..
make unit_tests
./tests/unit_tests

# Compile and run integration tests
make integration_tests
./tests/integration_tests

# Run all tests
./tests/unit_tests && ./tests/integration_tests

# Use VS Code tasks (Recommended)
# Ctrl+Shift+P -> Tasks: Run Task -> "Run Tests" (unit tests)
# Ctrl+Shift+P -> Tasks: Run Task -> "Run Integration Tests"
# Ctrl+Shift+P -> Tasks: Run Task -> "Run All Tests"
```

### Unit Tests (7 tests)

- **HelloServerTest**: Tests basic server initialization
- **HelloServerConstructorTest**: Tests different address configurations
- **HelloServerFunctionalTest**: Basic functional tests
- **HelloServerMultipleInstancesTest**: Tests multiple instances
- **HelloServerConfigTest**: Tests different thread configurations
- **HelloServerEdgeCasesTest**: Tests edge cases

### Integration Tests (4 tests)

- **TestRootEndpoint**: Tests GET / returns "Hello World"
- **TestHelloEndpoint**: Tests GET /hello returns "Hello World"
- **TestNonExistentEndpoint**: Tests non-existent endpoint returns 404
- **TestConcurrentRequests**: Tests 5 concurrent requests

### Running Tests with Detailed Output

```bash
cd build
./tests/unit_tests --gtest_verbose
./tests/integration_tests --gtest_verbose
```

## Development

### Adding New Endpoints

To add new endpoints, edit the methods in the `HelloServer` class:

1. Add the declaration in the header `include/hello_server.h`
2. Implement the method in `src/hello_server.cpp`
3. Register the route in the `setupRoutes()` method

### New Endpoint Example

```cpp
// In header file
void statusHandler(const Pistache::Rest::Request& request,
                  Pistache::Http::ResponseWriter response);

// In implementation
void HelloServer::statusHandler(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
    response.send(Pistache::Http::Code::Ok, "Server is running!");
}

// In setupRoutes()
Routes::Get(router, "/status", Routes::bind(&HelloServer::statusHandler, this));
```

## Troubleshooting

### Library not found error
If you encounter errors related to missing libraries:
```bash
sudo ldconfig
```

### Port in use
If port 9080 is already in use, change it in `main.cpp`:
```cpp
Port port(8080); // or another available port
```

### Firewall
If you can't access from other machines, configure the firewall:
```bash
sudo firewall-cmd --permanent --add-port=9080/tcp
sudo firewall-cmd --reload
```
