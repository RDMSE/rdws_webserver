# Serviços C++ Executáveis

Esta pasta contém **executáveis C++ independentes** que implementam a visão **"um executável por endpoint"**.

## 🏗️ Arquitetura

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ Serverless Func │───▶│   C++ Service   │───▶│    Response     │
│   (Node.js)     │    │   (Executable)  │    │     (JSON)      │
│   Port 8083     │    │  ./users_service│    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 📂 Estrutura

```
services/
├── users/
│   ├── main.cpp           # Lógica do serviço de usuários
│   ├── CMakeLists.txt     # Build config
│   └── users_service      # Executável (após build)
├── orders/
│   ├── main.cpp           # Lógica do serviço de pedidos  
│   ├── CMakeLists.txt     # Build config
│   └── orders_service     # Executável (após build)
└── CMakeLists.txt         # Config geral
```

## 🚀 Como Funciona

### **1. Função Serverless chama Executável C++**

```javascript
// functions/users-api/handler.js
const { exec } = require('child_process');

const command = '/path/to/users_service "GET" "/users"';
const { stdout } = await execAsync(command);
const response = JSON.parse(stdout);
```

### **2. Executável C++ processa e retorna JSON**

```cpp
// services/users/main.cpp
int main(int argc, char* argv[]) {
    std::string method = argv[1]; // "GET"
    std::string path = argv[2];   // "/users"
    
    // Processar lógica de negócio
    std::string json = processUsers(method, path);
    
    // Retornar JSON via stdout
    std::cout << json << std::endl;
    return 0;
}
```

## 🎯 Benefícios

| Aspecto | Vantagem |
|---------|----------|
| **Isolamento** | ✅ Cada serviço é completamente independente |
| **Performance** | ⚡ C++ nativo, sem overhead de framework |
| **Escalabilidade** | 🚀 Pode executar N instâncias do mesmo executável |
| **Debugging** | 🔍 Pode testar executável diretamente |
| **Deploy** | 📦 Executável único, sem dependências |

## 🛠️ Desenvolvimento

### **Criar Novo Serviço**

```bash
# 1. Criar diretório
mkdir services/products

# 2. Criar main.cpp
cat > services/products/main.cpp << 'EOF'
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string method = argc > 1 ? argv[1] : "GET";
    std::string path = argc > 2 ? argv[2] : "/products";
    
    std::cout << R"({"products":[],"source":"products_service"})" << std::endl;
    return 0;
}
EOF

# 3. Criar CMakeLists.txt
cat > services/products/CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.10)
project(ProductsService VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(products_service main.cpp)
EOF

# 4. Adicionar ao CMakeLists.txt principal
echo "add_subdirectory(products)" >> services/CMakeLists.txt
```

### **Testar Localmente**

```bash
# Build
cd build && make

# Testar executável diretamente
./services/users/users_service "GET" "/users"
./services/users/users_service "GET" "/users/1"

# Resultado: JSON puro do C++
{"users":[...],"source":"users_service C++ executable"}
```

## 📡 URLs Finais

Após deploy via GitHub Actions:

| Endpoint | Acesso |
|----------|--------|
| **Função Users** | `http://servidor:8083/` |
| **Função Orders** | `http://servidor:8084/` |
| **Executável Users** | `./services/users_service "GET" "/users"` |
| **Executável Orders** | `./services/orders_service "GET" "/orders"` |

## 🔄 Fluxo Completo

```
1. Client → GET http://servidor:8083/
2. Node.js Function → exec("./users_service GET /users")  
3. C++ Executable → Process & return JSON
4. Node.js Function → Parse JSON & respond
5. Client ← JSON Response
```

**Esta é a implementação REAL de "um executável por endpoint"!** 🎯