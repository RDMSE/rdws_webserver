#!/bin/bash

# Setup script for Linux Mint development environment
# This script installs all dependencies needed to develop the C++ REST server locally

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
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

echo -e "${BLUE}🐧 Linux Mint C++ REST Server Development Setup${NC}"
echo -e "${BLUE}===============================================${NC}"
echo ""

# Check if running on Linux Mint
if ! grep -qi "mint" /etc/os-release && ! grep -qi "ubuntu" /etc/os-release; then
    print_warning "This script is designed for Linux Mint/Ubuntu. You may need to adapt package names."
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Update package list
print_status "Updating package list..."
sudo apt update

# Install build essentials
print_status "Installing build essentials..."
sudo apt install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    pkg-config \
    meson \
    ninja-build \
    software-properties-common

# Install development libraries
print_status "Installing development libraries..."
sudo apt install -y \
    libcurl4-openssl-dev \
    rapidjson-dev \
    libssl-dev \
    zlib1g-dev

# Install Google Test
print_status "Installing Google Test..."
sudo apt install -y libgtest-dev libgmock-dev

# Try to install Pistache from packages first
print_status "Attempting to install Pistache from packages..."
if sudo apt install -y libpistache-dev; then
    print_success "Pistache installed from packages"
    PISTACHE_INSTALLED=true
else
    print_warning "Pistache not available in packages, will compile from source"
    PISTACHE_INSTALLED=false
fi

# If Pistache not available in packages, compile from source
if [ "$PISTACHE_INSTALLED" = false ]; then
    print_status "Installing Pistache dependencies..."
    sudo apt install -y \
        libfmt-dev \
        libspdlog-dev \
        libtbb-dev

    print_status "Compiling Pistache from source..."
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"

    git clone --depth=1 https://github.com/pistacheio/pistache.git
    cd pistache

    meson setup build \
        --buildtype=release \
        --prefix=/usr/local \
        -Db_lto=true \
        -DPISTACHE_USE_SSL=ON

    meson compile -C build
    sudo meson install -C build

    # Update library cache
    sudo ldconfig

    cd /
    rm -rf "$TEMP_DIR"

    print_success "Pistache compiled and installed"
fi

# Verify installations
print_status "Verifying installations..."

# Check CMake
if command -v cmake >/dev/null 2>&1; then
    CMAKE_VERSION=$(cmake --version | head -n1)
    print_success "CMake: $CMAKE_VERSION"
else
    print_error "CMake not found"
fi

# Check GCC
if command -v g++ >/dev/null 2>&1; then
    GCC_VERSION=$(g++ --version | head -n1)
    print_success "G++: $GCC_VERSION"
else
    print_error "G++ not found"
fi

# Check pkg-config for Pistache
if pkg-config --exists libpistache; then
    PISTACHE_VERSION=$(pkg-config --modversion libpistache)
    print_success "Pistache: $PISTACHE_VERSION"
else
    print_warning "Pistache not found via pkg-config, but may still work"
fi

# Check Google Test
if [ -d "/usr/include/gtest" ] || [ -d "/usr/local/include/gtest" ]; then
    print_success "Google Test: Available"
else
    print_error "Google Test headers not found"
fi

# Create a test build to verify everything works
print_status "Testing build environment..."
if [ -f "../CMakeLists.txt" ]; then
    print_status "Running test build..."
    mkdir -p /tmp/cpp-rest-test-build
    cd /tmp/cpp-rest-test-build

    if PKG_CONFIG_PATH="/usr/local/lib64/pkgconfig:/usr/local/lib/pkgconfig" cmake "$(dirname "$0")/.." && make -j$(nproc); then
        print_success "Test build successful!"
        cd /
        rm -rf /tmp/cpp-rest-test-build
    else
        print_error "Test build failed. Check error messages above."
        cd /
        rm -rf /tmp/cpp-rest-test-build
        exit 1
    fi
else
    print_warning "CMakeLists.txt not found. Skipping test build."
fi

echo ""
print_success "Linux Mint development environment setup complete!"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "1. 📂 Open project in VS Code"
echo "2. Use VS Code tasks (Ctrl+Shift+P → 'Tasks: Run Task')"
echo "3. 🐳 For Docker development: ./scripts/local_setup.sh --docker"
echo "4. For remote deploy: Use VS Code task 'Deploy to Server'"
echo ""
echo -e "${YELLOW}Build locally:${NC}"
echo "  mkdir build && cd build"
echo "  PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig:/usr/local/lib/pkgconfig cmake .."
echo "  make -j$(nproc)"
echo ""
