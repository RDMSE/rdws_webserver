````markdown
# Quick Fedora Server Deployment

This guide presents the fastest way to deploy the API Gateway on Fedora Server.

## 1-Command Deployment

```bash
# Download and execute automated script
curl -fsSL https://raw.githubusercontent.com/RDMSE/rdws_webserver/14-featureserverinfra-enable-servless-archtecture/scripts/deploy-fedora.sh | bash
```

## What the script does automatically:

1. **Checks dependencies** (Node.js, CMake, build tools)
2. **Installs missing dependencies** (if run as root)
3. **Clones/updates repository** to `/opt/rdws_webserver`
4. **Compiles C++ microservices**
5. **Configures firewall** (port 8080)
6. **Starts service** (PM2, Docker or systemd)
7. **Verifies service health**

## Manual Step-by-Step Deployment

If you prefer to do it manually:

### 1. Prepare System
```bash
# Update system
sudo dnf update -y

# Install dependencies
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y cmake gcc-c++ git nodejs npm

# Install PM2 (recommended)
sudo npm install -g pm2
```

### 2. Deploy
```bash
# Clone project
sudo git clone https://github.com/RDMSE/rdws_webserver.git /opt/rdws_webserver
sudo chown -R $USER:$USER /opt/rdws_webserver
cd /opt/rdws_webserver

# Checkout branch
git checkout 14-featureserverinfra-enable-servless-archtecture

# Install Node.js dependencies
npm install --production

# Compile microservices
mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ..

# Configure firewall
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload

# Start with PM2
pm2 start api-gateway.js --name api-gateway
pm2 save && pm2 startup
```

### 3. Verify Deployment
```bash
# Run complete verification
./scripts/verify-deploy.sh

# Or manual test
curl http://localhost:8080/health
```

## Access from Other Machines

```bash
# Replace YOUR_IP with server IP
curl http://YOUR_IP:8080/health
curl http://YOUR_IP:8080/users
curl http://YOUR_IP:8080/orders
```

## Service Management

### PM2 (Recommended)
```bash
pm2 status                # Status
pm2 logs api-gateway      # Logs
pm2 restart api-gateway   # Restart
pm2 stop api-gateway      # Stop
pm2 monit                 # Monitor
```

### Docker
```bash
docker ps                      # Status
docker logs api-gateway        # Logs
docker restart api-gateway     # Restart
docker stop api-gateway        # Stop
```

### Systemd
```bash
sudo systemctl status api-gateway     # Status
sudo journalctl -u api-gateway -f     # Logs
sudo systemctl restart api-gateway    # Restart
sudo systemctl stop api-gateway       # Stop
```

## Troubleshooting

### Check if running
```bash
# Check processes
ps aux | grep api-gateway

# Check port
sudo netstat -tlnp | grep :8080

# Basic test
curl http://localhost:8080/health
```

### Debug Logs
```bash
# PM2
pm2 logs api-gateway --err

# Docker
docker logs api-gateway

# Systemd
sudo journalctl -u api-gateway --since "5 minutes ago"
```

### Recompile Microservices
```bash
cd /opt/rdws_webserver/build
make clean && make -j$(nproc)

# Restart service after recompilation
pm2 restart api-gateway
```

### Firewall
```bash
# Check if port is open
sudo firewall-cmd --list-ports

# Open port if needed
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload
```

## Available Endpoints

| Endpoint | Description |
|----------|-------------|
| `GET /health` | Service status |
| `GET /users` | List users |
| `GET /users/:id` | User by ID |
| `GET /orders` | List orders |
| `GET /orders/:id` | Order by ID |
| `GET /users/:userId/orders` | Orders for a user |
| `GET /api-docs` | API documentation |

## Updates

To update the code:

```bash
cd /opt/rdws_webserver
git pull origin 14-featureserverinfra-enable-servless-archtecture
npm install --production
cd build && make -j$(nproc) && cd ..
pm2 restart api-gateway
```

## Success Verification

If everything worked correctly, you should see:

```bash
$ curl http://YOUR_IP:8080/health
{
  "status": "ok",
  "timestamp": "2025-10-03T10:00:00.000Z",
  "services": {
    "users": {"status": "healthy", "responseTime": 12},
    "orders": {"status": "healthy", "responseTime": 8}
  }
}
```

## Done!

Your API Gateway is running! Access:
- **API:** http://YOUR_IP:8080
- **Health:** http://YOUR_IP:8080/health
- **Docs:** http://YOUR_IP:8080/api-docs

---

**For detailed support, see: [DEPLOY-FEDORA.md](DEPLOY-FEDORA.md)**

````
