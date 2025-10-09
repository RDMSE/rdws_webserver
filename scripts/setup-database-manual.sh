#!/bin/bash
# Manual Database Setup
# For setting up database locally or on server

set -e

ENVIRONMENT=${1:-development}

echo "Manual Database Setup"
echo "Environment: $ENVIRONMENT"
echo ""

# Load environment variables to show connection info
source "$(dirname "$0")/load_env.sh" "$ENVIRONMENT"

# Check if PostgreSQL is installed
if ! command -v psql &> /dev/null; then
    echo "PostgreSQL not found!"
    echo "Run: ./scripts/setup-postgresql.sh"
    exit 1
fi

echo "PostgreSQL found"

# Check if PostgreSQL is running
if ! sudo systemctl is-active --quiet postgresql; then
    echo "Starting PostgreSQL..."
    sudo systemctl start postgresql
fi

echo "PostgreSQL is running"

# Run migrations
echo "Installing Node.js dependencies..."
npm install

echo "Running migrations for $ENVIRONMENT..."
./scripts/migrate-database.sh $ENVIRONMENT migrate

echo "Running seeds for $ENVIRONMENT..."
./scripts/migrate-database.sh $ENVIRONMENT seed

echo "Database status:"
./scripts/migrate-database.sh $ENVIRONMENT status

echo ""
echo "Database setup complete!"
echo "Environment: $ENVIRONMENT"
echo "Database: $DB_NAME"
echo "User: $DB_USER"
echo "Host: $DB_HOST"
echo "Port: $DB_PORT"
echo ""
echo "Connection string:"
echo "postgresql://$DB_USER:$DB_PASS@$DB_HOST:$DB_PORT/$DB_NAME"
