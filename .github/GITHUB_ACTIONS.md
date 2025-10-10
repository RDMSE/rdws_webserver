# GitHub Actions Setup Guide - Self-Hosted Runner

This document explains how to configure GitHub Actions with a **self-hosted runner** on your Fedora server.

## Overview

This project uses **self-hosted runners** running directly on your Fedora server, providing:
- Native environment - No containers needed
- Faster builds - Local dependencies and caching
- Resource efficiency - Uses your server directly
- Same environment - Development = CI = Production
- Sequential execution - Jobs run one at a time on the same runner

## Current Runner Configuration

### Runner Tags:
```
self-hosted Linux X64 webserver fedora
```

### Workflow Targeting:
All workflows target: `[self-hosted, Linux, X64, webserver, fedora]`

## Required Configuration

### 1. Configure Runner with Webserver Tag

Ensure your runner has the `webserver` tag:

```bash
# Configure runner with required tags
./config.sh --url https://github.com/RDMSE/rdws_webserver --token YOUR_TOKEN --labels self-hosted,Linux,X64,webserver,fedora

# Install as service
sudo ./svc.sh install
sudo ./svc.sh start
```

### 2. Server Prerequisites

Ensure your Fedora server has all dependencies:
```bash
# Development tools (auto-installed by workflows if missing)
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y cmake gcc-c++ make git curl

# Testing framework
sudo dnf install -y gtest-devel gmock-devel

# Node.js (auto-installed by workflows if missing)
sudo dnf install -y nodejs npm

# PM2 for production deployment
sudo npm install -g pm2

# Firewall configuration
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload
```

## Workflow Behaviors

### CI Workflow (`ci.yml`)
**Triggers:** Push/PR to main/develop
**Runner:** `[self-hosted, Linux, X64, webserver, fedora]`
**Jobs:**
1. **Build C++ Microservices** - Compiles users_service and orders_service
2. **Test API Gateway** - Tests Node.js/TypeScript API Gateway with mocked microservices
3. **Integration Tests** - End-to-end testing with real microservices
4. **CI Summary** - Reports overall status

**Actions:**
- Auto-installs dependencies if missing (CMake, Node.js, Google Test)
- Builds natively on your Fedora server
- Runs comprehensive unit and integration tests
- Uploads artifacts (binaries, test results)
- Uses dynamic port allocation for testing

### Deploy Workflow (`deploy.yml`)
**Triggers:** Push to main or manual dispatch
**Runner:** `[self-hosted, Linux, X64, webserver, fedora]`
**Jobs:**
1. **Build** - Builds everything for production
2. **Deploy** - Deploys to `/opt/rdws_webserver`
3. **Post-Deploy Tests** - Validates deployment

**Actions:**
- Builds with Release configuration
- Deploys directly to `/opt/rdws_webserver`
- Uses PM2 for process management
- Configures firewall (port 8080)
- Performs health checks and load testing
- No SSH needed - runs locally on server

### Release Workflow (`release.yml`)
**Triggers:** Version tags (v*) or manual dispatch
**Runner:** `[self-hosted, Linux, X64, webserver, fedora]`
**Actions:**
- Creates release builds
- Generates changelog from git history
- Packages with installation scripts
- Creates Docker images
- Creates GitHub releases with assets
- Uploads .tar.gz, .zip, and Docker image

## How to Use

### Running CI
**Automatic:** Push or create PR - CI runs automatically
```bash
git push origin develop  # Triggers CI
# or
git push origin main     # Triggers CI + Deploy
```

### Deploying
**Automatic:** Push to main → Builds and deploys to `/opt/rdws_webserver`
```bash
git push origin main
```

**Manual:** Actions → Deploy API Gateway + Microservices → Run workflow
- Can choose environment (production/staging)
- Deploys using PM2 with health checks

### Creating Releases
**Method 1: Tag-based (Recommended)**
```bash
git tag v1.0.0
git push origin v1.0.0
```

**Method 2: Manual**
- Go to Actions → Release Microservices API Gateway
- Enter version and prerelease status
- Creates packages with installation scripts

## Advantages of Self-Hosted

### vs. GitHub Hosted Runners:
**No dependency installation** - Everything already on server
**Faster builds** - No container startup time
**Local deployment** - No SSH/network overhead
**Resource control** - Use your server's full power
**Persistent state** - Cache between builds
**Same environment** - Dev = CI = Prod

### Concurrent Job Handling:
- **Sequential execution** - One job at a time (default)
- **Isolated workspaces** - Each repo gets separate `_work` directory
- **Resource sharing** - Firmware and webserver builds queue up

## Monitoring

### Check Runner Status:
```bash
# On your server, in runner directory
./run.sh --once  # Test run
sudo systemctl status actions.runner.*  # If installed as service
journalctl -u actions.runner.* -f  # Follow logs
```

### View Running Jobs:
- GitHub repo → Actions tab
- See which jobs are running/queued
- Download artifacts (test results, binaries)

### Check Deployment Status:
```bash
# PM2 status
pm2 status
pm2 logs api-gateway
pm2 monit

# Service health
curl http://localhost:8080/health
curl http://localhost:8080/users

# Production directory
ls -la /opt/rdws_webserver
```

### Logs Location:
```bash
# Runner logs
tail -f _diag/Runner_*.log

# Job logs
ls -la _work/

# PM2 logs
pm2 logs --lines 100
```

## Troubleshooting

## Troubleshooting

### Runner Not Picking Up Jobs
```bash
# Check runner status
./run.sh --check

# Check network connectivity
curl -I https://api.github.com

# Check labels match
cat .runner  # Should include webserver tag

# Restart runner service
sudo ./svc.sh stop
sudo ./svc.sh start
```

### Build Failures
```bash
# Check dependencies
which cmake g++ node npm
pkg-config --exists gtest

# Check workspace
ls -la _work/rdws_webserver/

# View specific job logs in GitHub Actions tab
```

### Deploy Failures
```bash
# Check PM2 status
pm2 status
pm2 logs api-gateway --err

# Check port availability
sudo netstat -tlnp | grep :8080

# Check production directory
ls -la /opt/rdws_webserver
sudo chown -R $USER:$USER /opt/rdws_webserver

# Manual health check
curl -v http://localhost:8080/health
```

### TypeScript/Node.js Issues
```bash
# Check TypeScript compilation
cd /opt/rdws_webserver
npx tsc src/api-gateway/api-gateway.ts --target es2020 --module commonjs

# Test direct execution
node --require ts-node/register src/api-gateway/api-gateway.ts

# Check dependencies
npm list ts-node typescript
```

## Job Queue Management

Since this uses a self-hosted runner:
1. **Sequential execution** - Jobs run one at a time
2. **Automatic queuing** - Multiple triggers queue up
3. **Clean isolation** - Each job gets separate workspace
4. **Resource sharing** - All workflows share the same runner

## Project Structure

### Artifacts Created:
- **C++ microservices** (`users_service`, `orders_service`)
- **Test results** (XML format, uploaded to GitHub)
- **Release packages** (.tar.gz, .zip, Docker images)
- **PM2 deployment** (production-ready process management)

### Deployment Structure:
```
/opt/rdws_webserver/
├── src/
│   ├── api-gateway/
│   │   └── api-gateway.ts      # Main API Gateway
│   ├── types/
├── build/                  # C++ binaries
│   └── services/
│       ├── users/users_service
│       └── orders/orders_service
├── scripts/                # Management scripts
├── package.json           # Node.js dependencies
└── *.md                   # Documentation
```

## Next Steps

1. **Verify runner configuration** with webserver tag
2. **Test CI** by creating a pull request
3. **Test deploy** by pushing to main branch
4. **Create first release** by tagging a version
5. **Monitor workflows** in GitHub Actions tab
6. **Set up PM2 monitoring** for production

## Benefits Summary

✅ **Simplified Setup** - No SSH keys or remote connections needed
✅ **Faster Execution** - Native builds without containers
✅ **Better Security** - No external SSH access required
✅ **Resource Efficiency** - Direct use of server resources
✅ **Consistent Environment** - Same setup from dev to prod
✅ **Comprehensive Testing** - Unit, integration, and load tests
✅ **Production Ready** - PM2 deployment with health checks

Your GitHub Actions now run directly on your server with complete CI/CD pipeline!

## Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Self-hosted Runners](https://docs.github.com/en/actions/hosting-your-own-runners)
- [PM2 Documentation](https://pm2.keymetrics.io/docs/)
- [TypeScript Node.js Guide](https://nodejs.org/en/docs/guides/nodejs-typescript/)

---

**Note:** This configuration is optimized for the RDWS microservices project with TypeScript API Gateway and C++ microservices running on a self-hosted Fedora server.
