#!/bin/bash
# Fix PostgreSQL Permissions Script
# Fixes schema permissions for existing PostgreSQL installations

set -e

echo "Fixing PostgreSQL Permissions"
echo "================================="

# First, check if databases exist
echo "Checking if databases exist..."
sudo -u postgres psql -l | grep rdws || echo "No rdws databases found"

echo "Granting schema permissions to rdws_user..."

# Grant schema permissions for development database
echo "Development database permissions..."
if sudo -u postgres psql -d rdws_development -c "SELECT 1;" 2>/dev/null; then
    sudo -u postgres psql -d rdws_development -c "GRANT ALL ON SCHEMA public TO rdws_user;"
    sudo -u postgres psql -d rdws_development -c "GRANT CREATE ON SCHEMA public TO rdws_user;"
    echo "Development permissions applied"
else
    echo "Development database not found"
fi

# Grant schema permissions for production database
echo "Production database permissions..."
sudo -u postgres psql -d rdws_production -c "GRANT ALL ON SCHEMA public TO rdws_user;" 2>/dev/null || echo "Production DB not found"
sudo -u postgres psql -d rdws_production -c "GRANT CREATE ON SCHEMA public TO rdws_user;" 2>/dev/null || echo "Production DB not found"

# Set default privileges for future objects
echo "Setting default privileges..."
sudo -u postgres psql -d rdws_development -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;" 2>/dev/null || echo "Development DB not found"
sudo -u postgres psql -d rdws_development -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;" 2>/dev/null || echo "Development DB not found"
sudo -u postgres psql -d rdws_production -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;" 2>/dev/null || echo "Production DB not found"
sudo -u postgres psql -d rdws_production -c "ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;" 2>/dev/null || echo "Production DB not found"

# Test permissions
echo "Testing permissions..."
export PGPASSWORD="rdws_pass123"

echo "Testing CREATE TABLE permission in production:"
psql -h localhost -U rdws_user -d rdws_production -c "
CREATE TABLE IF NOT EXISTS permission_test (
    id SERIAL PRIMARY KEY,
    test_column VARCHAR(50)
);
DROP TABLE IF EXISTS permission_test;
" && echo "CREATE TABLE permission: OK" || echo "CREATE TABLE permission: FAILED"

echo ""
echo "Permission fix complete!"
