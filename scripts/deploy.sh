# DEPRECATED: This script was designed for Pistache server - needs update for microservices architecture
#!/bin/bash

# Deploy script for C++ REST Server
# Usage: ./deploy.sh [server_host] [server_user] [remote_path]

set -e  # Exit on any error

# Configuration (modify these for your setup)
SERVER_HOST="${1:-fedora-server.local}"
SERVER_USER="${2:-rdias}"
REMOTE_PATH="${3:-/home/rdias/sources/lab/server}"
LOCAL_PROJECT_DIR="$(dirname "$0")/.."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}C++ REST Server Deployment${NC}"
echo -e "${BLUE}==============================${NC}"
echo -e "Local project: ${LOCAL_PROJECT_DIR}"
echo -e "Remote server: ${SERVER_USER}@${SERVER_HOST}:${REMOTE_PATH}"
echo ""

# Function to print status
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

# Check if SSH connection works
print_status "Testing SSH connection..."
if ssh -o ConnectTimeout=5 "${SERVER_USER}@${SERVER_HOST}" "echo 'SSH connection successful'" 2>/dev/null; then
    print_success "SSH connection established"
else
    print_error "Failed to connect to ${SERVER_USER}@${SERVER_HOST}"
    print_error "Please check your SSH configuration and server access"
    exit 1
fi

# Create .deployignore if it doesn't exist
if [ ! -f "${LOCAL_PROJECT_DIR}/.deployignore" ]; then
    print_status "Creating .deployignore file..."
    cat > "${LOCAL_PROJECT_DIR}/.deployignore" << 'EOF'
# Build artifacts
build/
*.o
*.a
*.so
*.dylib

# IDE files
.vscode/settings.json
.vscode/launch.json
.idea/

# OS files
.DS_Store
Thumbs.db

# Logs
*.log

# Temporary files
*.tmp
*.temp
*~

# Git (optional - uncomment if you don't want .git on server)
# .git/
EOF
    print_success "Created .deployignore file"
fi

# Sync files using rsync
print_status "Syncing files to remote server..."

# Build rsync exclude options from .deployignore
EXCLUDE_OPTS=""
if [ -f "${LOCAL_PROJECT_DIR}/.deployignore" ]; then
    EXCLUDE_OPTS="--exclude-from=${LOCAL_PROJECT_DIR}/.deployignore"
fi

# Perform the sync
rsync -avz --delete \
    ${EXCLUDE_OPTS} \
    --progress \
    "${LOCAL_PROJECT_DIR}/" \
    "${SERVER_USER}@${SERVER_HOST}:${REMOTE_PATH}/"

if [ $? -eq 0 ]; then
    print_success "Files synchronized successfully"
else
    print_error "Failed to synchronize files"
    exit 1
fi

# Make scripts executable on remote server
print_status "Setting executable permissions on remote scripts..."
ssh "${SERVER_USER}@${SERVER_HOST}" "chmod +x ${REMOTE_PATH}/scripts/*.sh" 2>/dev/null || true

print_success "Deployment completed successfully!"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "1. Build on server: ssh ${SERVER_USER}@${SERVER_HOST} 'cd ${REMOTE_PATH} && ./scripts/remote_build.sh'"
echo "2. Run tests: ssh ${SERVER_USER}@${SERVER_HOST} 'cd ${REMOTE_PATH} && ./scripts/remote_test.sh'"
echo "3. Start server: ssh ${SERVER_USER}@${SERVER_HOST} 'cd ${REMOTE_PATH} && ./scripts/remote_run.sh'"
echo ""
