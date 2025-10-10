# GitHub Actions Workflows

This repository uses GitHub Actions for automated CI/CD with three main workflows.

## Available Workflows

### 1. CI - Build and Test (`ci.yml`)

**Trigger:** Push to `main`/`develop` and Pull Requests to `main`/`develop`

**Jobs:**
- **build-cpp-microservices**: Compiles C++ microservices
- **test-api-gateway**: Tests Node.js API Gateway
- **integration-tests**: Complete integration tests
- **ci-summary**: Results summary

**What it does:**
```
Installs dependencies (CMake, Node.js, Google Test)
Compiles C++ microservices (users_service, orders_service)
Tests C++ executables individually
Runs API Gateway tests (Jest)
Starts API Gateway and runs complete verification
Checks test coverage and code quality
```

### 2. Deploy (`deploy.yml`)

**Trigger:** Push to `main` + Manual

**Jobs:**
- **build**: Compiles everything and runs tests
- **deploy**: Production deployment using PM2
- **post-deploy-tests**: Post-deployment verification

**What it does:**
```
Compiles microservices in Release mode
Installs Node.js dependencies
Stops existing services (PM2/Docker/systemd)
Deploys to /opt/rdws_webserver
Configures firewall (port 8080)
Starts API Gateway with PM2
Runs health checks and load tests
```

### 3. Release (`release.yml`)

**Trigger:** Tags `v*` (ex: v1.0.0) + Manual

**What it does:**
```
Compiles release builds
Generates automatic changelog
Creates packages (.tar.gz, .zip)
Builds Docker image
Creates GitHub Release
Uploads all assets
```

## Runner Configuration

The workflows use self-hosted runner on Fedora Server with labels:
- `self-hosted`
- `Linux`
- `X64`
- `webserver`
- `fedora`

### Auto-installed Dependencies:
- Node.js LTS (via NodeSource)
- CMake + build tools
- Google Test + GMock
- PM2 (for production)

## 📊 Workflow Status

### CI Status
- C++ microservices compiling
- API Gateway tested with Jest
- Integration tests passing
- Health checks working

### Deploy Status
- Automatic deployment working
- PM2 managing the service
- Firewall configured
- Post-deploy health checks

## How to Use

### Development (CI)
```bash
# For features: Create PR to develop
git checkout -b feature/my-new-feature
git push origin feature/my-new-feature
# Create PR to develop - CI runs automatically

# For releases: Create PR from develop to main
git checkout develop
git push origin develop
# Create PR to main - CI runs and deploy follows after merge
```

### Manual Deploy
```bash
# Via GitHub Actions
gh workflow run deploy.yml
```

### Create Release
```bash
# Create tag and push
git tag v1.0.0
git push origin v1.0.0

# Or use GitHub Interface
```

## Logs and Debugging

### View Workflow Logs
```bash
# Via GitHub CLI
gh run list
gh run view [run-id]
gh run view [run-id] --log
```

### Server Logs (Production)
```bash
# PM2 logs
pm2 logs api-gateway

# Manual verification
./scripts/verify-deploy.sh
```

## Troubleshooting

### CI Failing?

1. **C++ Compilation Error**:
   ```bash
   # Check dependencies on runner
   cmake --version
   g++ --version
   pkg-config --list-all | grep gtest
   ```

2. **Node.js Tests Failing**:
   ```bash
   # Check if microservices were compiled
   ls -la build/src/services/*/

   # Test locally
   npm test
   ```

3. **Integration Tests Failing**:
   ```bash
   # Check if API Gateway starts
   BUILD_PATH=./build node api-gateway.js

   # Test endpoints
   curl http://localhost:8080/health
   ```

### Deploy Failing?

1. **PM2 Issues**:
   ```bash
   # On server
   pm2 status
   pm2 logs api-gateway
   pm2 restart api-gateway
   ```

2. **Firewall Issues**:
   ```bash
   # Check port
   sudo firewall-cmd --list-ports
   sudo netstat -tlnp | grep :8080
   ```

3. **Permission Issues**:
   ```bash
   # Check permissions
   ls -la /opt/rdws_webserver/
   whoami
   groups
   ```

## Future Improvements

- [ ] **Matrix builds** for different Node.js versions
- [ ] **Caching** for npm dependencies and C++ builds
- [ ] **Parallel jobs** to speed up CI
- [ ] **Notifications** for Slack/Discord on failures
- [ ] **Performance benchmarks** automation
- [ ] **Security scanning** with CodeQL
- [ ] **Docker multi-arch** builds

## Contributing

To contribute to the workflows:

1. Test locally first
2. Use small, specific commits
3. Document workflow changes
4. Test in branch before merging

---

**The workflows are optimized for microservices + API Gateway architecture!**
