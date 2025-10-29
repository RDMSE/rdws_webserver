````markdown
# API Gateway - curl commands

## First, start the server:
```bash
cd /home/rdias/sources/lab/rdws_webserver
# Build the services with Clean Architecture
make -C build
# Start the API Gateway
NODE_ENV=development npm run build && node dist/src/api-gateway/api-gateway.js
```

Server will be available on : **http://localhost:8080**

---

## curl commands for testing

### 1. **Health Check** - Verify if server is running
```bash
curl -X GET http://localhost:8080/health
```
**Expected response**: Status of microservices, response time, configuration

### 2. **API Documentation** - List all available endpoints
```bash
curl -X GET http://localhost:8080/api-docs
```
**Expected response**: Complete API documentation

---

## **Users Service** (Clean Architecture with Controllers)

### 3. **List all users**
```bash
curl -X GET http://localhost:8080/users
```

### 4. **Search user by ID**
```bash
curl -X GET http://localhost:8080/users/1
curl -X GET http://localhost:8080/users/2
curl -X GET http://localhost:8080/users/42
```

### 5. **Get user count**
```bash
curl -X GET http://localhost:8080/users/count
```

### 6. **Create new user**
```bash
curl -X POST http://localhost:8080/users \
  -H "Content-Type: application/json" \
  -d '{"name":"João Silva","email":"joao@example.com"}'

curl -X POST http://localhost:8080/users \
  -H "Content-Type: application/json" \
  -d '{"name":"Maria Santos","email":"maria@example.com"}'
```

### 7. **Update user**
```bash
curl -X PUT http://localhost:8080/users/1 \
  -H "Content-Type: application/json" \
  -d '{"name":"João Santos","email":"joao.santos@example.com"}'
```

### 8. **Delete user**
```bash
curl -X DELETE http://localhost:8080/users/1
```

### 9. **Search invalid user** (for error testing)
```bash
curl -X GET http://localhost:8080/users/999
curl -X GET http://localhost:8080/users/abc
```

---

## **Orders Service** (Clean Architecture with Controllers)

### 10. **List all orders**
```bash
curl -X GET http://localhost:8080/orders
```

### 11. **Search order by ID**
```bash
curl -X GET http://localhost:8080/orders/1
curl -X GET http://localhost:8080/orders/25
curl -X GET http://localhost:8080/orders/37
```

### 12. **Get order count**
```bash
curl -X GET http://localhost:8080/orders/count
```

### 13. **Search orders for a specific user**
```bash
curl -X GET http://localhost:8080/users/1/orders
curl -X GET http://localhost:8080/users/2/orders
curl -X GET http://localhost:8080/users/4/orders
```

### 14. **Create new order**
```bash
curl -X POST http://localhost:8080/orders \
  -H "Content-Type: application/json" \
  -d '{"userId":1,"product":"Laptop Pro","amount":1299.99,"status":"pending"}'

curl -X POST http://localhost:8080/orders \
  -H "Content-Type: application/json" \
  -d '{"userId":2,"product":"Mouse","amount":29.99,"status":"completed"}'
```

### 15. **Update order**
```bash
curl -X PUT http://localhost:8080/orders/1 \
  -H "Content-Type: application/json" \
  -d '{"status":"completed","amount":1199.99}'
```

### 16. **Delete order**
```bash
curl -X DELETE http://localhost:8080/orders/1
```

---

## 🔍 **Examples with verbose output** (for debug)

### With headers and request details:
```bash
curl -v -X GET http://localhost:8080/users
```

### With timing analysis:
```bash
curl -w "Time: %{time_total}s\nStatus: %{http_code}\nSize: %{size_download} bytes\n" \
  -X GET http://localhost:8080/users

# Detailed timing breakdown
curl -w "@-" -X GET http://localhost:8080/users <<'EOF'
     time_namelookup:  %{time_namelookup}\n
        time_connect:  %{time_connect}\n
     time_appconnect:  %{time_appconnect}\n
    time_pretransfer:  %{time_pretransfer}\n
       time_redirect:  %{time_redirect}\n
  time_starttransfer:  %{time_starttransfer}\n
                     ----------\n
          time_total:  %{time_total}\n
EOF
```

---

## **Expected Responses**

### **User success (formatted by UserController):**
```json
{
  "success": true,
  "statusCode": 200,
  "users": [
    {
      "id": 1,
      "name": "John Doe",
      "email": "john@example.com",
      "created_at": "2025-10-27 10:17:39.716947"
    }
  ],
  "total": 10,
  "source": "users-service C++ with Clean Architecture",
  "timestamp": 1729875123,
  "gateway": {
    "requestId": "generated-request-id",
    "serviceName": "users",
    "executionTime": 45,
    "timestamp": "2025-10-27T07:25:30.123Z"
  }
}
```

### **Single user response:**
```json
{
  "success": true,
  "statusCode": 200,
  "user": {
    "id": 1,
    "name": "John Doe",
    "email": "john@example.com",
    "created_at": "2025-10-27 10:17:39.716947"
  },
  "source": "users-service",
  "timestamp": 1729875123
}
```

### **Orders success (formatted by OrderController):**
```json
{
  "success": true,
  "statusCode": 200,
  "orders": [
    {
      "id": 1,
      "userId": 1,
      "product": "Laptop Pro",
      "amount": 1299.99,
      "status": "completed",
      "createdAt": "2025-10-27 10:17:39.71977"
    }
  ],
  "total": 42,
  "source": "orders-service C++ with Clean Architecture",
  "timestamp": 1729875123,
  "gateway": {
    "requestId": "generated-request-id", 
    "serviceName": "orders",
    "executionTime": 67,
    "timestamp": "2025-10-27T07:25:30.456Z"
  }
}
```

### **Count response:**
```json
{
  "success": true,
  "statusCode": 200,
  "count": 15,
  "source": "users-service",
  "timestamp": 1729875123
}
```

### **Error response (404 - formatted by BaseController):**
```json
{
  "error": true,
  "message": "User not found",
  "statusCode": 404,
  "timestamp": 1729875123,
  "service": "users-service"
}
```

### **Validation error (400):**
```json
{
  "error": true,
  "message": "Invalid user ID",
  "statusCode": 400,
  "timestamp": 1729875123,
  "service": "users-service"
}
```

### **Database error (500):**
```json
{
  "error": true,
  "message": "Database connection failed",
  "statusCode": 500,
  "timestamp": 1729875123,
  "service": "users-service"
}
```

---


## **Troubleshooting:**

### If the server does not respond:
1. Check if it is running: `ps aux | grep node`
2. Check the port: `netstat -tulpn | grep 8080`
3. Check the logs in the terminal where the server was started

### If you get a 500 error:
1. **Compile the services**: `make -C build`
2. **Check executables**: `ls -la build/src/services/*/`
3. **Verify PostgreSQL**: Check if database is running and accessible
4. **Check logs**: Look at API Gateway terminal for detailed error messages

### If compilation fails:
1. **Clean build**: `make -C build clean && make -C build`
2. **Check dependencies**: Ensure libpqxx, jsoncpp are installed
3. **Verify includes**: All should use simplified paths like `"common/database/idatabase.h"`

### Database connection issues:
1. **Check .env file**: Verify database credentials
2. **Test connection**: Use `scripts/test-postgresql.sh`
3. **Check permissions**: Ensure PostgreSQL allows connections

---

## **Testing:**

```bash
# Run all unit tests (should show 24/24 passing)
make -C build && ./build/tests/users_service_unit_tests && ./build/tests/orders_service_unit_tests

# Quick API test
curl -X GET http://localhost:8080/health

# Test Clean Architecture response format
curl -X GET http://localhost:8080/users | jq '.'
```

````

