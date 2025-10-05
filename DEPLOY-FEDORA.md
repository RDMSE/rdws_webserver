# 🚀 Deploy no Fedora Server - API Gateway + Microserviços C++

Guia completo para fazer deploy da sua arquitetura de microserviços no Fedora Server.

## 📋 Pré-requisitos no Servidor

### 1. Sistema Atualizado
```bash
sudo dnf update -y
```

### 2. Instalar Dependências Base
```bash
# Ferramentas de desenvolvimento
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y cmake gcc-c++ git wget curl

# Node.js (versão LTS)
curl -fsSL https://rpm.nodesource.com/setup_lts.x | sudo bash -
sudo dnf install -y nodejs

# PM2 para gerenciar processos Node.js
sudo npm install -g pm2

# Firewall e rede
sudo dnf install -y firewalld
```

### 3. Configurar Firewall
```bash
# Habilitar firewall
sudo systemctl enable --now firewalld

# Abrir porta do API Gateway
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload

# Verificar
sudo firewall-cmd --list-ports
```

## 📦 Deploy Manual (Método 1)

### 1. Clonar e Preparar Projeto
```bash
# No servidor
cd /opt
sudo git clone https://github.com/RDMSE/rdws_webserver.git
sudo chown -R $USER:$USER rdws_webserver
cd rdws_webserver

# Checkout da branch correta
git checkout 14-featureserverinfra-enable-servless-archtecture
```

### 2. Instalar Dependências Node.js
```bash
npm install --production
```

### 3. Compilar Microserviços C++
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Verificar se executáveis foram criados
ls -la services/*/
```

### 4. Configurar Variáveis de Ambiente
```bash
# Criar arquivo de configuração
sudo tee /opt/rdws_webserver/.env << 'EOF'
NODE_ENV=production
PORT=8080
BUILD_PATH=/opt/rdws_webserver/build
SERVICE_TIMEOUT=5000
EOF
```

### 5. Configurar PM2
```bash
cd /opt/rdws_webserver

# Criar arquivo de configuração do PM2
tee ecosystem.config.js << 'EOF'
module.exports = {
  apps: [{
    name: 'api-gateway',
    script: './api-gateway.js',
    instances: 'max',
    exec_mode: 'cluster',
    env: {
      NODE_ENV: 'production',
      PORT: 8080,
      BUILD_PATH: '/opt/rdws_webserver/build',
      SERVICE_TIMEOUT: 5000
    },
    error_file: '/var/log/api-gateway/error.log',
    out_file: '/var/log/api-gateway/out.log',
    log_file: '/var/log/api-gateway/combined.log',
    time: true,
    max_memory_restart: '500M',
    node_args: '--max-old-space-size=512'
  }]
};
EOF

# Criar diretório de logs
sudo mkdir -p /var/log/api-gateway
sudo chown $USER:$USER /var/log/api-gateway

# Iniciar aplicação
pm2 start ecosystem.config.js
pm2 save
pm2 startup

# Verificar status
pm2 status
pm2 logs
```

## 🐳 Deploy com Docker (Método 2 - Recomendado)

### 1. Instalar Docker
```bash
# Remover versões antigas
sudo dnf remove docker docker-client docker-client-latest docker-common docker-latest docker-latest-logrotate docker-logrotate docker-engine

# Adicionar repositório Docker
sudo dnf config-manager --add-repo https://download.docker.com/linux/fedora/docker-ce.repo

# Instalar Docker
sudo dnf install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# Habilitar e iniciar Docker
sudo systemctl enable --now docker

# Adicionar usuário ao grupo docker
sudo usermod -aG docker $USER
newgrp docker

# Verificar instalação
docker --version
docker compose version
```

### 2. Deploy com Docker Compose
```bash
cd /opt/rdws_webserver

# Compilar microserviços primeiro (necessário para o volume)
mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ..

# Fazer deploy
docker compose up -d api-gateway

# Verificar status
docker compose ps
docker compose logs api-gateway
```

## 🔧 Configuração de Sistema (Systemd)

### Método 3: Serviço Systemd Nativo

```bash
# Criar arquivo de serviço
sudo tee /etc/systemd/system/api-gateway.service << 'EOF'
[Unit]
Description=C++ Microservices API Gateway
Documentation=https://github.com/RDMSE/rdws_webserver
After=network.target
Wants=network.target

[Service]
Type=simple
User=rdias
Group=rdias
WorkingDirectory=/opt/rdws_webserver
ExecStart=/usr/bin/node api-gateway.js
Restart=always
RestartSec=3
TimeoutStopSec=10

# Environment
Environment=NODE_ENV=production
Environment=PORT=8080
Environment=BUILD_PATH=/opt/rdws_webserver/build
Environment=SERVICE_TIMEOUT=5000

# Logging
StandardOutput=journal
StandardError=journal
SyslogIdentifier=api-gateway

# Security
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ReadWritePaths=/opt/rdws_webserver
ProtectHome=true

[Install]
WantedBy=multi-user.target
EOF

# Recarregar systemd
sudo systemctl daemon-reload

# Habilitar e iniciar serviço
sudo systemctl enable api-gateway
sudo systemctl start api-gateway

# Verificar status
sudo systemctl status api-gateway
sudo journalctl -u api-gateway -f
```

## 🔍 Verificação do Deploy

### 1. Testar API Gateway
```bash
# Health check
curl http://localhost:8080/health

# Testar endpoints
curl http://localhost:8080/users
curl http://localhost:8080/orders
curl http://localhost:8080/api-docs
```

### 2. Testar de Máquina Externa
```bash
# Substitua SERVIDOR_IP pelo IP do seu servidor
curl http://SERVIDOR_IP:8080/health
```

### 3. Verificar Logs
```bash
# PM2
pm2 logs api-gateway

# Docker
docker compose logs api-gateway -f

# Systemd
sudo journalctl -u api-gateway -f
```

## 📊 Monitoramento

### 1. PM2 Monitoring
```bash
# Status detalhado
pm2 monit

# Reiniciar se necessário
pm2 restart api-gateway

# Ver métricas
pm2 show api-gateway
```

### 2. Docker Monitoring
```bash
# Status dos containers
docker compose ps

# Usar recursos
docker stats

# Health checks
docker compose exec api-gateway node -e "require('http').get('http://localhost:8080/health', res => console.log(res.statusCode))"
```

### 3. Logs do Sistema
```bash
# Logs do sistema
sudo journalctl -f

# Logs específicos do serviço
sudo journalctl -u api-gateway --since "1 hour ago"
```

## 🔒 Configurações de Segurança

### 1. SELinux (se habilitado)
```bash
# Verificar status do SELinux
getenforce

# Se necessário, criar políticas customizadas
sudo setsebool -P httpd_can_network_connect 1
```

### 2. Configurar Nginx como Proxy (Opcional)
```bash
# Instalar Nginx
sudo dnf install -y nginx

# Configurar proxy reverso
sudo tee /etc/nginx/conf.d/api-gateway.conf << 'EOF'
server {
    listen 80;
    server_name seu-dominio.com;

    location / {
        proxy_pass http://localhost:8080;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_cache_bypass $http_upgrade;
    }
}
EOF

# Habilitar e iniciar Nginx
sudo systemctl enable --now nginx

# Abrir porta 80 no firewall
sudo firewall-cmd --permanent --add-service=http
sudo firewall-cmd --reload
```

## 🚀 Script de Deploy Automatizado

Crie um script para automatizar o deploy:

```bash
# Criar script de deploy
tee deploy-fedora.sh << 'EOF'
#!/bin/bash

set -e

echo "🚀 Starting deployment on Fedora Server..."

# Atualizar código
echo "📥 Updating code..."
git pull origin 14-featureserverinfra-enable-servless-archtecture

# Instalar dependências
echo "📦 Installing dependencies..."
npm install --production

# Compilar microserviços
echo "🔨 Building microservices..."
cd build
make -j$(nproc)
cd ..

# Reiniciar serviço
echo "🔄 Restarting service..."
if command -v pm2 &> /dev/null; then
    pm2 restart api-gateway
elif command -v docker &> /dev/null; then
    docker compose restart api-gateway
else
    sudo systemctl restart api-gateway
fi

# Verificar saúde
echo "🏥 Health check..."
sleep 5
curl -f http://localhost:8080/health > /dev/null && echo "✅ Service is healthy!" || echo "❌ Service health check failed!"

echo "🎉 Deployment completed!"
EOF

chmod +x deploy-fedora.sh
```

## 📋 Checklist de Deploy

- [ ] ✅ Servidor Fedora atualizado
- [ ] ✅ Dependências instaladas (Node.js, C++, CMake)
- [ ] ✅ Projeto clonado em `/opt/rdws_webserver`
- [ ] ✅ Microserviços compilados
- [ ] ✅ Firewall configurado (porta 8080)
- [ ] ✅ Serviço configurado (PM2/Docker/Systemd)
- [ ] ✅ Logs configurados
- [ ] ✅ Health check funcionando
- [ ] ✅ Acesso externo testado

## 🆘 Solução de Problemas

### Problema: Microserviços não compilam
```bash
# Verificar dependências
sudo dnf install gcc-c++ cmake make

# Limpar e recompilar
rm -rf build
mkdir build && cd build
cmake .. && make VERBOSE=1
```

### Problema: Porta 8080 não acessível
```bash
# Verificar se está ouvindo
sudo netstat -tlnp | grep :8080

# Verificar firewall
sudo firewall-cmd --list-ports
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload
```

### Problema: PM2 não inicia
```bash
# Verificar logs
pm2 logs api-gateway --err

# Reiniciar PM2
pm2 kill
pm2 resurrect
```

O deploy no Fedora Server está pronto! Escolha o método que preferir (PM2, Docker ou Systemd) e siga o guia. 🚀