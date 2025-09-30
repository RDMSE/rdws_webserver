#!/bin/bash

# Remote run script for C++ REST Server
# This script runs on the remote server to start the application

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[RUN]${NC} $1"
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

# Parse command line arguments
BACKGROUND=false
STOP=false
STATUS=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--background)
            BACKGROUND=true
            shift
            ;;
        -s|--stop)
            STOP=true
            shift
            ;;
        --status)
            STATUS=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -b, --background    Run server in background"
            echo "  -s, --stop         Stop running server"
            echo "      --status       Show server status"
            echo "  -h, --help         Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option $1"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}C++ REST Server Remote Control${NC}"
echo -e "${BLUE}=================================${NC}"

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PID_FILE="$PROJECT_ROOT/server.pid"

# Function to check if server is running
is_server_running() {
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            return 0  # Running
        else
            rm -f "$PID_FILE"  # Remove stale PID file
            return 1  # Not running
        fi
    else
        return 1  # Not running
    fi
}

# Show status
if [ "$STATUS" = true ]; then
    if is_server_running; then
        local pid=$(cat "$PID_FILE")
        print_success "Server is running (PID: $pid)"
        print_status "Listening on port 9080"
        echo ""
        echo "Test endpoints:"
        echo "  curl http://$(hostname -I | awk '{print $1}'):9080/hello"
        echo "  curl http://$(hostname -I | awk '{print $1}'):9080/"
    else
        print_warning "Server is not running"
    fi
    exit 0
fi

# Stop server
if [ "$STOP" = true ]; then
    if is_server_running; then
        local pid=$(cat "$PID_FILE")
        print_status "Stopping server (PID: $pid)..."
        kill "$pid"

        # Wait for server to stop
        local count=0
        while kill -0 "$pid" 2>/dev/null && [ $count -lt 10 ]; do
            sleep 1
            count=$((count + 1))
        done

        if kill -0 "$pid" 2>/dev/null; then
            print_warning "Server didn't stop gracefully, forcing..."
            kill -9 "$pid"
        fi

        rm -f "$PID_FILE"
        print_success "Server stopped successfully"
    else
        print_warning "Server is not running"
    fi
    exit 0
fi

# Check if build exists
cd "$PROJECT_ROOT"

if [ ! -f "build/rest_server" ]; then
    print_error "Server executable not found. Please run ./scripts/remote_build.sh first"
    exit 1
fi

# Stop any existing server
if is_server_running; then
    print_warning "Server is already running. Stopping it first..."
    $0 --stop
    sleep 2
fi

cd build

print_status "Starting C++ REST Server..."

if [ "$BACKGROUND" = true ]; then
    # Run in background
    nohup ./rest_server > ../server.log 2>&1 &
    server_pid=$!
    echo $server_pid > "$PID_FILE"

    # Give server time to start
    sleep 2

    if is_server_running; then
        print_success "Server started in background (PID: $server_pid)"
        print_success "Log file: $PROJECT_ROOT/server.log"
        echo ""
        echo -e "${YELLOW}Server endpoints:${NC}"
        echo "  http://$(hostname -I | awk '{print $1}'):9080/hello"
        echo "  http://$(hostname -I | awk '{print $1}'):9080/"
        echo ""
        echo -e "${YELLOW}Management commands:${NC}"
        echo "  ./scripts/remote_run.sh --status   # Check status"
        echo "  ./scripts/remote_run.sh --stop     # Stop server"
        echo "  tail -f server.log                 # View logs"
    else
        print_error "Failed to start server in background"
        exit 1
    fi
else
    # Run in foreground
    print_status "Server will run in foreground. Press Ctrl+C to stop."
    print_success "Starting server..."
    echo ""
    ./rest_server
fi
