#!/bin/bash

# Deploy script for Fedora Server
# API Gateway + C++ Microservices deploy

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# configurations
PROJECT_DIR="/opt/rdws_webserver"
SERVICE_NAME="api-gateway"
PORT=8080

print_status "Starting deployment on Fedora Server..."

# Check if running as root for some operations
if [[ $EUID -eq 0 ]]; then
    print_warning "Running as root. Some operations will be adjusted."
    IS_ROOT=true
else
    IS_ROOT=false
fi

# 1. Check dependencies
print_status "📋 Checking dependencies..."

# Node.js
if ! command -v node &> /dev/null; then
    print_error "Node.js not found. Installing..."
    if [ "$IS_ROOT" = true ]; then
        curl -fsSL https://rpm.nodesource.com/setup_lts.x | bash -
        dnf install -y nodejs
    else
        print_error "Please install Node.js first or run as root"
        exit 1
    fi
else
    print_success "Node.js found: $(node --version)"
fi

# CMake and build tools
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found. Installing..."
    if [ "$IS_ROOT" = true ]; then
        dnf install -y cmake gcc-c++ make
    else
        print_error "Please install build tools first: sudo dnf install cmake gcc-c++ make"
        exit 1
    fi
else
    print_success "CMake found: $(cmake --version | head -1)"
fi

# 2. Prepare project directory
print_status "Preparing project directory..."

if [ ! -d "$PROJECT_DIR" ]; then
    print_status "Creating project directory: $PROJECT_DIR"
    if [ "$IS_ROOT" = true ]; then
        mkdir -p "$PROJECT_DIR"
    else
        sudo mkdir -p "$PROJECT_DIR"
        sudo chown $USER:$USER "$PROJECT_DIR"
    fi
fi

cd "$PROJECT_DIR"

# 3. Update code (if it already exists) or clone
if [ -d ".git" ]; then
    print_status "Updating existing repository..."
    git pull origin 14-featureserverinfra-enable-servless-archtecture
else
    print_status "Cloning repository..."
    if [ "$(ls -A)" ]; then
        print_warning "Directory not empty. Backing up to ${PROJECT_DIR}.backup"
        if [ "$IS_ROOT" = true ]; then
            mv "$PROJECT_DIR" "${PROJECT_DIR}.backup"
            mkdir -p "$PROJECT_DIR"
            cd "$PROJECT_DIR"
        else
            sudo mv "$PROJECT_DIR" "${PROJECT_DIR}.backup"
            sudo mkdir -p "$PROJECT_DIR"
            sudo chown $USER:$USER "$PROJECT_DIR"
            cd "$PROJECT_DIR"
        fi
    fi
    
    git clone https://github.com/RDMSE/rdws_webserver.git .
    git checkout 14-featureserverinfra-enable-servless-archtecture
fi

# 4. Install Node.js dependencies
print_status "Installing Node.js dependencies..."
npm install --production

# 5. Build C++ microservices
print_status "Building C++ microservices..."
mkdir -p build
cd build

# Clean previous build if exists
if [ -f "Makefile" ]; then
    make clean || true
fi

cmake ..
make -j$(nproc)

# Check if executables were created
if [ ! -f "services/users/users_service" ] || [ ! -f "services/orders/orders_service" ]; then
    print_error "Failed to build microservices!"
    exit 1
fi

print_success "Microservices built successfully!"
cd ..

# 6. Configure firewall
print_status "🔥 Configuring firewall..."
if command -v firewall-cmd &> /dev/null; then
    if [ "$IS_ROOT" = true ]; then
        firewall-cmd --permanent --add-port=${PORT}/tcp || true
        firewall-cmd --reload || true
        print_success "Firewall configured for port ${PORT}"
    else
        print_warning "Please configure firewall manually: sudo firewall-cmd --permanent --add-port=${PORT}/tcp && sudo firewall-cmd --reload"
    fi
else
    print_warning "firewalld not found. Please configure firewall manually."
fi

# 7. Configure and start service

print_status "Configuring service..."

# Choose deployment method based on what's available
if command -v pm2 &> /dev/null; then
    print_status "Using PM2 for process management..."

    # Create PM2 configuration
    cat > ecosystem.config.js << EOF
module.exports = {
  apps: [{
    name: '${SERVICE_NAME}',
    script: './api-gateway.js',
    instances: 'max',
    exec_mode: 'cluster',
    env: {
      NODE_ENV: 'production',
      PORT: ${PORT},
      BUILD_PATH: '${PROJECT_DIR}/build',
      SERVICE_TIMEOUT: 5000
    },
    error_file: '/var/log/${SERVICE_NAME}/error.log',
    out_file: '/var/log/${SERVICE_NAME}/out.log',
    log_file: '/var/log/${SERVICE_NAME}/combined.log',
    time: true,
    max_memory_restart: '500M'
  }]
};
EOF

    # Create log directory
    if [ "$IS_ROOT" = true ]; then
        mkdir -p "/var/log/${SERVICE_NAME}"
        chown $USER:$USER "/var/log/${SERVICE_NAME}"
    else
        sudo mkdir -p "/var/log/${SERVICE_NAME}"
        sudo chown $USER:$USER "/var/log/${SERVICE_NAME}"
    fi

    # Stop previous instance if exists
    pm2 stop ${SERVICE_NAME} || true
    pm2 delete ${SERVICE_NAME} || true

    # Start new instance
    pm2 start ecosystem.config.js
    pm2 save

    # Configure PM2 startup if necessary
    if [ "$IS_ROOT" = false ]; then
        print_status "Setting up PM2 startup..."
        pm2 startup | grep "sudo" | bash || print_warning "Failed to setup PM2 startup"
    fi

elif command -v docker &> /dev/null; then
    print_status "Using Docker for deployment..."

    # Stop previous container if exists
    docker stop ${SERVICE_NAME} || true
    docker rm ${SERVICE_NAME} || true

    # Build and run
    docker build -f Dockerfile.gateway -t ${SERVICE_NAME} .
    docker run -d \
        --name ${SERVICE_NAME} \
        --restart unless-stopped \
        -p ${PORT}:${PORT} \
        -v "${PROJECT_DIR}/build:/app/build:ro" \
        -e NODE_ENV=production \
        -e PORT=${PORT} \
        -e BUILD_PATH=/app/build \
        ${SERVICE_NAME}

else
    print_status "Using systemd for service management..."

    # Create systemd service file
    SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"
    
    if [ "$IS_ROOT" = true ]; then
        cat > ${SERVICE_FILE} << EOF
[Unit]
Description=C++ Microservices API Gateway
Documentation=https://github.com/RDMSE/rdws_webserver
After=network.target
Wants=network.target

[Service]
Type=simple
User=$USER
Group=$USER
WorkingDirectory=${PROJECT_DIR}
ExecStart=/usr/bin/node api-gateway.js
Restart=always
RestartSec=3
TimeoutStopSec=10

# Environment
Environment=NODE_ENV=production
Environment=PORT=${PORT}
Environment=BUILD_PATH=${PROJECT_DIR}/build
Environment=SERVICE_TIMEOUT=5000

# Logging
StandardOutput=journal
StandardError=journal
SyslogIdentifier=${SERVICE_NAME}

[Install]
WantedBy=multi-user.target
EOF

        systemctl daemon-reload
        systemctl enable ${SERVICE_NAME}
        systemctl restart ${SERVICE_NAME}
    else
        print_error "Need root privileges for systemd service. Please run with sudo or install PM2/Docker"
        exit 1
    fi
fi

# 8. Wait for service to start
print_status "Waiting for service to start..."
sleep 5

# 9. Check service health
print_status "Performing health check..."

HEALTH_URL="http://localhost:${PORT}/health"
MAX_RETRIES=10
RETRY_COUNT=0

while [ $RETRY_COUNT -lt $MAX_RETRIES ]; do
    if curl -f -s $HEALTH_URL > /dev/null; then
        print_success "Service is healthy!"
        break
    else
        RETRY_COUNT=$((RETRY_COUNT + 1))
        print_status "Health check attempt $RETRY_COUNT/$MAX_RETRIES..."
        sleep 2
    fi
done

if [ $RETRY_COUNT -eq $MAX_RETRIES ]; then
    print_error "Service health check failed after $MAX_RETRIES attempts!"
    print_error "Please check logs and troubleshoot manually."
    exit 1
fi

# 10. Show final information
print_success "Deployment completed successfully!"
echo ""
print_status "Service Information:"
echo "  URL: http://$(hostname -I | awk '{print $1}'):${PORT}"
echo "  Health: ${HEALTH_URL}"
echo "  Project: ${PROJECT_DIR}"
echo ""
print_status "Available endpoints:"
echo "  GET  /health      - Service health check"
echo "  GET  /users       - List all users"
echo "  GET  /users/:id   - Get user by ID"
echo "  GET  /orders      - List all orders"
echo "  GET  /orders/:id  - Get order by ID"
echo "  GET  /api-docs    - API documentation"
echo ""

# Show useful commands based on the method used
if command -v pm2 &> /dev/null; then
    print_status "Useful PM2 commands:"
    echo "  pm2 status                - Show service status"
    echo "  pm2 logs ${SERVICE_NAME}  - View logs"
    echo "  pm2 restart ${SERVICE_NAME} - Restart service"
    echo "  pm2 monit                 - Monitor dashboard"
elif command -v docker &> /dev/null; then
    print_status "Useful Docker commands:"
    echo "  docker ps                 - Show running containers"
    echo "  docker logs ${SERVICE_NAME} - View logs"
    echo "  docker restart ${SERVICE_NAME} - Restart service"
else
    print_status "Useful systemd commands:"
    echo "  sudo systemctl status ${SERVICE_NAME}  - Show service status"
    echo "  sudo journalctl -u ${SERVICE_NAME} -f  - View logs"
    echo "  sudo systemctl restart ${SERVICE_NAME} - Restart service"
fi

print_success "C++ microservices API Gateway is now running!"