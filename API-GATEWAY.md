````markdown
# API Gateway - C++ Microservices

A unified gateway to access C++ microservices through a simple and robust HTTP interface.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation and Execution](#installation-and-execution)
- [API Endpoints](#api-endpoints)
- [Usage Examples](#usage-examples)
- [Configuration](#configuration)
- [Docker](#docker)
- [Testing](#testing)
- [Monitoring](#monitoring)

## Overview

The API Gateway provides a unified HTTP interface to access multiple C++ executable microservices. It centralizes:

- **Routing** requests to appropriate microservices
- **Error handling** standardization
- **Logging** and request tracing
- **Parameter validation**
- **Service health monitoring**

### Architecture

```
Client → API Gateway (port 8080) → C++ Executables
                ↓
    ┌─────────────────┬─────────────────┐
    │ users_service   │ orders_service  │
    │ (executable)    │ (executable)    │
    └─────────────────┴─────────────────┘
```

## Features

- **Transparent proxy** for C++ microservices
- **Robust error handling** with appropriate HTTP codes
- **Request ID** for request tracing
- **Automatic health checks** for services
- **Input parameter validation**
- **Configurable timeout** to prevent freezes
- **CORS** and security headers
- **Detailed logging** with timestamps
- **Graceful shutdown** for zero-downtime deployments

## Installation and Execution

### Requirements

- Node.js 18+
- C++ microservices compiled in `./build/src/services/`

### Local Installation

```bash
# 1. Install dependencies
npm install

# 2. Compile C++ microservices
mkdir -p build && cd build
cmake .. && make

# 3. Run API Gateway
npm start

# Or for development (with auto-reload)
npm run dev
```

### Verification

```bash
# Test if it's working
curl http://localhost:8080/health
```

## API Endpoints

### Base URL: `http://localhost:8080`

| Method | Endpoint                | Description                       |
| ------ | ----------------------- | --------------------------------- |
| `GET`  | `/health`               | Service status and gateway health |
| `GET`  | `/api-docs`             | API documentation                 |
| `GET`  | `/users`                | List all users                    |
| `GET`  | `/users/:id`            | Get user by ID                    |
| `GET`  | `/orders`               | List all orders                   |
| `GET`  | `/orders/:id`           | Get order by ID                   |
| `GET`  | `/users/:userId/orders` | Get orders for a user             |

### Responses

All responses include gateway metadata:

```json
{
  "users": [...],
  "total": 5,
  "source": "users_service C++ executable",
  "gateway": {
    "requestId": "abc123",
    "serviceName": "users",
    "executionTime": 45,
    "timestamp": "2025-10-03T10:00:00.000Z"
  }
}
```

### Error Handling

```json
{
  "error": true,
  "message": "Service users timed out after 5000ms",
  "requestId": "abc123",
  "timestamp": "2025-10-03T10:00:00.000Z"
}
```

## Usage Examples

### 1. List Users

```bash
curl -X GET http://localhost:8080/users
```

```json
{
  "users": [
    {
      "id": 1,
      "name": "João Silva",
      "email": "joao@example.com"
    }
  ],
  "total": 1,
  "source": "users_service C++ executable",
  "endpoint": "/users",
  "timestamp": 1759478812,
  "gateway": {
    "requestId": "abc123",
    "serviceName": "users",
    "executionTime": 12,
    "timestamp": "2025-10-03T10:00:00.000Z"
  }
}
```

### 2. Get User by ID

```bash
curl -X GET http://localhost:8080/users/1
```

### 3. Check Service Health

```bash
curl -X GET http://localhost:8080/health
```

```json
{
  "status": "ok",
  "timestamp": "2025-10-03T10:00:00.000Z",
  "requestId": "def456",
  "config": {
    "environment": "production",
    "buildPath": "./build",
    "timeout": 5000
  },
  "services": {
    "users": {
      "status": "healthy",
      "responseTime": 23
    },
    "orders": {
      "status": "healthy",
      "responseTime": 31
    }
  }
}
```

### 4. JavaScript/Frontend

```javascript
// Using fetch
const response = await fetch('http://localhost:8080/users');
const data = await response.json();

if (data.error) {
  console.error('Error:', data.message);
} else {
  console.log('Users:', data.users);
  console.log('Request ID:', data.gateway.requestId);
}
```

### 5. Python

```python
import requests

response = requests.get('http://localhost:8080/orders')
data = response.json()

if data.get('error'):
    print(f"Error: {data['message']}")
else:
    print(f"Orders: {len(data['orders'])}")
```

## Configuration

### Environment Variables

| Variable          | Default       | Description                          |
| ----------------- | ------------- | ------------------------------------ |
| `PORT`            | `8080`        | Server port                          |
| `BUILD_PATH`      | `./build`     | Path to compiled executables         |
| `NODE_ENV`        | `development` | Environment (development/production) |
| `SERVICE_TIMEOUT` | `5000`        | Timeout in ms for microservices      |

### Example

```bash
export PORT=3000
export BUILD_PATH=/app/build
export SERVICE_TIMEOUT=10000
npm start
```

## Docker

### Build Image

```bash
docker build -f Dockerfile.gateway -t api-gateway .
```

### Run Container

```bash
docker run -d \
  --name api-gateway \
  -p 8080:8080 \
  -v $(pwd)/build:/app/build:ro \
  api-gateway
```

### Docker Compose

```bash
# Run gateway only
docker-compose up api-gateway

# Run with optional serverless functions
docker-compose --profile serverless up
```

### Health Check

The container includes automatic health check:

```bash
docker ps  # View health status
docker logs api-gateway  # View logs
```

## Testing

### Run Tests

```bash
# All tests
npm test

# Tests in watch mode
npm run test:watch

# With coverage
npm test -- --coverage
```

### Included Tests

- Route validation
- Error handling
- Security headers
- Request IDs
- Microservice integration
- Health checks

## Monitoring

### Logs

The gateway produces structured logs:

```
[abc123] Calling: "./build/src/services/users/users_service" "GET" "/users"
[abc123] users completed in 23ms
```

### Metrics via Health Endpoint

The `/health` endpoint provides real-time metrics:

- Individual service status
- Service response times
- Current gateway configuration

### Request Tracing

Each request receives a unique ID that can be used for tracing:

- `X-Request-ID` header in response
- `requestId` field in JSON response
- Logs with Request ID

## Development

### Project Structure

```
├── api-gateway.js          # Main gateway code
├── package.json           # Dependencies and scripts
├── Dockerfile.gateway     # Gateway container
├── __tests__/            # Automated tests
│   └── api-gateway.test.js
└── build/                # Compiled microservices
    └── src/
        └── services/     # C++ executables location
    └── services/
        ├── users/
        │   └── users_service
        └── orders/
            └── orders_service
```

### Adding New Microservices

1. **Compile** the C++ executable in `build/src/services/name/`
2. **Add routes** in `api-gateway.js`
3. **Include** in health check
4. **Add tests** in `__tests__/`

### Debug

```bash
# Run with detailed logs
DEBUG=* npm start

# Run in development mode
npm run dev
```

## Contributing

1. Fork the project
2. Create a branch for your feature
3. Add tests for new functionality
4. Run `npm test` to verify
5. Submit a pull request

## License

MIT License - see LICENSE file for details.

---

**API Gateway is production ready!**

For support or questions, check container logs and the `/health` endpoint for diagnostics.
````
