#!/bin/bash
# Generate C++ Configuration Files
# Called by deploy scripts to create config files for microservices

set -e

ENVIRONMENT=${1:-production}
CONFIG_DIR=${2:-/opt/rdws_webserver/config}

echo "Generating C++ microservices configuration..."
echo "Environment: $ENVIRONMENT"
echo "Config directory: $CONFIG_DIR"

# Load environment variables
source "$(dirname "$0")/load_env.sh" "$ENVIRONMENT"

# Create config directory
mkdir -p "$CONFIG_DIR"

# Generate database configuration JSON
cat > "$CONFIG_DIR/database.json" << EOF
{
  "environment": "$ENVIRONMENT",
  "host": "$DB_HOST",
  "port": $DB_PORT,
  "user": "$DB_USER",
  "password": "$DB_PASS",
  "database": "$DB_NAME",
  "connection_string": "postgresql://$DB_USER:$DB_PASS@$DB_HOST:$DB_PORT/$DB_NAME"
}
EOF

# Set proper permissions
chmod 600 "$CONFIG_DIR/database.json"

echo "Configuration files generated successfully!"
echo "Database config: $CONFIG_DIR/database.json"

# Verify configuration
if [ -f "$CONFIG_DIR/database.json" ]; then
    echo "Database configuration created"
    echo "Preview (password hidden):"
    sed 's/"password": "[^"]*"/"password": "***"/' "$CONFIG_DIR/database.json"
else
    echo "Failed to create database configuration"
    exit 1
fi