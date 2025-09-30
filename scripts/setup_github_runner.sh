#!/bin/bash

# GitHub Self-Hosted Runner Setup Script for Fedora Server
# This script installs and configures a GitHub Actions runner on your Fedora server

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

echo -e "${BLUE}🏃 GitHub Actions Self-Hosted Runner Setup${NC}"
echo -e "${BLUE}===========================================${NC}"
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    print_error "Please do not run this script as root. Run as your regular user (rdias)."
    exit 1
fi

# Check if we're on Fedora
if ! grep -qi "fedora" /etc/os-release; then
    print_warning "This script is designed for Fedora. You may need to adapt package names."
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Variables
RUNNER_USER=$(whoami)
RUNNER_HOME="/home/$RUNNER_USER"
RUNNER_DIR="$RUNNER_HOME/actions-runner"
SERVICE_NAME="actions-runner"

print_status "Setting up GitHub Actions runner for user: $RUNNER_USER"
print_status "Runner will be installed in: $RUNNER_DIR"
echo ""

# Install required packages
print_status "Installing required packages..."
sudo dnf update -y
sudo dnf install -y \
    curl \
    wget \
    tar \
    gzip \
    git \
    jq \
    systemd

print_success "Required packages installed"

# Create runner directory
print_status "Creating runner directory..."
mkdir -p "$RUNNER_DIR"
cd "$RUNNER_DIR"

# Download and extract the latest runner
print_status "Downloading GitHub Actions runner..."
RUNNER_VERSION=$(curl -s https://api.github.com/repos/actions/runner/releases/latest | jq -r '.tag_name' | sed 's/v//')
RUNNER_URL="https://github.com/actions/runner/releases/download/v${RUNNER_VERSION}/actions-runner-linux-x64-${RUNNER_VERSION}.tar.gz"

print_status "Downloading runner version: $RUNNER_VERSION"
curl -o actions-runner-linux-x64.tar.gz -L "$RUNNER_URL"

print_status "Extracting runner..."
tar xzf actions-runner-linux-x64.tar.gz
rm actions-runner-linux-x64.tar.gz

print_success "Runner downloaded and extracted"

# Create configuration script
print_status "Creating configuration helper script..."
cat > configure_runner.sh << 'EOF'
#!/bin/bash
echo "GitHub Actions Runner Configuration"
echo "=================================="
echo ""
echo "To configure this runner, you need:"
echo "1. Your GitHub repository URL (e.g., https://github.com/rdmeneze/rdws_webserver)"
echo "2. A registration token from GitHub"
echo ""
echo "Steps to get the token:"
echo "1. Go to your repository on GitHub"
echo "2. Settings → Actions → Runners"
echo "3. Click 'New self-hosted runner'"
echo "4. Copy the token from the configuration command"
echo ""
read -p "Enter your repository URL: " REPO_URL
read -p "Enter the registration token: " REG_TOKEN
read -p "Enter runner name (default: fedora-server): " RUNNER_NAME
RUNNER_NAME=${RUNNER_NAME:-fedora-server}

echo ""
echo "Configuring runner..."
./config.sh --url "$REPO_URL" --token "$REG_TOKEN" --name "$RUNNER_NAME" --work _work --labels fedora,self-hosted,linux,x64 --unattended

if [ $? -eq 0 ]; then
    echo "Runner configured successfully!"
    echo "You can now install it as a service by running: sudo ./install_service.sh"
else
    echo "Configuration failed. Please check your token and try again."
    exit 1
fi
EOF

chmod +x configure_runner.sh

# Create service installation script
print_status "Creating service installation script..."
cat > install_service.sh << EOF
#!/bin/bash
# Install GitHub Actions Runner as systemd service

if [ "\$EUID" -ne 0 ]; then
    echo "Please run this script as root (sudo ./install_service.sh)"
    exit 1
fi

# Install the service
sudo ./svc.sh install $RUNNER_USER

# Start and enable the service
sudo ./svc.sh start

# Check status
sudo ./svc.sh status

echo ""
echo "GitHub Actions Runner service installed and started!"
echo "📊 Service status: systemctl status actions.runner.*"
echo "🔍 View logs: journalctl -u actions.runner.* -f"
echo ""
echo "The runner should now appear as 'Online' in your GitHub repository settings."
EOF

chmod +x install_service.sh

# Create uninstall script
print_status "Creating uninstall script..."
cat > uninstall_runner.sh << 'EOF'
#!/bin/bash
# Uninstall GitHub Actions Runner

echo "Uninstalling GitHub Actions Runner..."

# Stop and uninstall service
if systemctl is-active --quiet actions.runner.*.service; then
    echo "Stopping service..."
    sudo ./svc.sh stop
    sudo ./svc.sh uninstall
fi

# Remove runner configuration
if [ -f .runner ]; then
    echo "Removing runner configuration..."
    echo "You'll need a removal token from GitHub:"
    echo "Go to Settings → Actions → Runners → Click on your runner → Remove"
    read -p "Enter the removal token: " REMOVAL_TOKEN
    ./config.sh remove --token "$REMOVAL_TOKEN"
fi

echo "Runner uninstalled"
EOF

chmod +x uninstall_runner.sh

# Create status check script
print_status "Creating status check script..."
cat > check_runner.sh << 'EOF'
#!/bin/bash
# Check GitHub Actions Runner status

echo "🏃 GitHub Actions Runner Status"
echo "==============================="
echo ""

# Check if runner is configured
if [ -f .runner ]; then
    echo "Runner is configured"
    echo "Runner info:"
    cat .runner | jq '.'
    echo ""
else
    echo "Runner is not configured"
    echo "Run ./configure_runner.sh to set up the runner"
    exit 1
fi

# Check service status
if systemctl is-active --quiet actions.runner.*.service; then
    echo "Service is running"
    systemctl status actions.runner.*.service --no-pager -l
else
    echo "Service is not running"
    echo "Available services:"
    systemctl list-units --type=service | grep actions.runner || echo "No runner services found"
fi

echo ""
echo "Recent logs:"
journalctl -u actions.runner.* --since="10 minutes ago" --no-pager | tail -10

echo ""
echo "System resources:"
echo "CPU: $(nproc) cores"
echo "Memory: $(free -h | awk '/^Mem:/ {print $2}')"
echo "Disk: $(df -h . | awk 'NR==2 {print $4 " available"}')"
EOF

chmod +x check_runner.sh

# Create startup script
print_status "Creating manual startup script..."
cat > start_runner.sh << 'EOF'
#!/bin/bash
# Start GitHub Actions Runner manually (for testing)

echo "Starting GitHub Actions Runner..."
echo "===================================="
echo ""

if [ ! -f .runner ]; then
    echo "Runner is not configured"
    echo "Run ./configure_runner.sh first"
    exit 1
fi

echo "Starting runner..."
echo "Press Ctrl+C to stop the runner"
echo ""

./run.sh
EOF

chmod +x start_runner.sh

print_success "Scripts created successfully!"
echo ""

print_status "Creating README for runner setup..."
cat > RUNNER_README.md << 'EOF'
# GitHub Actions Self-Hosted Runner

This directory contains a GitHub Actions self-hosted runner for your Fedora server.

## Quick Setup

1. **Configure the runner:**
   ```bash
   ./configure_runner.sh
   ```
   You'll need:
   - Repository URL: `https://github.com/rdmeneze/rdws_webserver`
   - Registration token from GitHub (Settings → Actions → Runners → New self-hosted runner)

2. **Install as service:**
   ```bash
   sudo ./install_service.sh
   ```

3. **Check status:**
   ```bash
   ./check_runner.sh
   ```

## Available Scripts

- `configure_runner.sh` - Configure the runner with GitHub
- `install_service.sh` - Install as systemd service (run with sudo)
- `uninstall_runner.sh` - Remove the runner
- `check_runner.sh` - Check runner status
- `start_runner.sh` - Start runner manually (for testing)

## Service Management

```bash
# Check service status
sudo systemctl status actions.runner.*

# View logs
sudo journalctl -u actions.runner.* -f

# Start/stop service
sudo systemctl start actions.runner.*
sudo systemctl stop actions.runner.*
```

## Troubleshooting

### Runner not appearing online
- Check if service is running: `sudo systemctl status actions.runner.*`
- Check logs: `sudo journalctl -u actions.runner.* -f`
- Verify network connectivity: `curl https://api.github.com`

### Permission issues
- Ensure runner is owned by correct user: `ls -la`
- Check file permissions: `chmod +x *.sh`

### Service won't start
- Check if runner is configured: `ls -la .runner`
- Try manual start: `./start_runner.sh`
- Check system resources: `free -h && df -h`

## Updating Runner

1. Stop service: `sudo systemctl stop actions.runner.*`
2. Remove old installation: `./uninstall_runner.sh`
3. Download new runner version
4. Reconfigure: `./configure_runner.sh`
5. Reinstall service: `sudo ./install_service.sh`
EOF

print_success "Setup complete!"
echo ""
echo -e "${YELLOW}Next Steps:${NC}"
echo "1. Configure the runner: ./configure_runner.sh"
echo "2. Install as service: sudo ./install_service.sh"
echo "3. Check status: ./check_runner.sh"
echo ""
echo -e "${YELLOW}Documentation:${NC}"
echo "- Runner setup guide: cat RUNNER_README.md"
echo "- GitHub docs: https://docs.github.com/en/actions/hosting-your-own-runners"
echo ""
echo -e "${GREEN}GitHub Actions runner is ready for configuration!${NC}"
