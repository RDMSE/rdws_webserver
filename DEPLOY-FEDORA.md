````markdown
# 🚀 Fedora Server Deployment - API Gateway + C++ Microservices

Complete guide to deploy your microservices architecture on Fedora Server.

## Server Prerequisites

### 1. Updated System
```bash
sudo dnf update -y
```

### 2. Install Base Dependencies
```bash
# Development tools
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y cmake gcc-c++ git wget curl

# Node.js (LTS version)
curl -fsSL https://rpm.nodesource.com/setup_lts.x | sudo bash -
sudo dnf install -y nodejs

# PM2 to manage Node.js processes
sudo npm install -g pm2

# Firewall and network
sudo dnf install -y firewalld
```

### 3. Configure Firewall
```bash
# Enable firewall
sudo systemctl enable --now firewalld

# Open API Gateway port
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload

# Verify
sudo firewall-cmd --list-ports
```

## Manual Deployment (Method 1)

### 1. Clone and Prepare Project
```bash
# On the server
cd /opt
sudo git clone https://github.com/RDMSE/rdws_webserver.git
sudo chown -R $USER:$USER rdws_webserver
cd rdws_webserver

# Checkout correct branch
git checkout 14-featureserverinfra-enable-servless-archtecture
```

### 2. Install Node.js Dependencies
```bash
npm install --production
```

### 3. Compile C++ Microservices
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Verify executables were created
ls -la services/*/
```

### 4. Configure Environment Variables
```bash
# Create configuration file
sudo tee /opt/rdws_webserver/.env << 'EOF'
NODE_ENV=production
PORT=8080
BUILD_PATH=/opt/rdws_webserver/build
SERVICE_TIMEOUT=5000
EOF
```

### 5. Configure PM2
```bash
cd /opt/rdws_webserver

# Create PM2 configuration file
tee ecosystem.config.js << 'EOF'
module.exports = {
  apps: [{
    name: 'api-gateway',
    script: './api-gateway.js',
    instances: 'max',
    exec_mode: 'cluster',
    env: {
      NODE_ENV: 'production',
      PORT: 8080,
      BUILD_PATH: '/opt/rdws_webserver/build',
      SERVICE_TIMEOUT: 5000
    },
    error_file: '/var/log/api-gateway/error.log',
    out_file: '/var/log/api-gateway/out.log',
    log_file: '/var/log/api-gateway/combined.log',
    time: true,
    max_memory_restart: '500M',
    node_args: '--max-old-space-size=512'
  }]
};
EOF

# Create logs directory
sudo mkdir -p /var/log/api-gateway
sudo chown $USER:$USER /var/log/api-gateway

# Start application
pm2 start ecosystem.config.js
pm2 save
pm2 startup

# Check status
pm2 status
pm2 logs
```

## Docker Deployment (Method 2 - Recommended)

### 1. Install Docker
```bash
# Remove old versions
sudo dnf remove docker docker-client docker-client-latest docker-common docker-latest docker-latest-logrotate docker-logrotate docker-engine

# Add Docker repository
sudo dnf config-manager --add-repo https://download.docker.com/linux/fedora/docker-ce.repo

# Install Docker
sudo dnf install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# Enable and start Docker
sudo systemctl enable --now docker

# Add user to docker group
sudo usermod -aG docker $USER
newgrp docker

# Verify installation
docker --version
docker compose version
```

### 2. Deploy with Docker Compose
```bash
cd /opt/rdws_webserver

# Compile microservices first (needed for volume)
mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ..

# Deploy
docker compose up -d api-gateway

# Check status
docker compose ps
docker compose logs api-gateway
```

## System Configuration (Systemd)

### Method 3: Native Systemd Service

```bash
# Create service file
sudo tee /etc/systemd/system/api-gateway.service << 'EOF'
[Unit]
Description=C++ Microservices API Gateway
Documentation=https://github.com/RDMSE/rdws_webserver
After=network.target
Wants=network.target

[Service]
Type=simple
User=rdias
Group=rdias
WorkingDirectory=/opt/rdws_webserver
ExecStart=/usr/bin/node api-gateway.js
Restart=always
RestartSec=3
TimeoutStopSec=10

# Environment
Environment=NODE_ENV=production
Environment=PORT=8080
Environment=BUILD_PATH=/opt/rdws_webserver/build
Environment=SERVICE_TIMEOUT=5000

# Logging
StandardOutput=journal
StandardError=journal
SyslogIdentifier=api-gateway

# Security
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ReadWritePaths=/opt/rdws_webserver
ProtectHome=true

[Install]
WantedBy=multi-user.target
EOF

# Reload systemd
sudo systemctl daemon-reload

# Enable and start service
sudo systemctl enable api-gateway
sudo systemctl start api-gateway

# Check status
sudo systemctl status api-gateway
sudo journalctl -u api-gateway -f
```

## Deployment Verification

### 1. Test API Gateway
```bash
# Health check
curl http://localhost:8080/health

# Test endpoints
curl http://localhost:8080/users
curl http://localhost:8080/orders
curl http://localhost:8080/api-docs
```

### 2. Test from External Machine
```bash
# Replace SERVER_IP with your server's IP
curl http://SERVER_IP:8080/health
```

### 3. Check Logs
```bash
# PM2
pm2 logs api-gateway

# Docker
docker compose logs api-gateway -f

# Systemd
sudo journalctl -u api-gateway -f
```

## Monitoring

### 1. PM2 Monitoring
```bash
# Detailed status
pm2 monit

# Restart if needed
pm2 restart api-gateway

# View metrics
pm2 show api-gateway
```

### 2. Docker Monitoring
```bash
# Container status
docker compose ps

# Resource usage
docker stats

# Health checks
docker compose exec api-gateway node -e "require('http').get('http://localhost:8080/health', res => console.log(res.statusCode))"
```

### 3. System Logs
```bash
# System logs
sudo journalctl -f

# Service-specific logs
sudo journalctl -u api-gateway --since "1 hour ago"
```

## Security Configuration

### 1. SELinux (if enabled)
```bash
# Check SELinux status
getenforce

# If needed, create custom policies
sudo setsebool -P httpd_can_network_connect 1
```

### 2. Configure Nginx as Proxy (Optional)
```bash
# Install Nginx
sudo dnf install -y nginx

# Configure reverse proxy
sudo tee /etc/nginx/conf.d/api-gateway.conf << 'EOF'
server {
    listen 80;
    server_name your-domain.com;

    location / {
        proxy_pass http://localhost:8080;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_cache_bypass $http_upgrade;
    }
}
EOF

# Enable and start Nginx
sudo systemctl enable --now nginx

# Open port 80 in firewall
sudo firewall-cmd --permanent --add-service=http
sudo firewall-cmd --reload
```

## Automated Deployment Script

Create a script to automate deployment:

```bash
# Create deployment script
tee deploy-fedora.sh << 'EOF'
#!/bin/bash

set -e

echo "Starting deployment on Fedora Server..."

# Update code
echo "Updating code..."
git pull origin 14-featureserverinfra-enable-servless-archtecture

# Install dependencies
echo "Installing dependencies..."
npm install --production

# Compile microservices
echo "Building microservices..."
cd build
make -j$(nproc)
cd ..

# Restart service
echo "Restarting service..."
if command -v pm2 &> /dev/null; then
    pm2 restart api-gateway
elif command -v docker &> /dev/null; then
    docker compose restart api-gateway
else
    sudo systemctl restart api-gateway
fi

# Health check
echo "Health check..."
sleep 5
curl -f http://localhost:8080/health > /dev/null && echo "Service is healthy!" || echo "Service health check failed!"

echo "Deployment completed!"
EOF

chmod +x deploy-fedora.sh
```

## Deployment Checklist

- [ ] Updated Fedora server
- [ ] Dependencies installed (Node.js, C++, CMake)
- [ ] Project cloned in `/opt/rdws_webserver`
- [ ] Microservices compiled
- [ ] Firewall configured (port 8080)
- [ ] Service configured (PM2/Docker/Systemd)
- [ ] Logs configured
- [ ] Health check working
- [ ] External access tested

## Troubleshooting

### Issue: Microservices won't compile
```bash
# Check dependencies
sudo dnf install gcc-c++ cmake make

# Clean and recompile
rm -rf build
mkdir build && cd build
cmake .. && make VERBOSE=1
```

### Issue: Port 8080 not accessible
```bash
# Check if listening
sudo netstat -tlnp | grep :8080

# Check firewall
sudo firewall-cmd --list-ports
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload
```

### Issue: PM2 won't start
```bash
# Check logs
pm2 logs api-gateway --err

# Restart PM2
pm2 kill
pm2 resurrect
```

Fedora Server deployment is ready! Choose your preferred method (PM2, Docker, or Systemd) and follow the guide.

````
