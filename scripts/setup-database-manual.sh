#!/bin/bash
# Manual Database Setup
# For setting up database locally or on server

set -e

ENVIRONMENT=${1:-development}

echo "Manual Database Setup"
echo "Environment: $ENVIRONMENT"
echo ""

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
echo "Database: rdws_$ENVIRONMENT"
echo "User: rdws_user"
echo "Password: rdws_pass123"
echo ""
echo "Connection string:"
echo "postgresql://rdws_user:rdws_pass123@localhost:5432/rdws_$ENVIRONMENT"
