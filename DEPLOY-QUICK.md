# 🚀 Deploy Rápido no Fedora Server

Este guia apresenta a forma mais rápida de fazer deploy do API Gateway no Fedora Server.

## ⚡ Deploy em 1 Comando

```bash
# Download e execução do script automatizado
curl -fsSL https://raw.githubusercontent.com/RDMSE/rdws_webserver/14-featureserverinfra-enable-servless-archtecture/scripts/deploy-fedora.sh | bash
```

## 📋 O que o script faz automaticamente:

1. ✅ **Verifica dependências** (Node.js, CMake, build tools)
2. ✅ **Instala dependências faltantes** (se executado como root)
3. ✅ **Clona/atualiza o repositório** em `/opt/rdws_webserver`
4. ✅ **Compila os microserviços C++**
5. ✅ **Configura o firewall** (porta 8080)
6. ✅ **Inicia o serviço** (PM2, Docker ou systemd)
7. ✅ **Verifica saúde** do serviço

## 🔧 Deploy Manual Passo a Passo

Se preferir fazer manualmente:

### 1. Preparar Sistema
```bash
# Atualizar sistema
sudo dnf update -y

# Instalar dependências
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y cmake gcc-c++ git nodejs npm

# Instalar PM2 (recomendado)
sudo npm install -g pm2
```

### 2. Fazer Deploy
```bash
# Clonar projeto
sudo git clone https://github.com/RDMSE/rdws_webserver.git /opt/rdws_webserver
sudo chown -R $USER:$USER /opt/rdws_webserver
cd /opt/rdws_webserver

# Checkout da branch
git checkout 14-featureserverinfra-enable-servless-archtecture

# Instalar dependências Node.js
npm install --production

# Compilar microserviços
mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ..

# Configurar firewall
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload

# Iniciar com PM2
pm2 start api-gateway.js --name api-gateway
pm2 save && pm2 startup
```

### 3. Verificar Deploy
```bash
# Executar verificação completa
./scripts/verify-deploy.sh

# Ou teste manual
curl http://localhost:8080/health
```

## 🌐 Acessar de Outras Máquinas

```bash
# Substituir SEU_IP pelo IP do servidor
curl http://SEU_IP:8080/health
curl http://SEU_IP:8080/users
curl http://SEU_IP:8080/orders
```

## 🔧 Gerenciamento do Serviço

### PM2 (Recomendado)
```bash
pm2 status                # Status
pm2 logs api-gateway      # Logs
pm2 restart api-gateway   # Reiniciar
pm2 stop api-gateway      # Parar
pm2 monit                 # Monitor
```

### Docker
```bash
docker ps                      # Status
docker logs api-gateway        # Logs  
docker restart api-gateway     # Reiniciar
docker stop api-gateway        # Parar
```

### Systemd
```bash
sudo systemctl status api-gateway     # Status
sudo journalctl -u api-gateway -f     # Logs
sudo systemctl restart api-gateway    # Reiniciar
sudo systemctl stop api-gateway       # Parar
```

## 🆘 Solução de Problemas

### Verificar se está rodando
```bash
# Verificar processos
ps aux | grep api-gateway

# Verificar porta
sudo netstat -tlnp | grep :8080

# Teste básico
curl http://localhost:8080/health
```

### Logs de Debug
```bash
# PM2
pm2 logs api-gateway --err

# Docker
docker logs api-gateway

# Systemd
sudo journalctl -u api-gateway --since "5 minutes ago"
```

### Recompilar Microserviços
```bash
cd /opt/rdws_webserver/build
make clean && make -j$(nproc)

# Reiniciar serviço após recompilação
pm2 restart api-gateway
```

### Firewall
```bash
# Verificar se porta está aberta
sudo firewall-cmd --list-ports

# Abrir porta se necessário
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload
```

## 📊 Endpoints Disponíveis

| Endpoint | Descrição |
|----------|-----------|
| `GET /health` | Status dos serviços |
| `GET /users` | Lista usuários |
| `GET /users/:id` | Usuário por ID |
| `GET /orders` | Lista pedidos |
| `GET /orders/:id` | Pedido por ID |
| `GET /users/:userId/orders` | Pedidos de um usuário |
| `GET /api-docs` | Documentação da API |

## 🔄 Atualização

Para atualizar o código:

```bash
cd /opt/rdws_webserver
git pull origin 14-featureserverinfra-enable-servless-archtecture
npm install --production
cd build && make -j$(nproc) && cd ..
pm2 restart api-gateway
```

## ✅ Verificação de Sucesso

Se tudo deu certo, você deve ver:

```bash
$ curl http://SEU_IP:8080/health
{
  "status": "ok",
  "timestamp": "2025-10-03T10:00:00.000Z",
  "services": {
    "users": {"status": "healthy", "responseTime": 12},
    "orders": {"status": "healthy", "responseTime": 8}
  }
}
```

## 🎉 Pronto!

Seu API Gateway está rodando! Acesse:
- **API:** http://SEU_IP:8080
- **Health:** http://SEU_IP:8080/health  
- **Docs:** http://SEU_IP:8080/api-docs

---

**Para suporte detalhado, consulte: [DEPLOY-FEDORA.md](DEPLOY-FEDORA.md)**