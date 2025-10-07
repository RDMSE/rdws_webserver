#!/bin/bash
# PostgreSQL Installation & Setup Script for Fedora Server
# This script installs PostgreSQL and sets up dev/prod databases

set -e

echo "Setting up PostgreSQL on Fedora Server..."

# Install PostgreSQL
echo "Installing PostgreSQL..."
sudo dnf install -y postgresql postgresql-server postgresql-contrib

# Initialize database (only if not already initialized)
if [ ! -d "/var/lib/pgsql/data/base" ]; then
    echo "Initializing PostgreSQL database..."
    sudo postgresql-setup --initdb
fi

# Start and enable PostgreSQL
echo "Starting PostgreSQL service..."
sudo systemctl start postgresql
sudo systemctl enable postgresql

# Configure PostgreSQL for remote connections (if needed)
echo "⚙️ Configuring PostgreSQL..."
sudo -u postgres psql -c "ALTER USER postgres PASSWORD 'postgres123';"

# Configure authentication to allow password authentication
echo "🔐 Configuring authentication..."
sudo sed -i "s/#listen_addresses = 'localhost'/listen_addresses = 'localhost'/" /var/lib/pgsql/data/postgresql.conf
sudo sed -i "s/ident$/md5/" /var/lib/pgsql/data/pg_hba.conf
sudo sed -i "s/peer$/md5/" /var/lib/pgsql/data/pg_hba.conf

# Restart PostgreSQL to apply configuration changes
echo "Restarting PostgreSQL..."
sudo systemctl restart postgresql

# Wait a moment for restart
sleep 3

# Create application databases
echo "Creating application databases..."
sudo -u postgres createdb rdws_development 2>/dev/null || echo "Development DB already exists"
sudo -u postgres createdb rdws_production 2>/dev/null || echo "Production DB already exists"

# Create application user
echo "Creating application user..."
sudo -u postgres psql -c "CREATE USER rdws_user WITH PASSWORD 'rdws_pass123';" 2>/dev/null || echo "User already exists"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rdws_development TO rdws_user;"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rdws_production TO rdws_user;"

# Grant schema permissions (PostgreSQL 15+ requirement)
echo "Granting schema permissions..."
sudo -u postgres psql -d rdws_development -c "GRANT ALL ON SCHEMA public TO rdws_user;"
sudo -u postgres psql -d rdws_development -c "GRANT CREATE ON SCHEMA public TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "GRANT ALL ON SCHEMA public TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "GRANT CREATE ON SCHEMA public TO rdws_user;"

# Set default privileges for future objects
echo "🛡️ Setting default privileges..."
sudo -u postgres psql -d rdws_development -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;"
sudo -u postgres psql -d rdws_development -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;"

# Show status
echo "PostgreSQL setup complete!"
sudo systemctl status postgresql --no-pager
echo ""
echo "Databases created:"
sudo -u postgres psql -l | grep rdws
echo ""
echo "Connection info:"
echo "  Host: localhost"
echo "  User: rdws_user"
echo "  Password: rdws_pass123"
echo "  Databases: rdws_development, rdws_production"
echo "  Port: 5432"
