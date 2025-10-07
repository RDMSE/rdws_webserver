#!/bin/bash
# Test PostgreSQL Connection Script
# Run this locally to test remote PostgreSQL

echo "PostgreSQL Connection Test"
echo "============================="

SERVER="fedora-server.local"
DB_USER="rdws_user"
DB_PASS="rdws_pass123"
DB_NAME="rdws_production"

echo "Testing connection to $SERVER..."

# Test 1: SSH and check PostgreSQL status
echo ""
echo "Test 1: PostgreSQL Service Status"
ssh rdias@$SERVER "sudo systemctl status postgresql --no-pager" || echo "❌ SSH connection failed"

# Test 2: List databases via SSH
echo ""
echo "Test 2: List Databases"
ssh rdias@$SERVER "export PGPASSWORD='$DB_PASS' && psql -h localhost -U $DB_USER -l" || echo "❌ Database connection failed"

# Test 3: Test specific database connection
echo ""
echo "Test 3: Connect to Production Database"
ssh rdias@$SERVER "export PGPASSWORD='$DB_PASS' && psql -h localhost -U $DB_USER -d $DB_NAME -c 'SELECT current_database(), current_user, now();'" || echo "❌ Production DB connection failed"

# Test 4: Check if migrations table exists
echo ""
echo "Test 4: Check Migrations Table"
ssh rdias@$SERVER "export PGPASSWORD='$DB_PASS' && psql -h localhost -U $DB_USER -d $DB_NAME -c 'SELECT COUNT(*) as migration_count FROM migrations;'" 2>/dev/null && echo "✅ Migrations table exists" || echo "❌ Migrations table not found (expected if migrations haven't run)"

# Test 5: Test table creation permissions
echo ""
echo "Test 5: Test Table Creation"
ssh rdias@$SERVER "export PGPASSWORD='$DB_PASS' && psql -h localhost -U $DB_USER -d $DB_NAME -c 'CREATE TABLE test_permissions (id SERIAL, test_text VARCHAR(50)); DROP TABLE test_permissions;'" && echo "✅ CREATE/DROP permissions OK" || echo "❌ CREATE permissions failed"

# Test 6: Check port accessibility (optional)
echo ""
echo "Test 6: Port Accessibility"
if command -v nmap &> /dev/null; then
    nmap -p 5432 $SERVER | grep 5432 || echo "Port 5432 not accessible externally (this is normal/secure)"
else
    echo "ℹnmap not available, skipping port test"
fi

echo ""
echo "Test completed!"
echo ""
echo "Quick connect command:"
echo "ssh rdias@$SERVER"
echo "export PGPASSWORD='$DB_PASS'"
echo "psql -h localhost -U $DB_USER -d $DB_NAME"
