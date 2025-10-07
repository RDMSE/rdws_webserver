#!/bin/bash
# Complete PostgreSQL Setup Script
# Ensures PostgreSQL is fully configured with databases and permissions

set -e

echo "Complete PostgreSQL Setup"
echo "============================"

# Check PostgreSQL status
echo "Checking PostgreSQL status..."
if ! sudo systemctl is-active --quiet postgresql; then
    echo "Starting PostgreSQL..."
    sudo systemctl start postgresql
    sleep 2
fi

# Create databases if they don't exist (with timeout)
echo "Creating databases..."
timeout 30 sudo -u postgres createdb rdws_development 2>/dev/null || echo "Development DB already exists or creation failed"
timeout 30 sudo -u postgres createdb rdws_production 2>/dev/null || echo "Production DB already exists or creation failed"

# Verify databases exist
echo "Verifying databases..."
if sudo -u postgres psql -lqt | cut -d \| -f 1 | grep -qw rdws_development; then
    echo "rdws_development exists"
else
    echo "rdws_development not found"
fi

if sudo -u postgres psql -lqt | cut -d \| -f 1 | grep -qw rdws_production; then
    echo "rdws_production exists"
else
    echo "rdws_production not found"
fi

# Create user if doesn't exist and grant database privileges
echo "Setting up user and database privileges..."
sudo -u postgres psql -c "CREATE USER rdws_user WITH PASSWORD 'rdws_pass123';" 2>/dev/null || echo "User already exists"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rdws_development TO rdws_user;"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rdws_production TO rdws_user;"

# Grant schema permissions (PostgreSQL 15+ requirement)
echo "Setting schema permissions..."
sudo -u postgres psql -d rdws_development -c "GRANT ALL ON SCHEMA public TO rdws_user;"
sudo -u postgres psql -d rdws_development -c "GRANT CREATE ON SCHEMA public TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "GRANT ALL ON SCHEMA public TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "GRANT CREATE ON SCHEMA public TO rdws_user;"

# Set default privileges for future objects
echo "Setting default privileges for future objects..."
sudo -u postgres psql -d rdws_development -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;"
sudo -u postgres psql -d rdws_development -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;"
sudo -u postgres psql -d rdws_production -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;"

# Test permissions by creating and dropping a test table
echo "Testing permissions..."
export PGPASSWORD="rdws_pass123"
psql -h localhost -U rdws_user -d rdws_production -c "
CREATE TABLE IF NOT EXISTS permission_test (
    id SERIAL PRIMARY KEY,
    test_column VARCHAR(50)
);
DROP TABLE IF EXISTS permission_test;
" && echo "CREATE TABLE permission: OK" || echo "CREATE TABLE permission: FAILED"

# Show final status
echo ""
echo "Final status:"
echo "Databases:"
sudo -u postgres psql -l | grep rdws
echo ""
echo "User permissions:"
sudo -u postgres psql -c "\du rdws_user"

echo ""
echo "PostgreSQL setup complete!"
echo "Connection info:"
echo "  Host: localhost"
echo "  Port: 5432"
echo "  User: rdws_user"
echo "  Password: rdws_pass123"
echo "  Databases: rdws_development, rdws_production"
