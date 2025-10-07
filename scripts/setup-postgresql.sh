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
echo "Configuring PostgreSQL..."
sudo -u postgres psql -c "ALTER USER postgres PASSWORD 'postgres123';"

# Create application databases
echo "Creating application databases..."
sudo -u postgres createdb rdws_development 2>/dev/null || echo "Development DB already exists"
sudo -u postgres createdb rdws_production 2>/dev/null || echo "Production DB already exists"
sudo -u postgres createdb rdws_test 2>/dev/null || echo "Test DB already exists"

# Create application user
echo "Creating application user..."
sudo -u postgres psql -c "CREATE USER rdws_user WITH PASSWORD 'rdws_pass123';" 2>/dev/null || echo "User already exists"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rdws_development TO rdws_user;"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rdws_production TO rdws_user;"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rdws_test TO rdws_user;"

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
echo "  Databases: rdws_development, rdws_production, rdws_test"
echo "  Port: 5432"
