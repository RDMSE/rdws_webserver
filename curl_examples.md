# API Gateway - curl commands

## first, start the server:
```bash
cd /home/rdias/sources/lab/rdws_webserver
NODE_ENV=development node dist/src/api-gateway/api-gateway.js
```

Server will be available on : **http://localhost:8080**

---

## curl commands for testing

### 1. **Health Check** - Verify if server is running
```bash
curl -X GET http://localhost:8080/health
```
**Resposta esperada**: Status dos microserviços, tempo de resposta, configuração

### 2. **API Documentation** - List all available endpoints
```bash
curl -X GET http://localhost:8080/api-docs
```
**Expected answer** Complete API documentation

---

## **Users Service**

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

### 5. **Search invalid user** (for error testing)
```bash
curl -X GET http://localhost:8080/users/999
curl -X GET http://localhost:8080/users/abc
```

---

## **Orders Service** 

### 6. **List all orders**
```bash
curl -X GET http://localhost:8080/orders
```

### 7. **Search order by ID**
```bash
curl -X GET http://localhost:8080/orders/1
curl -X GET http://localhost:8080/orders/25
curl -X GET http://localhost:8080/orders/37
```

### 8. **Search orders for a specific user**
```bash
curl -X GET http://localhost:8080/users/1/orders
curl -X GET http://localhost:8080/users/2/orders
curl -X GET http://localhost:8080/users/4/orders
```

---

## 🔍 **Examples with verbose output** (for debug)

### With headres and request details:
```bash
curl -v -X GET http://localhost:8080/users
```

### with timing:
```bash
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

##  **What we should receive as answer:**

### **User success:**
```json
{
  "success": true,
  "statusCode": 200,
  "users": [
    {
      "id": 1,
      "name": "John Doe",
      "email": "john@example.com",
      "created_at": "2025-10-07 10:17:39.716947"
    }
  ],
  "total": 10,
  "source": "microservice C++ with PostgreSQL",
  "timestamp": 1761203926,
  "gateway": {
    "requestId": "generated-request-id",
    "serviceName": "users",
    "executionTime": 45,
    "timestamp": "2025-10-23T07:25:30.123Z"
  }
}
```

### **Orders success:**
```json
{
  "orders": [
    {
      "id": 1,
      "userId": 1,
      "product": "Laptop Pro",
      "amount": 999.99,
      "status": "completed",
      "createdAt": "2025-10-07 10:17:39.71977"
    }
  ],
  "total": 42,
  "source": "orders_service C++ executable",
  "endpoint": "/orders",
  "timestamp": 1761203899,
  "gateway": {
    "requestId": "generated-request-id", 
    "serviceName": "orders",
    "executionTime": 67,
    "timestamp": "2025-10-23T07:25:30.456Z"
  }
}
```

### **Error response (404):**
```json
{
  "error": true,
  "message": "Route GET /invalid-route not found",
  "requestId": "generated-request-id",
  "availableRoutes": ["/health", "/api-docs", "/users", "/orders"]
}
```

---

## **Key Features:**

1. **Request ID**: Each request has an unique ID for tracking
2. **Gateway Metadata**: Detailed execution information (timming, service, etc.)  
3. **Error Handling**: Robust error handling with details
4. **Event/Context**: Internally uses AWS Lambda-like Event/Context
5. **JSON Integration**: API Gateway serialises Event/Context for C++ microservices
6. **Performance**: C++ executables for maximum performance
7. **Observability**: Structured logs with timestamps and request IDs

---

## **Troubleshooting:**

### If the server does not respond:
1. Check if it is running: `ps aux | grep node`
2. Check the port: `netstat -tulpn | grep 8080`
3. Check the logs in the terminal where the server was uploaded

### If you get a 500 error:
1. Check if the C++ microservices have been compiled: `ls -la build/src/services/*/`
2. Check if the PostgreSQL database is running
3. Check the logs in the API Gateway terminal

