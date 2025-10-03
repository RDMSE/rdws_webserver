# DEPRECATED: This script was designed for Pistache server - needs update for microservices architecture
#!/bin/bash

# Remote test script for C++ REST Server
# This script runs on the remote server to execute all tests

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

echo -e "${BLUE}🧪 Running Tests on Remote Server${NC}"
echo -e "${BLUE}==================================${NC}"

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

# Check if build directory exists
if [ ! -d "build" ]; then
    print_error "Build directory not found. Please run ./scripts/remote_build.sh first"
    exit 1
fi

cd build

# Check if test executables exist
if [ ! -f "tests/unit_tests" ]; then
    print_error "Unit tests executable not found. Please run ./scripts/remote_build.sh first"
    exit 1
fi

if [ ! -f "tests/integration_tests" ]; then
    print_error "Integration tests executable not found. Please run ./scripts/remote_build.sh first"
    exit 1
fi

# Run unit tests
print_status "Running unit tests..."
echo ""
./tests/unit_tests

if [ $? -eq 0 ]; then
    print_success "Unit tests passed"
else
    print_error "Unit tests failed"
    exit 1
fi

echo ""
print_status "Running integration tests..."
echo ""

# Run integration tests with timeout
timeout 60s ./tests/integration_tests

if [ $? -eq 0 ]; then
    print_success "Integration tests passed"
elif [ $? -eq 124 ]; then
    print_error "Integration tests timed out (60s)"
    exit 1
else
    print_error "Integration tests failed"
    exit 1
fi

# Generate test report if requested
if [ "$1" = "--report" ] || [ "$1" = "-r" ]; then
    print_status "Generating test reports..."

    # Run tests with XML output
    ./tests/unit_tests --gtest_output=xml:unit_test_results.xml
    timeout 60s ./tests/integration_tests --gtest_output=xml:integration_test_results.xml

    print_success "Test reports generated:"
    echo "- Unit tests: build/unit_test_results.xml"
    echo "- Integration tests: build/integration_test_results.xml"
fi

echo ""
print_success "All tests passed successfully!"
echo ""
echo -e "${YELLOW}Test Summary:${NC}"
echo "- Unit tests: PASSED"
echo "- Integration tests: PASSED"
echo ""
echo -e "${YELLOW}Next step:${NC}"
echo "- Start server: ./scripts/remote_run.sh"
