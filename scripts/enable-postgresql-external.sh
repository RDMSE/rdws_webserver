#!/bin/bash
# Enable PostgreSQL External Connections
# Allows DBeaver and other external clients to connect

echo "Enabling PostgreSQL External Connections for DBeaver"
echo "======================================================"

# Backup current config
sudo cp /var/lib/pgsql/data/postgresql.conf /var/lib/pgsql/data/postgresql.conf.backup
sudo cp /var/lib/pgsql/data/pg_hba.conf /var/lib/pgsql/data/pg_hba.conf.backup

# Enable listening on all addresses
echo "Configuring postgresql.conf..."
sudo sed -i "s/#listen_addresses = 'localhost'/listen_addresses = '*'/" /var/lib/pgsql/data/postgresql.conf
sudo sed -i "s/listen_addresses = 'localhost'/listen_addresses = '*'/" /var/lib/pgsql/data/postgresql.conf

# Add external connection rule to pg_hba.conf
echo "Adding external access rule..."
sudo tee -a /var/lib/pgsql/data/pg_hba.conf > /dev/null << 'EOF'

# Allow external connections for rdws_user
host    rdws_development    rdws_user       0.0.0.0/0               md5
host    rdws_production     rdws_user       0.0.0.0/0               md5
EOF

# Configure firewall
echo "Configuring firewall..."
sudo firewall-cmd --permanent --add-port=5432/tcp
sudo firewall-cmd --reload

# Restart PostgreSQL
echo "Restarting PostgreSQL..."
sudo systemctl restart postgresql

# Show status
echo "PostgreSQL Status:"
sudo systemctl status postgresql --no-pager

echo ""
echo "External connections enabled!"
echo ""
echo "DBeaver Connection Info:"
echo "  Host: fedora-server.local (or IP address)"
echo "  Port: 5432"
echo "  Database: rdws_production"
echo "  Username: rdws_user"
echo "  Password: rdws_pass123"
echo ""
echo "Test connection:"
echo "  nmap -p 5432 fedora-server.local"
