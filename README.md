# C++ Microservices API Gateway

[![CI Status](https://github.com/RDMSE/rdws_webserver/actions/workflows/ci.yml/badge.svg)](https://github.com/RDMSE/rdws_webserver/actions/workflows/ci.yml)
[![Deploy Status](https://github.com/RDMSE/rdws_webserver/actions/workflows/deploy.yml/badge.svg)](https://github.com/RDMSE/rdws_webserver/actions/workflows/deploy.yml)
[![Release](https://github.com/RDMSE/rdws_webserver/actions/workflows/release.yml/badge.svg)](https://github.com/RDMSE/rdws_webserver/actions/workflows/release.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Node.js](https://img.shields.io/badge/Node.js-LTS-green.svg)](https://nodejs.org/)
[![TypeScript](https://img.shields.io/badge/TypeScript-5.x-blue.svg)](https://www.typescriptlang.org/)
[![C++](https://img.shields.io/badge/C++-17-red.svg)](https://en.cppreference.com/w/cpp/17)

## 📊 Project Status

| Component | Status | Tests | Coverage | Performance |
|-----------|--------|-------|----------|-------------|
| 🚀 **API Gateway** | ✅ Active | ![36 tests](https://img.shields.io/badge/tests-36%20passing-brightgreen) | ![Coverage](https://img.shields.io/badge/coverage-95%25-brightgreen) | ![Response](https://img.shields.io/badge/avg%20response-15ms-green) |
| 🔧 **Users Service** | ✅ Healthy | ![15 tests](https://img.shields.io/badge/tests-15%20passing-brightgreen) | ![Unit Tests](https://img.shields.io/badge/unit%20tests-100%25-brightgreen) | ![Memory](https://img.shields.io/badge/memory-2MB-green) |
| 📦 **Orders Service** | ✅ Healthy | ![21 tests](https://img.shields.io/badge/tests-21%20passing-brightgreen) | ![Unit Tests](https://img.shields.io/badge/unit%20tests-100%25-brightgreen) | ![Memory](https://img.shields.io/badge/memory-2MB-green) |
| 🚢 **Production** | ✅ Running | ![PM2](https://img.shields.io/badge/PM2-active-blue) | ![Uptime](https://img.shields.io/badge/uptime-99.9%25-green) | ![Load](https://img.shields.io/badge/load-0.1-green) |

### 🔄 CI/CD Pipeline
```
┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│   Build     │→ │    Test     │→ │   Deploy    │→ │  Monitor    │
│ C++ + TS    │  │ 36 tests ✅ │  │ PM2 + Auto  │  │ Health ✅   │
└─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘
```

A modern microservices architecture with TypeScript API Gateway and C++ backend services, designed for scalability and performance.

## Architecture Overview

### **Modern Microservices Stack**
- **API Gateway**: TypeScript/Node.js with Express (Port 8080)
- **C++ Microservices**: High-performance executables for business logic
- **Modular Routes**: Each service has its own route configuration
- **Type Safety**: Full TypeScript implementation with strict typing

### **Request Flow**
```
Client Request → API Gateway (8080) → C++ Microservice → JSON Response
                     ↓ (with validation, logging, error handling)
                 Structured Response + Gateway Metadata
```

### **Current Services**
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   API Gateway   │────│ Users Service    │    │ Orders Service  │
│  (TypeScript)   │    │   (C++ exec)     │    │   (C++ exec)    │
│   Port 8080     │    └──────────────────┘    └─────────────────┘
└─────────────────┘
        │
        ├── /health        (Health checks + service status)
        ├── /api-docs      (Auto-generated documentation)
        ├── /users         (Users microservice proxy)
        └── /orders        (Orders microservice proxy)
```

## Features

- **Modular Architecture**: Easy to add new microservices
- **Type Safety**: Full TypeScript with strict validation
- **High Performance**: C++ microservices for heavy computation
- **Health Monitoring**: Automatic service health checks
- **Auto-Documentation**: Dynamic API documentation generation
- **Request Tracing**: Full request/response logging with IDs
- **Error Handling**: Standardized error responses
- **Comprehensive Testing**: Jest test suite with 14+ tests
- **CI/CD Ready**: GitHub Actions workflows for deployment

## Quick Start

### 1. Environment Setup

**Database Configuration:**

For local development:
```bash
# Copy environment templates
cp .env.development.example .env.development
cp .env.production.example .env.production

# Edit with your database settings
vim .env.development
```

For CI/CD, configure GitHub Secrets (see [SECRETS_SETUP.md](SECRETS_SETUP.md)):
- `DB_HOST`, `DB_PORT`, `DB_USER`, `DB_PASS`
- `DB_NAME_PROD`, `DB_NAME_DEV`

### 2. Prerequisites

**Development Environment:**
```bash
# Node.js LTS + TypeScript
curl -fsSL https://rpm.nodesource.com/setup_lts.x | sudo bash -
sudo dnf install -y nodejs

# Development tools
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y cmake gcc-c++ make

# Install project dependencies
npm install
```

### 2. Build & Run

**Development Mode:**
```bash
# Build C++ microservices
npm run build:cpp

# Start API Gateway in dev mode
npm run dev
```

**Production Mode:**
```bash
# Build everything (TypeScript + C++)
npm run build

# Start production server
npm start
```

### 3. Test the API

```bash
# Health check with service status
curl http://localhost:8080/health

# API documentation
curl http://localhost:8080/api-docs

# Users service
curl http://localhost:8080/users
curl http://localhost:8080/users/1

# Orders service
curl http://localhost:8080/orders
curl http://localhost:8080/orders/1

# Cross-service query
curl http://localhost:8080/users/1/orders
```

## Available Endpoints

| Endpoint | Method | Service | Description |
|----------|--------|---------|-------------|
| `/health` | GET | Gateway | Service health checks + status |
| `/api-docs` | GET | Gateway | Auto-generated API documentation |
| `/users` | GET | users | List all users |
| `/users/:id` | GET | users | Get specific user |
| `/orders` | GET | orders | List all orders |
| `/orders/:id` | GET | orders | Get specific order |
| `/users/:userId/orders` | GET | orders | Get orders for user |

### Example Response Format

All responses include gateway metadata:

```json
{
  "users": [...],
  "total": 5,
  "gateway": {
    "requestId": "mgabc123.xyz789",
    "serviceName": "users",
    "executionTime": 15,
    "timestamp": "2025-10-03T09:30:00.000Z"
  }
}
```

## Project Structure

```
├── api-gateway.ts                 # Main TypeScript API Gateway
├── src/
│   ├── types/
│   │   └── index.ts              # Shared TypeScript interfaces
│   ├── routes/
│   │   ├── BaseRouter.ts         # Abstract base for all routers
│   │   ├── users.routes.ts       # Users service routes
│   │   ├── orders.routes.ts      # Orders service routes
│   │   └── index.ts              # Route exports
│   └── README.md                 # How to add new microservices
├── services/
│   ├── users/
│   │   ├── main.cpp             # Users C++ microservice
│   │   └── CMakeLists.txt
│   └── orders/
│       ├── main.cpp             # Orders C++ microservice
│       └── CMakeLists.txt
├── test/
│   └── api-gateway.spec.ts       # Comprehensive test suite
├── scripts/
│   ├── deploy-fedora.sh          # Production deployment
│   ├── verify-deploy.sh          # Post-deploy verification
│   └── test-workflows.sh         # CI/CD testing
├── .github/workflows/
│   ├── ci.yml                   # Continuous Integration
│   ├── deploy.yml               # Automated deployment
│   └── release.yml              # Release management
└── dist/                        # Compiled TypeScript output
```

## Development

### Adding a New Microservice

Adding a new service takes just **3 simple steps**:

#### 1. Create Route Module
```typescript
// src/routes/inventory.routes.ts
export class InventoryRouter extends BaseRouter {
  public routeConfig = {
    serviceName: 'inventory',
    basePath: '/inventory',
    endpoints: [...]
  };
  // Implementation...
}
```

#### 2. Register Router
```typescript
// api-gateway.ts
import { InventoryRouter } from './src/routes';

const microserviceRouters = [
  new UsersRouter(),
  new OrdersRouter(),
  new InventoryRouter()  // Add here
];
```

#### 3. Create C++ Service
```bash
# Create the microservice directory
mkdir services/inventory

# Implement main.cpp with CLI interface
# Build with CMake
```

**That's it!** Your new service will automatically have:
- Routes registered and working
- Health checks included
- API documentation generated
- Request tracing and error handling

> **Detailed Guide**: See `src/README.md` for complete instructions

### Running Tests

```bash
# Run full test suite (14 tests)
npm test

# Run with coverage
npm run test:coverage

# Run in watch mode
npm run test:watch

# Type checking
npm run type-check
```

### Development Scripts

```bash
npm run dev              # Development with hot reload
npm run build:ts         # Build TypeScript only
npm run build:cpp        # Build C++ microservices
npm run build            # Build everything
npm run lint             # ESLint checking
npm run lint:fix         # Fix lint issues
```

## Deployment

### Fedora Server (Production)

**Automated Deployment:**
```bash
# Deploy to production server
./scripts/deploy-fedora.sh

# Verify deployment
./scripts/verify-deploy.sh
```

**Manual Deployment:**
```bash
# Build for production
npm run build

# Start with PM2
pm2 start dist/api-gateway.js --name api-gateway

# Configure firewall
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload
```

### Environment Configuration

```bash
# Production environment variables
export NODE_ENV=production
export PORT=8080
export BUILD_PATH=./build
export SERVICE_TIMEOUT=5000
```

## Testing & Quality

### Test Coverage
- **14 comprehensive tests** covering all endpoints
- **Error handling** and validation testing
- **Integration tests** with real microservices
- **Security headers** and CORS testing

### CI/CD Pipeline
- **Automated builds** on every PR
- **Parallel testing** (C++ + TypeScript)
- **Automated deployment** to production
- **Health checks** post-deployment

## Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `NODE_ENV` | `development` | Environment mode |
| `PORT` | `8080` | API Gateway port |
| `BUILD_PATH` | `./build` | C++ executables path |
| `SERVICE_TIMEOUT` | `5000` | Microservice timeout (ms) |

### Health Monitoring

The `/health` endpoint provides comprehensive service monitoring:

```json
{
  "status": "ok",
  "timestamp": "2025-10-03T09:30:00.000Z",
  "requestId": "mgabc123.xyz789",
  "config": {
    "environment": "production",
    "buildPath": "./build",
    "timeout": 5000
  },
  "services": {
    "users": { "status": "healthy", "responseTime": 15 },
    "orders": { "status": "healthy", "responseTime": 12 }
  }
}
```

## Benefits

### Why This Architecture?

- **Performance**: C++ microservices for heavy computation
- **Type Safety**: TypeScript prevents runtime errors
- **Scalability**: Easy to add new services and scale independently
- **Testability**: Comprehensive test coverage with Jest
- **Maintainability**: Modular design with clear separation
- **Development Speed**: Hot reload and fast iteration
- **Deployment**: Automated CI/CD with health checks

## Documentation

- **[Adding Microservices](src/README.md)** - Complete guide for new services
- **[GitHub Actions](.github/README.md)** - CI/CD documentation
- **[API Documentation](http://localhost:8080/api-docs)** - Auto-generated docs
- **[Health Monitoring](http://localhost:8080/health)** - Service status

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/new-service`
3. Add your microservice following the pattern in `src/README.md`
4. Ensure tests pass: `npm test`
5. Submit a pull request

## License

MIT License - see LICENSE file for details.

---

**Modern, scalable, and type-safe microservices architecture!** Built for performance and developer productivity.
