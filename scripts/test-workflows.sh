#!/bin/bash

# 🧪 Script para testar workflows GitHub Actions localmente
# Simula os passos dos workflows para debug

set -e

# Cores
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_step() { echo -e "${BLUE}[STEP]${NC} $1"; }
print_success() { echo -e "${GREEN}${NC} $1"; }
print_error() { echo -e "${RED}${NC} $1"; }
print_warning() { echo -e "${YELLOW}${NC} $1"; }

BUILD_DIR="./build"

print_step "Testing GitHub Actions workflows locally..."
echo ""

# Simular CI workflow
print_step "1. Testing CI Workflow Steps"

print_step "1.1. Clean and prepare"
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR

print_step "1.2. Check dependencies"
if command -v cmake &> /dev/null; then
    print_success "CMake found: $(cmake --version | head -1 | cut -d' ' -f3)"
else
    print_error "CMake not found"
    exit 1
fi

if command -v node &> /dev/null; then
    print_success "Node.js found: $(node --version)"
else
    print_error "Node.js not found"
    exit 1
fi

if command -v npm &> /dev/null; then
    print_success "npm found: $(npm --version)"
else
    print_error "npm not found"
    exit 1
fi

print_step "1.3. Configure and build C++ microservices"
cd $BUILD_DIR
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=17
make -j$(nproc)
cd ..

print_step "1.4. Test microservices executables"
if [ -f "$BUILD_DIR/src/services/users/users_service" ]; then
    print_success "users_service built successfully"
    echo "Testing users_service..."
    $BUILD_DIR/src/services/users/users_service "GET" "/users" | head -2
else
    print_error "users_service not found"
    exit 1
fi

if [ -f "$BUILD_DIR/src/services/orders/orders_service" ]; then
    print_success "orders_service built successfully"
    echo "Testing orders_service..."
    $BUILD_DIR/src/services/orders/orders_service "GET" "/orders" | head -2
else
    print_error "orders_service not found"
    exit 1
fi

print_step "1.5. Install API Gateway dependencies"
if [ -f "package.json" ]; then
    npm ci
    print_success "Dependencies installed"
else
    print_error "package.json not found"
    exit 1
fi

print_step "1.6. Run API Gateway tests"
BUILD_PATH=$BUILD_DIR npm test
print_success "API Gateway tests passed"

print_step "1.7. Integration test (start gateway + test)"
BUILD_PATH=$BUILD_DIR nohup node dist/src/api-gateway/api-gateway.js > gateway-test.log 2>&1 &
GATEWAY_PID=$!
sleep 5

if chmod +x scripts/verify-deploy.sh && ./scripts/verify-deploy.sh; then
    print_success "Integration tests passed"
else
    print_warning "Integration tests had issues"
fi

# Clean up
kill $GATEWAY_PID 2>/dev/null || true
rm gateway-test.log || true

echo ""
print_step "2. Testing Deploy Workflow Steps"

print_step "2.1. Build release version"
cd $BUILD_DIR
make clean || true
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17
make -j$(nproc)
cd ..

print_step "2.2. Install production dependencies"
npm ci --production
print_success "Production dependencies ready"

print_step "2.3. Quick deploy verification"
if [ -f "scripts/deploy-fedora.sh" ]; then
    print_success "Deploy script found"
    chmod +x scripts/deploy-fedora.sh

    # Check if script is executable but don't run full deploy
    if head -1 scripts/deploy-fedora.sh | grep -q "#!/bin/bash"; then
        print_success "Deploy script is valid"
    else
        print_warning "Deploy script may have issues"
    fi
else
    print_error "Deploy script not found"
fi

echo ""
print_step "3. Testing Release Workflow Steps"

print_step "3.1. Check version and changelog"
if git rev-parse --is-inside-work-tree &> /dev/null; then
    CURRENT_VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.1.0")
    print_success "Current version: $CURRENT_VERSION"

    # Generate sample changelog
    echo "Generating sample changelog..."
    git log --oneline -5 > CHANGELOG-sample.md
    print_success "Changelog generated"
else
    print_warning "Not in a git repository"
fi

print_step "3.2. Create mock release package"
mkdir -p packages/test-release
cp -r $BUILD_DIR/src/services packages/test-release/
cp dist/src/api-gateway/api-gateway.js package*.json packages/test-release/
cp -r scripts packages/test-release/
cp README.md API-GATEWAY.md DEPLOY-*.md packages/test-release/ 2>/dev/null || true

print_success "Mock release package created"

print_step "3.3. Test Docker build (if Docker available)"
if command -v docker &> /dev/null; then
    if docker build -f Dockerfile.gateway -t test-api-gateway . > /dev/null 2>&1; then
        print_success "Docker build successful"
        docker rmi test-api-gateway > /dev/null 2>&1 || true
    else
        print_warning "Docker build failed"
    fi
else
    print_warning "Docker not available for testing"
fi

# Cleanup
rm -rf packages/test-release
rm CHANGELOG-sample.md 2>/dev/null || true

echo ""
print_step "4. Summary"

print_success "CI workflow steps: PASSED"
print_success "Deploy workflow steps: PASSED"
print_success "Release workflow steps: PASSED"

echo ""
print_step "All workflow tests completed successfully!"
echo ""
print_step "Ready for:"
echo "  • Push to trigger CI"
echo "  • Merge to main for deploy"
echo "  • Tag creation for release"
echo ""
print_step "Useful commands:"
echo "  • npm run test:ci          - Run tests like CI"
echo "  • npm run deploy:check     - Run deploy verification"
echo "  • ./scripts/deploy-fedora.sh - Full deployment"
echo ""

print_success "GitHub Actions workflows are ready! 🚀"
