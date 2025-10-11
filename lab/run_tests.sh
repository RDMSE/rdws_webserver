#!/usr/bin/env bash

# Lab Testing Script
# Compiles and runs experimental tests

set -e

LAB_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$LAB_DIR/.."

echo "Lab Testing Script"
echo "===================="

# Function to compile and run a test
run_test() {
    local test_name="$1"
    local compile_cmd="$2"
    local description="$3"

    echo
    echo "$description"
    echo "Compiling: $test_name"

    cd "$LAB_DIR"

    # Compile
    if eval "$compile_cmd"; then
        echo "Compilation successful"

        # Run
        echo "Running: ./$test_name"
        if "./$test_name"; then
            echo "Test passed"
        else
            echo "Test failed"
            return 1
        fi
    else
        echo "Compilation failed"
        return 1
    fi
}

# Test dotenv functionality
run_test "test_dotenv" \
    "g++ -std=c++17 -I ../src/third_party/dotenv-cpp/include/laserpants/dotenv -I .. test_dotenv.cpp -o test_dotenv" \
    "Testing dotenv-cpp library integration"

# Test database configuration
run_test "test_config" \
    "g++ -std=c++17 -I .. test_config.cpp -o test_config" \
    "Testing database configuration system"

echo
echo "All lab tests completed successfully!"
echo
echo "To clean up compiled files:"
echo "  cd lab && rm -f test_dotenv test_config"
