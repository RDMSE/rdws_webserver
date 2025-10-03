# DEPRECATED: This script was designed for Pistache server - needs update for microservices architecture
#!/bin/bash

# Local development setup script
# This script helps set up local development environment

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

echo -e "${BLUE}🛠️  C++ REST Server - Local Development Setup${NC}"
echo -e "${BLUE}=============================================${NC}"
echo ""

# Check if Docker is available
if command -v docker >/dev/null 2>&1 && command -v docker-compose >/dev/null 2>&1; then
    print_status "Docker and Docker Compose found"

    echo ""
    echo -e "${YELLOW}Option 1: Docker Development Environment${NC}"
    echo "This creates a containerized environment identical to your server"
    echo ""
    echo "Commands:"
    echo "  docker-compose up -d     # Start development container"
    echo "  docker-compose exec cpp-dev bash  # Enter container"
    echo "  docker-compose down      # Stop container"
    echo ""
else
    print_warning "Docker not found. Local installation required."
fi

# Check current OS
if command -v lsb_release >/dev/null 2>&1; then
    OS_NAME=$(lsb_release -is)
    OS_VERSION=$(lsb_release -rs)
    print_status "Detected OS: $OS_NAME $OS_VERSION"

    if [[ "$OS_NAME" == "LinuxMint" ]]; then
        echo ""
        echo -e "${YELLOW}Option 2: Native Linux Mint Development${NC}"
        echo "Install dependencies directly on Linux Mint:"
        echo ""
        echo "# Update system"
        echo "sudo apt update && sudo apt upgrade -y"
        echo ""
        echo "# Install build tools"
        echo "sudo apt install -y build-essential cmake git curl"
        echo ""
        echo "# Install development libraries"
        echo "sudo apt install -y libcurl4-openssl-dev rapidjson-dev"
        echo ""
        echo "# Install Google Test"
        echo "sudo apt install -y libgtest-dev libgmock-dev"
        echo ""
        echo "# Install Pistache (if available) or compile from source"
        echo "sudo apt install -y libpistache-dev || echo 'Need to compile Pistache manually'"
        echo ""
    fi
fi

echo ""
echo -e "${YELLOW}Setup Steps for Local Development:${NC}"
echo ""
echo "1. 📋 Copy project to your local Linux Mint machine"
echo "2. Choose development method (Docker or Native)"
echo "3. ⚙️  Configure VS Code settings:"
echo "   - Copy .vscode/settings-local-dev.json to .vscode/settings.json"
echo "   - Copy .vscode/tasks-local-dev.json to .vscode/tasks.json"
echo "   - Update remote server details in settings.json"
echo "4. 🔑 Set up SSH key authentication to your server"
echo "5. Use VS Code tasks for deploy & remote execution"
echo ""
echo -e "${YELLOW}VS Code Task Usage:${NC}"
echo "  Ctrl+Shift+P → 'Tasks: Run Task' → '🔄 Full Deploy & Run'"
echo "  Ctrl+Shift+P → 'Tasks: Run Task' → '⚡ Quick Deploy & Build'"
echo "  Ctrl+Shift+P → 'Tasks: Run Task' → 'Deploy to Server'"
echo ""
echo -e "${YELLOW}SSH Setup (run on your local machine):${NC}"
echo "  ssh-keygen -t rsa -b 4096 -C 'your_email@example.com'"
echo "  ssh-copy-id rdias@fedora-server.local"
echo ""

if [ "$1" = "--docker" ]; then
    print_status "Starting Docker development environment..."
    docker-compose up -d
    print_success "Docker container started. Connect with:"
    echo "  docker-compose exec cpp-dev bash"
fi
