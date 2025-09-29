# GitHub Actions Setup Guide - Self-Hosted Runner

This document explains how to configure GitHub Actions with a **self-hosted runner** on your Fedora server.

## Overview

This project uses **self-hosted runners** running directly on your Fedora server, providing:
- Native environment - No containers needed
- Faster builds - Local dependencies and caching
- Resource efficiency - Uses your server directly
- Same environment - Development = CI = Production

## Runner Tags Strategy

### Current Setup:
Your runner should have these tags:
```
self-hosted Linux X64 firmware fedora webserver
```

### Workflow Targeting:
- **Firmware builds**: `[self-hosted, Linux, X64, firmware, fedora]`
- **Webserver builds**: `[self-hosted, Linux, X64, webserver, fedora]`

This allows both projects to use the same runner but target specific tags.

## Required Configuration

### 1. Add Webserver Tag to Your Runner

If you need to add the `webserver` tag to your existing runner:

#### Option A: Re-configure runner
```bash
# Navigate to your runner directory
cd /path/to/your/runner

# Remove current configuration
sudo ./svc.sh stop
sudo ./svc.sh uninstall
./config.sh remove --token YOUR_REMOVAL_TOKEN

# Re-configure with additional tag
./config.sh --url https://github.com/rdmeneze/rdws_webserver --token YOUR_NEW_TOKEN --labels self-hosted,Linux,X64,firmware,fedora,webserver

# Reinstall service
sudo ./svc.sh install
sudo ./svc.sh start
```

#### Option B: Edit runner configuration
```bash
# Navigate to runner directory
cd /path/to/your/runner

# Stop service
sudo ./svc.sh stop

# Edit .runner file and add webserver to labels
# Restart service
sudo ./svc.sh start
```

### 2. Server Prerequisites

Ensure your Fedora server has all dependencies:
```bash
# Development tools (probably already installed)
sudo dnf groupinstall -y "Development Tools" "Development Libraries"

# Webserver specific dependencies
sudo dnf install -y cmake gcc-c++ make git curl
sudo dnf install -y curl-devel rapidjson-devel gtest-devel gmock-devel

# Pistache (if not already installed)
sudo dnf install -y pistache-devel

# Optional: Code quality tools
sudo dnf install -y clang-tools-extra  # for clang-format
```

## 🔄 Workflow Behaviors

### CI Workflow (`ci.yml`)
**Triggers:** Push/PR to main/develop
**Runner:** `[self-hosted, Linux, X64, webserver, fedora]`
**Actions:**
- ✅ Builds natively on your Fedora server
- ✅ Runs unit and integration tests
- ✅ Code quality checks (if clang-format available)
- ✅ Uploads artifacts

### Deploy Workflow (`deploy.yml`)
**Triggers:** Push to main or manual
**Runner:** `[self-hosted, Linux, X64, webserver, fedora]`
**Actions:**
- ✅ Builds project locally
- ✅ Runs tests locally
- ✅ Deploys locally (no SSH needed!)
- ✅ Restarts server locally
- ✅ Health checks

### Release Workflow (`release.yml`)
**Triggers:** Version tags or manual
**Runner:** `[self-hosted, Linux, X64, webserver, fedora]`
**Actions:**
- ✅ Creates release builds
- ✅ Packages with installation scripts
- ✅ Creates GitHub releases
- ✅ Optionally installs locally

## 🎯 How to Use

### Running CI
Push or create PR - CI runs automatically on your server.

### Deploying
**Automatic:** Push to main → Builds and deploys locally
**Manual:** Actions → Deploy to Server → Run workflow

### Creating Releases
```bash
git tag v1.0.0
git push origin v1.0.0
```

## 💡 Advantages of Self-Hosted

### vs. GitHub Hosted Runners:
✅ **No dependency installation** - Everything already on server
✅ **Faster builds** - No container startup time
✅ **Local deployment** - No SSH/network overhead
✅ **Resource control** - Use your server's full power
✅ **Persistent state** - Cache between builds
✅ **Same environment** - Dev = CI = Prod

### Concurrent Job Handling:
- **Sequential execution** - One job at a time (default)
- **Isolated workspaces** - Each repo gets separate `_work` directory
- **Resource sharing** - Firmware and webserver builds queue up

## 🔍 Monitoring

### Check Runner Status:
```bash
# On your server, in runner directory
./run.sh --once  # Test run
sudo systemctl status actions.runner.*  # If installed as service
```

### View Running Jobs:
- GitHub repo → Actions tab
- See which jobs are running/queued

### Logs Location:
```bash
# Runner logs
tail -f _diag/Runner_*.log

# Job logs
ls -la _work/
```

## 🛠️ Troubleshooting

### Runner Not Picking Up Jobs
```bash
# Check runner status
./run.sh --check

# Check network connectivity
curl -I https://api.github.com

# Check labels match
cat .runner  # Should include webserver tag
```

### Build Failures
```bash
# Check dependencies
which cmake g++ pkg-config
pkg-config --exists libpistache

# Check workspace
ls -la _work/rdws_webserver/
```

### Conflicts with Firmware Builds
- Jobs run sequentially by default
- Check `_work/` directory for multiple workspaces
- Monitor with `ps aux | grep cmake`

## 🔄 Job Queue Management

When both firmware and webserver jobs trigger:
1. **First job runs** (e.g., firmware)
2. **Second job queues** (e.g., webserver)
3. **Automatic execution** when first completes
4. **Clean isolation** - separate workspaces

## 🚀 Next Steps

1. ✅ **Add webserver tag** to existing runner
2. ✅ **Test CI** by pushing to repo
3. ✅ **Test deploy** by merging to main
4. ✅ **Create first release** with version tag
5. ✅ **Monitor performance** and adjust as needed

## 📚 Benefits Summary

**Simplified Setup** - No SSH keys or remote connections needed
⚡ **Faster Execution** - Native builds without containers
🔒 **Better Security** - No external SSH access required
📈 **Resource Efficiency** - Direct use of server resources
**Consistent Environment** - Same setup from dev to prod

Your GitHub Actions now run directly on your server with the same environment as your development setup! 🚀

## Workflow Triggers

### CI Workflow (`ci.yml`)
**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` branch

**Actions:**
- Builds project in Fedora container
- Runs unit and integration tests
- Performs security scan with CodeQL
- Checks code formatting
- Uploads test results and build artifacts

### Deploy Workflow (`deploy.yml`)
**Triggers:**
- Push to `main` branch (automatic)
- Manual trigger via GitHub Actions UI

**Actions:**
- Deploys code to server via rsync
- Builds project on server
- Runs tests on server
- Restarts server
- Performs health check
- Notifies deployment status

### Release Workflow (`release.yml`)
**Triggers:**
- Push of version tags (e.g., `v1.0.0`, `v2.1.0-beta`)
- Manual trigger via GitHub Actions UI

**Actions:**
- Builds release version
- Creates release package with binaries
- Generates changelog from git commits
- Creates GitHub release with assets
- Optionally deploys to production (stable releases only)

## 🏃 How to Use

### Running CI
CI runs automatically on pushes and PRs. No action needed.

### Deploying to Server
**Automatic:** Push to `main` branch
```bash
git push origin main
```

**Manual:** Go to Actions → Deploy to Server → Run workflow

### Creating Releases

**Method 1: Tag-based (Recommended)**
```bash
# Create and push a version tag
git tag v1.0.0
git push origin v1.0.0
```

**Method 2: Manual**
1. Go to Actions → Release → Run workflow
2. Enter version (e.g., `v1.0.0`)
3. Choose if pre-release or not

### Version Naming Convention
- `v1.0.0` - Stable release
- `v1.0.0-beta` - Pre-release (beta)
- `v1.0.0-alpha` - Pre-release (alpha)
- `v1.0.0-rc.1` - Release candidate

## 📦 Release Packages

Each release creates a package containing:
- `rest_server` - Main server binary
- `scripts/` - Management scripts
- `install.sh` - Installation script
- `start_server.sh` - Quick start script
- `cpp-rest-server.service` - Systemd service file
- `README.md` & `DEVELOPMENT.md` - Documentation

### Installing a Release
```bash
# Download from GitHub Releases
wget https://github.com/rdmeneze/rdws_webserver/releases/latest/download/cpp-rest-server-1.0.0.tar.gz

# Extract and install
tar xzf cpp-rest-server-1.0.0.tar.gz
cd cpp-rest-server-1.0.0
./install.sh

# Or with systemd service
./install.sh --systemd
sudo systemctl start cpp-rest-server
```

## 🔍 Monitoring

### Check Workflow Status
- Go to Actions tab in GitHub
- View logs of each workflow run
- Download artifacts (test results, binaries)

### View Deployment Status
- Check deploy workflow logs
- Monitor server status via SSH
- Check health endpoint: `curl http://your-server:9080/hello`

## 🛠️ Troubleshooting

### Common Issues

#### SSH Connection Failed
- Check `DEPLOY_SSH_KEY` secret is correct
- Verify SSH key is added to server
- Check `SERVER_HOST` and `SERVER_USER` secrets

#### Build Failed
- Check Fedora container compatibility
- Verify all dependencies are installed
- Review CMake configuration

#### Deploy Failed
- Check server disk space
- Verify server is accessible
- Check script permissions

#### Tests Failed
- Review test logs in workflow
- Check if server environment matches CI
- Verify test dependencies

### Debug Tips

#### Enable SSH Debug
Temporarily add to deploy workflow:
```yaml
- name: Debug SSH
  run: ssh -v ${{ secrets.SERVER_USER }}@${{ secrets.SERVER_HOST }} "echo 'Debug connection'"
```

#### View Server Logs
```bash
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/rdws_webserver && tail -f server.log"
```

#### Manual Deploy Test
```bash
# Test rsync manually
rsync -avz --exclude-from=.deployignore ./ rdias@fedora-server.local:/home/rdias/sources/lab/rdws_webserver/
```

## 🔒 Security Notes

- SSH keys are stored as encrypted secrets
- Only authorized contributors can trigger deploys
- Production environment can have additional protection rules
- CodeQL security scanning runs on every build
- Dependencies are cached but verified each run

## 🚀 Next Steps

1. **Set up secrets** in GitHub repository settings
2. **Test CI** by creating a pull request
3. **Test deploy** by pushing to main branch
4. **Create first release** by tagging a version
5. **Monitor workflows** in GitHub Actions tab

## 📚 Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [GitHub Environments](https://docs.github.com/en/actions/deployment/targeting-different-environments)
- [SSH Agent Action](https://github.com/webfactory/ssh-agent)
- [CodeQL Analysis](https://docs.github.com/en/code-security/code-scanning/automatically-scanning-your-code-for-vulnerabilities-and-errors)
