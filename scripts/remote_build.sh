#!/bin/bash

# Remote build script for C++ REST Server
# This script runs on the remote server to build the project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[BUILD]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo -e "${BLUE}🔨 Building C++ REST Server on Remote Server${NC}"
echo -e "${BLUE}============================================${NC}"

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

print_status "Project root: $PROJECT_ROOT"
print_status "Cleaning previous build..."

# Clean previous build
rm -rf build/

print_status "Creating build directory..."
mkdir -p build
cd build

print_status "Running CMake configuration..."
PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig cmake ..

if [ $? -ne 0 ]; then
    print_error "CMake configuration failed"
    exit 1
fi

print_status "Building main server..."
make rest_server

if [ $? -ne 0 ]; then
    print_error "Server build failed"
    exit 1
fi

print_status "Building unit tests..."
make unit_tests

if [ $? -ne 0 ]; then
    print_error "Unit tests build failed"
    exit 1
fi

print_status "Building integration tests..."
make integration_tests

if [ $? -ne 0 ]; then
    print_error "Integration tests build failed"
    exit 1
fi

print_success "Build completed successfully!"
echo ""
echo -e "${YELLOW}Built artifacts:${NC}"
echo "- Server executable: build/rest_server"
echo "- Unit tests: build/tests/unit_tests"
echo "- Integration tests: build/tests/integration_tests"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "- Run tests: ./scripts/remote_test.sh"
echo "- Start server: ./scripts/remote_run.sh"
