#!/bin/bash
# Setup PostgreSQL with persistent external access configuration
# This script preserves external access settings across deploys

set -e

echo "Setting up PostgreSQL with persistent external access..."

# PostgreSQL data directory
PG_DATA_DIR="/var/lib/pgsql/data"

# Backup current pg_hba.conf
sudo cp $PG_DATA_DIR/pg_hba.conf $PG_DATA_DIR/pg_hba.conf.backup.$(date +%Y%m%d_%H%M%S)

# Create a complete new pg_hba.conf with all necessary rules
cat << 'EOF' | sudo tee $PG_DATA_DIR/pg_hba.conf > /dev/null
# TYPE  DATABASE        USER            ADDRESS                 METHOD

# "local" is for Unix domain socket connections only
local   all             postgres                                trust
local   all             all                                     md5

# IPv4 local connections:
host    all             all             127.0.0.1/32            md5

# IPv6 local connections:
host    all             all             ::1/128                 md5

# External access for DBeaver and remote clients
host    rdws_development    rdws_user       0.0.0.0/0               md5
host    rdws_production     rdws_user       0.0.0.0/0               md5

# Allow replication connections from localhost, by a user with the
# replication privilege.
local   replication     all                                     peer
host    replication     all             127.0.0.1/32            ident
host    replication     all             ::1/128                 ident
EOF

echo "pg_hba.conf configured with persistent external access"

# Configure postgresql.conf for external connections
if ! grep -q "listen_addresses = '\*'" $PG_DATA_DIR/postgresql.conf; then
    sudo sed -i "s/#listen_addresses = 'localhost'/listen_addresses = '*'/" $PG_DATA_DIR/postgresql.conf
    sudo sed -i "s/listen_addresses = 'localhost'/listen_addresses = '*'/" $PG_DATA_DIR/postgresql.conf
    echo "postgresql.conf configured for external connections"
fi

# Configure firewall
if command -v firewall-cmd &> /dev/null; then
    sudo firewall-cmd --permanent --add-port=5432/tcp 2>/dev/null && echo "Firewall rule added for port 5432" || echo "Port 5432 already open"
    sudo firewall-cmd --reload 2>/dev/null && echo "Firewall reloaded" || echo "Firewall reload failed"
fi

echo "Restarting PostgreSQL to apply changes..."
sudo systemctl restart postgresql

echo "PostgreSQL configured with persistent external access!"
echo ""
echo "Connection details for DBeaver:"
echo "   Host: fedora-server.local (or server IP)"
echo "   Port: 5432"
echo "   Database: rdws_development or rdws_production"
echo "   Username: rdws_user"
echo "   Password: rdws_password"
