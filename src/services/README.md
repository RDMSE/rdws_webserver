````markdown
# C++ Executable Services

This folder contains **independent C++ executables** that implement the **"one executable per endpoint"** vision.

## Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  API Gateway    │───▶│   C++ Service   │───▶│    Response     │
│ (TypeScript)    │    │   (Executable)  │    │     (JSON)      │
│   Port 8080     │    │  ./users_service│    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Structure

```
src/services/
├── users/
│   ├── main.cpp           # Users service logic
│   ├── CMakeLists.txt     # Build config
│   └── users_service      # Executable (after build)
├── orders/
│   ├── main.cpp           # Orders service logic
│   ├── CMakeLists.txt     # Build config
│   └── orders_service     # Executable (after build)
└── CMakeLists.txt         # General config
```

## How It Works

### **1. API Gateway calls C++ Executable**

```typescript
// src/api-gateway/api-gateway.ts
const servicePath = `${config.buildPath}/src/services/${serviceName}/${serviceName}_service`;
const command = `"${servicePath}" "${method}" "${path}"`;

const { stdout } = await execAsync(command, { timeout: 5000 });
const response = JSON.parse(stdout);
```

### **2. C++ Executable processes and returns JSON**

```cpp
// services/users/main.cpp
int main(int argc, char* argv[]) {
    std::string method = argv[1]; // "GET"
    std::string path = argv[2];   // "/users"

    // Process business logic
    std::string json = processUsers(method, path);

    // Return JSON via stdout
    std::cout << json << std::endl;
    return 0;
}
```

## Benefits

| Aspect | Advantage |
|---------|----------|
| **Isolation** | Each service is completely independent |
| **Performance** | Native C++, no framework overhead |
| **Scalability** | Can run N instances of the same executable |
| **Debugging** | Can test executable directly |
| **Deploy** | Single executable, no dependencies |

## Development

### **Create New Service**

```bash
# 1. Create directory
mkdir src/services/products

# 2. Create main.cpp
cat > src/services/products/main.cpp << 'EOF'
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string method = argc > 1 ? argv[1] : "GET";
    std::string path = argc > 2 ? argv[2] : "/products";

    std::cout << R"({"products":[],"source":"products_service"})" << std::endl;
    return 0;
}
EOF

# 3. Create CMakeLists.txt
cat > src/services/products/CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.10)
project(ProductsService VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(products_service main.cpp)
EOF

# 4. Add to main CMakeLists.txt
echo "add_subdirectory(products)" >> src/services/CMakeLists.txt
```

### **Test Locally**

```bash
# Build
cd build && make

# Test executable directly
# 1. GET /users (list all)
./users_service \
    '{"httpMethod":"GET","path":"/users","resource":"/users","headers":{"Content-Type":"application/json"},"queryStringParameters":{},"pathParameters":{},"body":"","isBase64Encoded":false}' \
    '{"requestId":"test-123","functionName":"users-service","functionVersion":"1.0","timeout":30000,"memoryLimitMB":128}'

# 2. GET /users/1 (find user) 
./users_service \
    '{"httpMethod":"GET","path":"/users/1","resource":"/users/{id}","headers":{},"queryStringParameters":{},"pathParameters":{"id":"1"},"body":"","isBase64Encoded":false}' \
    '{"requestId":"test-456","functionName":"users-service","functionVersion":"1.0","timeout":30000,"memoryLimitMB":128}'
    
# 3. GET /users/count (count users)
./users_service \
    '{"httpMethod":"GET","path":"/users/count","resource":"/users/{id}","headers":{},"queryStringParameters":{},"pathParameters":{"id":"count"},"body":"","isBase64Encoded":false}' \
    '{"requestId":"test-789","functionName":"users-service","functionVersion":"1.0","timeout":30000,"memoryLimitMB":128}'

# 4. POST /users (create new user)
./users_service \
    '{"httpMethod":"POST","path":"/users","resource":"/users","headers":{"Content-Type":"application/json"},"queryStringParameters":{},"pathParameters":{},"body":"{\"name\":\"João Silva\",\"email\":\"joao@example.com\"}","isBase64Encoded":false}' \
    '{"requestId":"test-create","functionName":"users-service","functionVersion":"1.0","timeout":30000,"memoryLimitMB":128}'

# 5. PUT /users/1 (update user)
./users_service \
    '{"httpMethod":"PUT","path":"/users/1","resource":"/users/{id}","headers":{"Content-Type":"application/json"},"queryStringParameters":{},"pathParameters":{"id":"1"},"body":"{\"name\":\"João Santos\",\"email\":\"joao.santos@example.com\"}","isBase64Encoded":false}' \
    '{"requestId":"test-update","functionName":"users-service","functionVersion":"1.0","timeout":30000,"memoryLimitMB":128}'
    
# 6. DELETE /users/1 (delete user)
./users_service \
    '{"httpMethod":"DELETE","path":"/users/1","resource":"/users/{id}","headers":{},"queryStringParameters":{},"pathParameters":{"id":"1"},"body":"","isBase64Encoded":false}' \
    '{"requestId":"test-delete","functionName":"users-service","functionVersion":"1.0","timeout":30000,"memoryLimitMB":128}'
    
# 7. Error test (invalid ID)
./users_service \
    '{"httpMethod":"GET","path":"/users/abc","resource":"/users/{id}","headers":{},"queryStringParameters":{},"pathParameters":{"id":"abc"},"body":"","isBase64Encoded":false}' \
    '{"requestId":"test-error","functionName":"users-service","functionVersion":"1.0","timeout":30000,"memoryLimitMB":128}'

# Result: Pure JSON from C++
{"users":[...],"source":"users_service C++ executable"}
```

## Final URLs

After deploy via GitHub Actions:

| Endpoint | Access |
|----------|--------|
| **API Gateway** | `http://server:8080/` |
| **Users API** | `http://server:8080/users` |
| **Orders API** | `http://server:8080/orders` |
| **Health Check** | `http://server:8080/health` |
| **API Docs** | `http://server:8080/api-docs` |
| **Users Executable** | `./build/src/services/users/users_service "GET" "/users"` |
| **Orders Executable** | `./build/src/services/orders/orders_service "GET" "/orders"` |

## Complete Flow

```
1. Client → GET http://server:8080/users
2. API Gateway → Route to users service
3. Gateway → exec("./build/src/services/users/users_service GET /users")
4. C++ Executable → Process & return JSON
5. API Gateway → Parse JSON & add metadata
6. Client ← JSON Response with gateway info
```

**This is the REAL implementation of "one executable per endpoint" with unified API Gateway!**
````
