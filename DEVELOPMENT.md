# 🏠➡️🖥️ Local Development + Remote Deploy Guide

## Visão Geral

Este guia explica como desenvolver localmente no Linux Mint e fazer deploy no servidor Fedora remoto.

## 📋 Pré-requisitos

### Na sua máquina local (Linux Mint):
- VS Code instalado
- Git instalado
- SSH configurado para o servidor
- Docker (opcional, mas recomendado)

### No servidor remoto (Fedora):
- Projeto já configurado (como está agora)
- SSH server rodando
- Suas chaves SSH configuradas

## 🚀 Setup Inicial

### 1. Copiar projeto para máquina local

Agora que o shell está configurado corretamente, você pode usar métodos padrão:

**Método Recomendado - SCP**:
```bash
# Na sua máquina Linux Mint
scp -r rdias@fedora-server.local:/home/rdias/sources/lab/server ~/cpp-projects/
cd ~/cpp-projects/server
```

**Alternativa - Rsync** (mais rápido para atualizações):
```bash
# Sincronização com rsync
rsync -avz --exclude='build/' --exclude='.git/' \
    rdias@fedora-server.local:/home/rdias/sources/lab/server/ \
    ~/cpp-projects/server/
cd ~/cpp-projects/server
```

**Para backup/pacote** (opcional):
```bash
# No servidor, criar pacote com timestamp
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && ./scripts/create_package.sh"

# Baixar o pacote
scp rdias@fedora-server.local:/tmp/cpp-rest-server-*.tar.gz ~/cpp-projects/
cd ~/cpp-projects && tar xzf cpp-rest-server-*.tar.gz
```

### 2. Configurar VS Code

```bash
# Copiar configurações para desenvolvimento local
cp .vscode/settings-local-dev.json .vscode/settings.json
cp .vscode/tasks-local-dev.json .vscode/tasks.json
```

Edite `.vscode/settings.json` e ajuste:
```json
{
    "remote.host": "fedora-server.local",  // ou IP do seu servidor
    "remote.user": "rdias",                // seu usuário no servidor
    "remote.path": "/home/rdias/sources/lab/server"
}
```

### 3. Configurar SSH (se não feito)

```bash
# Gerar chave SSH (se não tiver)
ssh-keygen -t rsa -b 4096 -C "seu_email@exemplo.com"

# Copiar chave para servidor
ssh-copy-id rdias@fedora-server.local

# Testar conexão
ssh rdias@fedora-server.local "echo 'SSH funcionando!'"
```

## 💻 Opções de Desenvolvimento Local

### Opção 1: Docker (Recomendado)

**Vantagens:**
- Ambiente idêntico ao servidor
- Não precisa instalar dependências localmente
- Isolamento completo

```bash
# Iniciar ambiente de desenvolvimento
docker-compose up -d

# Entrar no container
docker-compose exec cpp-dev bash

# Dentro do container, você pode:
mkdir build && cd build
PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig cmake ..
make
```

### Opção 2: Instalação Nativa Linux Mint

```bash
# Instalar dependências
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git curl
sudo apt install -y libcurl4-openssl-dev rapidjson-dev
sudo apt install -y libgtest-dev libgmock-dev

# Compilar Pistache (se não estiver nos repos)
git clone https://github.com/pistacheio/pistache.git /tmp/pistache
cd /tmp/pistache
meson setup build --buildtype=release
meson compile -C build
sudo meson install -C build
sudo ldconfig
```

## 🔄 Workflow de Desenvolvimento

### VS Code Tasks (Ctrl+Shift+P → "Tasks: Run Task")

1. **🔄 Full Deploy & Run**
   - Deploy código → Build remoto → Testa → Executa servidor
   - Use este para deployment completo

2. **⚡ Quick Deploy & Build**  
   - Deploy código → Build remoto
   - Use durante desenvolvimento

3. **🚀 Deploy to Server**
   - Apenas sincroniza arquivos
   - Rápido para mudanças pequenas

4. **🧪 Remote Test**
   - Executa testes no servidor
   - Unitários + Integração

5. **🛑 Remote Stop**
   - Para o servidor remoto

6. **📊 Remote Status**
   - Verifica se servidor está rodando

7. **📋 View Remote Logs**
   - Visualiza logs do servidor

### Workflow Típico de Desenvolvimento

```bash
# 1. Editar código localmente no VS Code
# 2. Salvar arquivos
# 3. Executar task "⚡ Quick Deploy & Build"
# 4. Se tudo OK, executar "🧪 Remote Test"
# 5. Para produção, executar "🔄 Full Deploy & Run"
```

## 📁 Estrutura de Arquivos

```
server/
├── scripts/
│   ├── deploy.sh           # Script de deploy
│   ├── remote_build.sh     # Build no servidor
│   ├── remote_test.sh      # Testes no servidor  
│   ├── remote_run.sh       # Execução no servidor
│   └── local_setup.sh      # Setup desenvolvimento local
├── .vscode/
│   ├── tasks-local-dev.json    # Tasks para dev local
│   └── settings-local-dev.json # Configurações para dev local
├── .deployignore          # Arquivos para não enviar no deploy
├── Dockerfile              # Container para dev local
└── docker-compose.yml      # Orquestração Docker
```

## 🛠️ Scripts Disponíveis

### Local (sua máquina):
```bash
./scripts/deploy.sh                    # Deploy para servidor
./scripts/local_setup.sh               # Setup ambiente local
./scripts/local_setup.sh --docker      # Setup com Docker
```

### Remoto (servidor):
```bash
./scripts/remote_build.sh              # Compilar projeto
./scripts/remote_test.sh               # Executar todos os testes
./scripts/remote_test.sh --report      # Testes + relatórios XML
./scripts/remote_run.sh                # Executar servidor (foreground)
./scripts/remote_run.sh --background   # Executar servidor (background)
./scripts/remote_run.sh --stop         # Parar servidor
./scripts/remote_run.sh --status       # Status do servidor
```

## 🔧 Comandos SSH Manuais

```bash
# Deploy manual
rsync -avz --exclude-from=.deployignore ./ rdias@fedora-server.local:/home/rdias/sources/lab/server/

# Build remoto
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && ./scripts/remote_build.sh"

# Testes remotos
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && ./scripts/remote_test.sh"

# Executar servidor
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && ./scripts/remote_run.sh --background"

# Ver logs
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && tail -f server.log"
```

## 🎯 Dicas de Desenvolvimento

### Desenvolvimento Eficiente:
1. **Use Docker** para consistência de ambiente
2. **Configure SSH keys** para evitar digitar senhas
3. **Use VS Code tasks** ao invés de comandos manuais
4. **Teste localmente** quando possível (Docker)
5. **Deploy frequentemente** com "Quick Deploy & Build"

### Debugging:
1. **Logs do servidor**: Task "📋 View Remote Logs"
2. **Status do servidor**: Task "📊 Remote Status"  
3. **Rebuild completo**: Task "🔄 Full Deploy & Run"
4. **Testes isolados**: SSH manual + `./scripts/remote_test.sh`

### Estrutura de Deploy:
- `.deployignore` controla o que NÃO é enviado
- `rsync` mantém sincronização eficiente
- Scripts remotos são executados via SSH

## 🚨 Troubleshooting

### Erro de SSH/SCP: "Received message too long" ou "shell produces output"

**Causa**: Seu `.bashrc` ou `.bash_profile` está gerando saída em sessões não-interativas.

**Culpados comuns**: `neofetch`, `fortune`, `cowsay`, `figlet`, `motd`, `echo` statements.

**Solução permanente** (recomendada):
```bash
# Edite ~/.bashrc no servidor e mude:
# neofetch                    # ❌ Executa sempre
# Para:
[[ $- == *i* ]] && neofetch   # ✅ Só em sessões interativas

# Ou use if:
if [[ $- == *i* ]]; then
    neofetch
    fortune
    # outros comandos que geram saída
fi
```

**Soluções temporárias** (se não puder editar .bashrc):

1. **Use o método de pacote**:
   ```bash
   ssh rdias@fedora-server.local "./scripts/create_package.sh"
   scp rdias@fedora-server.local:/tmp/cpp-rest-server-*.tar.gz ~/
   ```

2. **Temporariamente renomeie .bashrc**:
   ```bash
   ssh rdias@fedora-server.local "mv ~/.bashrc ~/.bashrc.tmp"
   # Faça a cópia normalmente
   ssh rdias@fedora-server.local "mv ~/.bashrc.tmp ~/.bashrc"
   ```

### Erro de SSH: "Permission denied" ou "Host key verification failed"

```bash
# Configurar chaves SSH
ssh-keygen -t rsa -b 4096 -C "seu_email@exemplo.com"
ssh-copy-id rdias@fedora-server.local

# Testar conexão
ssh rdias@fedora-server.local "echo 'SSH OK'"
```

### Erro de Deploy: "rsync command not found" ou "connection refused"

```bash
# Verificar conectividade
ping fedora-server.local

# Testar SSH verbose
ssh -v rdias@fedora-server.local

# Usar IP ao invés de hostname
ssh rdias@10.0.0.32  # substitua pelo IP do seu servidor
```

Este setup permite desenvolvimento ágil local com deploy simples para produção! 🚀