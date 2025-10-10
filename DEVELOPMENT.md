# Local Development + Remote Deploy Guide

## Overview

This guide explains how to develop locally on Linux Mint and deploy to the remote Fedora server.

## Prerequisites

### On your local machine (Linux Mint):
- VS Code installed
- Git installed
- SSH configured for the server
- Docker (optional, but recommended)

### On remote server (Fedora):
- Project already configured (as it is now)
- SSH server running
- Your SSH keys configured

## Initial Setup

### 1. Copy project to local machine

Now that the shell is properly configured, you can use standard methods:

**Recommended Method - SCP**:
```bash
# On your Linux Mint machine
scp -r rdias@fedora-server.local:/home/rdias/sources/lab/server ~/cpp-projects/
cd ~/cpp-projects/server
```

**Alternative - Rsync** (faster for updates):
```bash
# Synchronization with rsync
rsync -avz --exclude='build/' --exclude='.git/' \
    rdias@fedora-server.local:/home/rdias/sources/lab/server/ \
    ~/cpp-projects/server/
cd ~/cpp-projects/server
```

**For backup/package** (optional):
```bash
# On server, create timestamped package
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && ./scripts/create_package.sh"

# Download the package
scp rdias@fedora-server.local:/tmp/cpp-rest-server-*.tar.gz ~/cpp-projects/
cd ~/cpp-projects && tar xzf cpp-rest-server-*.tar.gz
```

### 2. Configure VS Code

```bash
# Copy settings for local development
cp .vscode/settings-local-dev.json .vscode/settings.json
cp .vscode/tasks-local-dev.json .vscode/tasks.json
```

Edit `.vscode/settings.json` and adjust:
```json
{
    "remote.host": "fedora-server.local",  // or your server IP
    "remote.user": "rdias",                // your username on server
    "remote.path": "/home/rdias/sources/lab/server"
}
```

### 3. Configure SSH (if not done)

```bash
# Generate SSH key (if you don't have one)
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"

# Copy key to server
ssh-copy-id rdias@fedora-server.local

# Test connection
ssh rdias@fedora-server.local "echo 'SSH working!'"
```

## Local Development Options

### Option 1: Docker (Recommended)

**Advantages:**
- Environment identical to server
- No need to install dependencies locally
- Complete isolation

```bash
# Start development environment
docker-compose up -d

# Enter the container
docker-compose exec cpp-dev bash

# Inside the container, you can:
mkdir build && cd build
PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig cmake ..
make
```

### Option 2: Native Linux Mint Installation

```bash
# Install dependencies
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git curl
sudo apt install -y libcurl4-openssl-dev rapidjson-dev
sudo apt install -y libgtest-dev libgmock-dev

meson compile -C build
sudo meson install -C build
sudo ldconfig
```

## Development Workflow

### VS Code Tasks (Ctrl+Shift+P → "Tasks: Run Task")

1. **Full Deploy & Run**
   - Deploy code → Remote build → Test → Run server
   - Use this for complete deployment

2. **Quick Deploy & Build**
   - Deploy code → Remote build
   - Use during development

3. **Deploy to Server**
   - Only synchronizes files
   - Fast for small changes

4. **Remote Test**
   - Run tests on server
   - Unit + Integration

5. **Remote Stop**
   - Stop remote server

6. **Remote Status**
   - Check if server is running

7. **View Remote Logs**
   - View server logs

### Typical Development Workflow

```bash
# 1. Edit code locally in VS Code
# 2. Save files
# 3. Run task "Quick Deploy & Build"
# 4. If everything OK, run "Remote Test"
# 5. For production, run "Full Deploy & Run"
```

## File Structure

```
server/
├── scripts/
│   ├── deploy.sh           # Deploy script
│   ├── remote_build.sh     # Build on server
│   ├── remote_test.sh      # Tests on server
│   ├── remote_run.sh       # Execution on server
│   └── local_setup.sh      # Local development setup
├── .vscode/
│   ├── tasks-local-dev.json    # Tasks for local dev
│   └── settings-local-dev.json # Settings for local dev
├── .deployignore          # Files not to send on deploy
├── Dockerfile              # Container for local dev
└── docker-compose.yml      # Docker orchestration
```

## Available Scripts

### Local (your machine):
```bash
./scripts/deploy.sh                    # Deploy to server
./scripts/local_setup.sh               # Setup local environment
./scripts/local_setup.sh --docker      # Setup with Docker
```

### Remote (server):
```bash
./scripts/remote_build.sh              # Compile project
./scripts/remote_test.sh               # Run all tests
./scripts/remote_test.sh --report      # Tests + XML reports
./scripts/remote_run.sh                # Run server (foreground)
./scripts/remote_run.sh --background   # Run server (background)
./scripts/remote_run.sh --stop         # Stop server
./scripts/remote_run.sh --status       # Server status
```

## Manual SSH Commands

```bash
# Manual deploy
rsync -avz --exclude-from=.deployignore ./ rdias@fedora-server.local:/home/rdias/sources/lab/server/

# Remote build
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && ./scripts/remote_build.sh"

# Remote tests
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && ./scripts/remote_test.sh"

# Run server
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && ./scripts/remote_run.sh --background"

# View logs
ssh rdias@fedora-server.local "cd /home/rdias/sources/lab/server && tail -f server.log"
```

## Development Tips

### Efficient Development:
1. **Use Docker** for environment consistency
2. **Configure SSH keys** to avoid typing passwords
3. **Use VS Code tasks** instead of manual commands
4. **Test locally** when possible (Docker)
5. **Deploy frequently** with "Quick Deploy & Build"

### Debugging:
1. **Server logs**: Task "View Remote Logs"
2. **Server status**: Task "Remote Status"
3. **Complete rebuild**: Task "Full Deploy & Run"
4. **Isolated tests**: Manual SSH + `./scripts/remote_test.sh`

### Deploy Structure:
- `.deployignore` controls what is NOT sent
- `rsync` maintains efficient synchronization
- Remote scripts are executed via SSH

## Troubleshooting

### SSH/SCP Error: "Received message too long" or "shell produces output"

**Cause**: Your `.bashrc` or `.bash_profile` is generating output in non-interactive sessions.

**Common culprits**: `neofetch`, `fortune`, `cowsay`, `figlet`, `motd`, `echo` statements.

**Permanent solution** (recommended):
```bash
# Edit ~/.bashrc on server and change:
# neofetch                    # Always executes
# To:
[[ $- == *i* ]] && neofetch   # Only in interactive sessions

# Or use if:
if [[ $- == *i* ]]; then
    neofetch
    fortune
    # other commands that generate output
fi
```

**Temporary solutions** (if you can't edit .bashrc):

1. **Use the package method**:
   ```bash
   ssh rdias@fedora-server.local "./scripts/create_package.sh"
   scp rdias@fedora-server.local:/tmp/cpp-rest-server-*.tar.gz ~/
   ```

2. **Temporarily rename .bashrc**:
   ```bash
   ssh rdias@fedora-server.local "mv ~/.bashrc ~/.bashrc.tmp"
   # Do the copy normally
   ssh rdias@fedora-server.local "mv ~/.bashrc.tmp ~/.bashrc"
   ```

### SSH Error: "Permission denied" or "Host key verification failed"

```bash
# Configure SSH keys
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
ssh-copy-id rdias@fedora-server.local

# Test connection
ssh rdias@fedora-server.local "echo 'SSH OK'"
```

### Deploy Error: "rsync command not found" or "connection refused"

```bash
# Check connectivity
ping fedora-server.local

# Test SSH verbose
ssh -v rdias@fedora-server.local

# Use IP instead of hostname
ssh rdias@10.0.0.32  # replace with your server IP
```

This setup allows agile local development with simple deployment to production!
