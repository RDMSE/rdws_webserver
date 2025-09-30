# Serverless Functions

Este diretório contém as funções serverless que estendem o servidor C++ REST.

## Arquitetura

- **Servidor Principal**: C++ com Pistache na porta 9080
- **Funções Serverless**: Node.js containers na porta 8082

## Funções Disponíveis

### hello-simple
- **Endpoint**: http://localhost:8082/
- **Método**: GET
- **Resposta**: JSON com timestamp e identificação serverless
- **Equivalente a**: `/hello` do servidor principal

## Deploy

As funções são automaticamente buildadas e deployadas via GitHub Actions:

1. `faas-cli build` - Constrói as imagens Docker
2. `docker run` - Deploy como containers standalone
3. Health checks automáticos

## Desenvolvimento Local

```bash
# Build function
cd functions
faas-cli build -f stack.yaml

# Run locally
docker run -d --name hello-function -p 8082:8080 local/hello-simple:latest

# Test
curl http://localhost:8082/
```

## Próximos Passos

1. Implementar proxy no servidor C++ para rotear `/hello` → função serverless
2. Adicionar mais endpoints como funções serverless
3. Migrar para arquitetura "um executável por endpoint"
