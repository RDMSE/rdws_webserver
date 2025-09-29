#!/bin/bash

# C++ REST Server Service Manager
# Usage: ./service_manager.sh {start|stop|restart|status|logs|install|uninstall}

set -e

SERVICE_NAME="cpp-rest-server"
SERVICE_FILE="cpp-rest-server.service"

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

case "$1" in
    start)
        print_status "Starting $SERVICE_NAME service..."
        sudo systemctl start $SERVICE_NAME
        if sudo systemctl is-active --quiet $SERVICE_NAME; then
            print_success "Service started successfully"
            sudo systemctl status $SERVICE_NAME --no-pager
        else
            print_error "Failed to start service"
            exit 1
        fi
        ;;
    
    stop)
        print_status "Stopping $SERVICE_NAME service..."
        sudo systemctl stop $SERVICE_NAME
        print_success "Service stopped"
        ;;
    
    restart)
        print_status "Restarting $SERVICE_NAME service..."
        sudo systemctl restart $SERVICE_NAME
        if sudo systemctl is-active --quiet $SERVICE_NAME; then
            print_success "Service restarted successfully"
        else
            print_error "Failed to restart service"
            exit 1
        fi
        ;;
    
    status)
        print_status "Checking $SERVICE_NAME service status..."
        sudo systemctl status $SERVICE_NAME --no-pager
        ;;
    
    logs)
        print_status "Showing $SERVICE_NAME service logs..."
        sudo journalctl -u $SERVICE_NAME -f
        ;;
    
    install)
        if [ ! -f "$SERVICE_FILE" ]; then
            print_error "Service file $SERVICE_FILE not found"
            exit 1
        fi
        
        print_status "Installing $SERVICE_NAME service..."
        sudo cp $SERVICE_FILE /etc/systemd/system/
        sudo systemctl daemon-reload
        sudo systemctl enable $SERVICE_NAME
        print_success "Service installed and enabled"
        print_status "Use './service_manager.sh start' to start the service"
        ;;
    
    uninstall)
        print_status "Uninstalling $SERVICE_NAME service..."
        sudo systemctl stop $SERVICE_NAME 2>/dev/null || true
        sudo systemctl disable $SERVICE_NAME 2>/dev/null || true
        sudo rm -f /etc/systemd/system/$SERVICE_FILE
        sudo systemctl daemon-reload
        print_success "Service uninstalled"
        ;;
    
    *)
        echo "Usage: $0 {start|stop|restart|status|logs|install|uninstall}"
        echo ""
        echo "Commands:"
        echo "  start     - Start the service"
        echo "  stop      - Stop the service"
        echo "  restart   - Restart the service"
        echo "  status    - Show service status"
        echo "  logs      - Show service logs (follow)"
        echo "  install   - Install service to systemd"
        echo "  uninstall - Remove service from systemd"
        exit 1
        ;;
esac