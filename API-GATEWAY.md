# 🚀 API Gateway - C++ Microservices

Um gateway unificado para acessar microserviços C++ através de uma interface HTTP simples e robusta.

## 📋 Índice

- [Visão Geral](#visão-geral)
- [Funcionalidades](#funcionalidades)
- [Instalação e Execução](#instalação-e-execução)
- [API Endpoints](#api-endpoints)
- [Exemplos de Uso](#exemplos-de-uso)
- [Configuração](#configuração)
- [Docker](#docker)
- [Testes](#testes)
- [Monitoramento](#monitoramento)

## 🎯 Visão Geral

O API Gateway fornece uma interface HTTP unificada para acessar múltiplos microserviços C++ executáveis. Ele centraliza:

- **Roteamento** de requisições para microserviços apropriados
- **Tratamento de erros** padronizado
- **Logging** e rastreamento de requisições
- **Validação** de parâmetros
- **Monitoramento** de saúde dos serviços

### Arquitetura

```
Client → API Gateway (porta 8080) → C++ Executáveis
                ↓
    ┌─────────────────┬─────────────────┐
    │ users_service   │ orders_service  │
    │ (executable)    │ (executable)    │
    └─────────────────┴─────────────────┘
```

## ✨ Funcionalidades

- ✅ **Proxy transparente** para microserviços C++
- ✅ **Tratamento de erros** robusto com códigos HTTP apropriados
- ✅ **Request ID** para rastreamento de requisições
- ✅ **Health checks** automáticos dos serviços
- ✅ **Validação** de parâmetros de entrada
- ✅ **Timeout** configurável para evitar travamentos
- ✅ **CORS** e headers de segurança
- ✅ **Logging** detalhado com timestamps
- ✅ **Graceful shutdown** para deploy sem downtime

## 🚀 Instalação e Execução

### Requisitos

- Node.js 18+
- C++ microserviços compilados em `./build/services/`

### Instalação Local

```bash
# 1. Instalar dependências
npm install

# 2. Compilar microserviços C++
mkdir -p build && cd build
cmake .. && make

# 3. Executar API Gateway
npm start

# Ou para desenvolvimento (com auto-reload)
npm run dev
```

### Verificação

```bash
# Testar se está funcionando
curl http://localhost:8080/health
```

## 📡 API Endpoints

### Base URL: `http://localhost:8080`

| Método | Endpoint | Descrição |
|--------|----------|-----------|
| `GET` | `/health` | Status dos serviços e saúde do gateway |
| `GET` | `/api-docs` | Documentação da API |
| `GET` | `/users` | Listar todos os usuários |
| `GET` | `/users/:id` | Obter usuário por ID |
| `GET` | `/orders` | Listar todos os pedidos |
| `GET` | `/orders/:id` | Obter pedido por ID |
| `GET` | `/users/:userId/orders` | Obter pedidos de um usuário |

### Respostas

Todas as respostas incluem metadados do gateway:

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

### Tratamento de Erros

```json
{
  "error": true,
  "message": "Service users timed out after 5000ms",
  "requestId": "abc123",
  "timestamp": "2025-10-03T10:00:00.000Z"
}
```

## 🧪 Exemplos de Uso

### 1. Listar Usuários

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

### 2. Obter Usuário por ID

```bash
curl -X GET http://localhost:8080/users/1
```

### 3. Verificar Saúde dos Serviços

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
// Usando fetch
const response = await fetch('http://localhost:8080/users');
const data = await response.json();

if (data.error) {
    console.error('Erro:', data.message);
} else {
    console.log('Usuários:', data.users);
    console.log('Request ID:', data.gateway.requestId);
}
```

### 5. Python

```python
import requests

response = requests.get('http://localhost:8080/orders')
data = response.json()

if data.get('error'):
    print(f"Erro: {data['message']}")
else:
    print(f"Pedidos: {len(data['orders'])}")
```

## ⚙️ Configuração

### Variáveis de Ambiente

| Variável | Padrão | Descrição |
|----------|--------|-----------|
| `PORT` | `8080` | Porta do servidor |
| `BUILD_PATH` | `./build` | Caminho para executáveis compilados |
| `NODE_ENV` | `development` | Ambiente (development/production) |
| `SERVICE_TIMEOUT` | `5000` | Timeout em ms para microserviços |

### Exemplo

```bash
export PORT=3000
export BUILD_PATH=/app/build
export SERVICE_TIMEOUT=10000
npm start
```

## 🐳 Docker

### Construir Imagem

```bash
docker build -f Dockerfile.gateway -t api-gateway .
```

### Executar Container

```bash
docker run -d \
  --name api-gateway \
  -p 8080:8080 \
  -v $(pwd)/build:/app/build:ro \
  api-gateway
```

### Docker Compose

```bash
# Executar apenas o gateway
docker-compose up api-gateway

# Executar com funções serverless opcionais
docker-compose --profile serverless up
```

### Health Check

O container inclui health check automático:

```bash
docker ps  # Ver status de saúde
docker logs api-gateway  # Ver logs
```

## 🧪 Testes

### Executar Testes

```bash
# Todos os testes
npm test

# Testes em modo watch
npm run test:watch

# Com coverage
npm test -- --coverage
```

### Testes Incluídos

- ✅ Validação de rotas
- ✅ Tratamento de erros
- ✅ Headers de segurança
- ✅ Request IDs
- ✅ Integração com microserviços
- ✅ Health checks

## 📊 Monitoramento

### Logs

O gateway produz logs estruturados:

```
[abc123] Calling: "./build/services/users/users_service" "GET" "/users"
[abc123] users completed in 23ms
```

### Métricas via Health Endpoint

O endpoint `/health` fornece métricas em tempo real:

- Status individual de cada serviço
- Tempo de resposta dos serviços
- Configuração atual do gateway

### Rastreamento de Requisições

Cada requisição recebe um ID único que pode ser usado para rastreamento:

- Header `X-Request-ID` na resposta
- Campo `requestId` no JSON de resposta
- Logs com o Request ID

## 🔧 Desenvolvimento

### Estrutura do Projeto

```
├── api-gateway.js          # Código principal do gateway
├── package.json           # Dependências e scripts
├── Dockerfile.gateway     # Container do gateway
├── __tests__/            # Testes automatizados
│   └── api-gateway.test.js
└── build/                # Microserviços compilados
    └── services/
        ├── users/
        │   └── users_service
        └── orders/
            └── orders_service
```

### Adicionando Novos Microserviços

1. **Compilar** o executável C++ em `build/services/nome/`
2. **Adicionar rotas** no `api-gateway.js`
3. **Incluir** no health check
4. **Adicionar testes** em `__tests__/`

### Debug

```bash
# Executar com logs detalhados
DEBUG=* npm start

# Executar em modo desenvolvimento
npm run dev
```

## 🤝 Contribuição

1. Faça fork do projeto
2. Crie uma branch para sua feature
3. Adicione testes para novas funcionalidades
4. Execute `npm test` para verificar
5. Envie um pull request

## 📝 Licença

MIT License - veja arquivo LICENSE para detalhes.

---

**🚀 API Gateway está pronto para produção!**

Para suporte ou dúvidas, verifique os logs do container e o endpoint `/health` para diagnóstico.