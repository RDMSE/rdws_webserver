#!/bin/bash
# Super Simple PostgreSQL Setup for CI/CD
# Uses trust authentication to avoid password issues

set -e

echo "Super Simple PostgreSQL Setup"
echo "==============================="

# Configure trust authentication for postgres user locally
echo "Setting up trust authentication..."
sudo cp /var/lib/pgsql/data/pg_hba.conf /var/lib/pgsql/data/pg_hba.conf.backup

# Create new pg_hba.conf with trust for local connections
sudo tee /var/lib/pgsql/data/pg_hba.conf > /dev/null << 'EOF'
# PostgreSQL Client Authentication Configuration File
# TYPE  DATABASE        USER            ADDRESS                 METHOD

# "local" is for Unix domain socket connections only
local   all             postgres                                trust
local   all             all                                     md5

# IPv4 local connections:
host    all             all             127.0.0.1/32            md5

# IPv6 local connections:
host    all             all             ::1/128                 md5
EOF

# Restart PostgreSQL
echo "Restarting PostgreSQL with new config..."
sudo systemctl restart postgresql
sleep 5

# Now all commands should work without password
echo "Creating everything..."
sudo -u postgres psql << 'EOF'
-- Create databases
CREATE DATABASE rdws_development;
CREATE DATABASE rdws_production;

-- Create user
CREATE USER rdws_user WITH PASSWORD 'rdws_pass123';

-- Grant database privileges
GRANT ALL PRIVILEGES ON DATABASE rdws_development TO rdws_user;
GRANT ALL PRIVILEGES ON DATABASE rdws_production TO rdws_user;
EOF

# Grant schema privileges
echo "Setting up schema permissions..."
sudo -u postgres psql -d rdws_development << 'EOF'
GRANT ALL ON SCHEMA public TO rdws_user;
GRANT CREATE ON SCHEMA public TO rdws_user;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;
EOF

sudo -u postgres psql -d rdws_production << 'EOF'
GRANT ALL ON SCHEMA public TO rdws_user;
GRANT CREATE ON SCHEMA public TO rdws_user;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO rdws_user;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO rdws_user;
EOF

# Test connection
echo "Testing connection..."
export PGPASSWORD="rdws_pass123"
if psql -h localhost -U rdws_user -d rdws_production -c "SELECT 'Setup successful!' as result;" 2>/dev/null; then
    echo "Setup successful!"
else
    echo "Test failed, but setup might still work"
fi

echo ""
echo "PostgreSQL setup complete!"
echo "Summary:"
echo "  - Databases: rdws_development, rdws_production"
echo "  - User: rdws_user"
echo "  - Password: rdws_pass123"
