#!/bin/bash
# PostgreSQL Connection Test Script
# Tests database connectivity and troubleshoots common issues

set -e

echo "🔍 PostgreSQL Connection Test"
echo "=============================="

# Check if PostgreSQL is running
echo "📊 Checking PostgreSQL status..."
sudo systemctl status postgresql --no-pager || echo "❌ PostgreSQL not running"

# Check configuration files
echo ""
echo "🔧 Checking authentication configuration..."
echo "pg_hba.conf:"
sudo cat /var/lib/pgsql/data/pg_hba.conf | grep -E "(local|host)" | head -5

echo ""
echo "postgresql.conf (listen_addresses):"
sudo grep "listen_addresses" /var/lib/pgsql/data/postgresql.conf || echo "Not found"

# Test connections
echo ""
echo "🔐 Testing connections..."

# Test as postgres user (should work)
echo "Testing as postgres user:"
sudo -u postgres psql -c "SELECT version();" 2>/dev/null && echo "✅ Postgres user connection: OK" || echo "❌ Postgres user connection: FAILED"

# Test as rdws_user
echo ""
echo "Testing as rdws_user:"
export PGPASSWORD="rdws_pass123"
psql -h localhost -U rdws_user -d rdws_production -c "SELECT current_database();" 2>/dev/null && echo "✅ rdws_user connection: OK" || echo "❌ rdws_user connection: FAILED"

# Show databases
echo ""
echo "📋 Available databases:"
sudo -u postgres psql -l | grep rdws

# Show users
echo ""
echo "👥 Database users:"
sudo -u postgres psql -c "\du" | grep rdws

echo ""
echo "🏁 Connection test complete!"
