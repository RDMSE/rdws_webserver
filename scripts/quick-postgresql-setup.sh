#!/bin/bash
# Quick PostgreSQL Setup for CI/CD
# Simple, fast setup that doesn't hang

set -e

echo "⚡ Quick PostgreSQL Setup for CI/CD"
echo "==================================="

# Restart PostgreSQL to ensure clean state
echo "Restarting PostgreSQL..."
sudo systemctl restart postgresql
sleep 3

# Create databases with explicit error handling
echo "Creating databases..."
echo "Creating rdws_development..."
if sudo -u postgres psql -c "SELECT 1 FROM pg_database WHERE datname='rdws_development';" | grep -q 1; then
    echo "rdws_development already exists"
else
    sudo -u postgres psql -c "CREATE DATABASE rdws_development;"
    echo "rdws_development created"
fi

echo "Creating rdws_production..."
if sudo -u postgres psql -c "SELECT 1 FROM pg_database WHERE datname='rdws_production';" | grep -q 1; then
    echo "rdws_production already exists"
else
    sudo -u postgres psql -c "CREATE DATABASE rdws_production;"
    echo "rdws_production created"
fi

# Create user
echo "Creating user..."
sudo -u postgres psql -c "DO \$\$ BEGIN CREATE USER rdws_user WITH PASSWORD 'rdws_pass123'; EXCEPTION WHEN duplicate_object THEN RAISE NOTICE 'User already exists'; END \$\$;"

# Grant database privileges
echo "Granting database privileges..."
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rdws_development TO rdws_user;"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rdws_production TO rdws_user;"

# Grant schema privileges
echo "Granting schema privileges..."
sudo -u postgres psql -d rdws_development -c "GRANT ALL ON SCHEMA public TO rdws_user;"
sudo -u postgres psql -d rdws_development -c "GRANT CREATE ON SCHEMA public TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "GRANT ALL ON SCHEMA public TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "GRANT CREATE ON SCHEMA public TO rdws_user;"

# Set default privileges
echo "Setting default privileges..."
sudo -u postgres psql -d rdws_development -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;"
sudo -u postgres psql -d rdws_development -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;"

# Quick test
echo "Quick permission test..."
export PGPASSWORD="rdws_pass123"
if psql -h localhost -U rdws_user -d rdws_production -c "SELECT current_database();" > /dev/null 2>&1; then
    echo "Connection test: OK"
else
    echo "Connection test: FAILED"
fi

echo "Quick setup complete!"
