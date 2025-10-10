#!/bin/bash
# PostgreSQL Connection Test Script
# Tests database connectivity and troubleshoots common issues

set -e

ENVIRONMENT=${1:-development}

echo "PostgreSQL Connection Test"
echo "=============================="
echo "Environment: $ENVIRONMENT"

# Load environment variables
source "$(dirname "$0")/load_env.sh" "$ENVIRONMENT"

# Check if PostgreSQL is running
echo "Checking PostgreSQL status..."
sudo systemctl status postgresql --no-pager || echo "PostgreSQL not running"

# Check configuration files
echo ""
echo "Checking authentication configuration..."
echo "pg_hba.conf:"
sudo cat /var/lib/pgsql/data/pg_hba.conf | grep -E "(local|host)" | head -5

echo ""
echo "postgresql.conf (listen_addresses):"
sudo grep "listen_addresses" /var/lib/pgsql/data/postgresql.conf || echo "Not found"

# Test connections
echo ""
echo "Testing connections..."

# Test as postgres user (should work)
echo "Testing as postgres user:"
sudo -u postgres psql -c "SELECT version();" 2>/dev/null && echo "Postgres user connection: OK" || echo "Postgres user connection: FAILED"

# Test as rdws_user
echo ""
echo "Testing as $DB_USER:"
export PGPASSWORD="$DB_PASS"
psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -c "SELECT current_database();" 2>/dev/null && echo "✅ $DB_USER connection: OK" || echo "❌ $DB_USER connection: FAILED"

# Show databases
echo ""
echo "Available databases:"
sudo -u postgres psql -l | grep rdws

# Show users
echo ""
echo "Database users:"
sudo -u postgres psql -c "\du" | grep rdws

echo ""
echo "Connection test complete!"
