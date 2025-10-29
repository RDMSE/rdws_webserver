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
# Determine database name based on environment
if [ "$ENVIRONMENT" == "production" ]; then
    DB_NAME_TO_USE=${DB_NAME_PROD:-rdws_production}
elif [ "$ENVIRONMENT" == "development" ]; then
    DB_NAME_TO_USE=${DB_NAME_DEV:-rdws_development}
else
    DB_NAME_TO_USE=${DB_NAME:-rdws_development}
fi

echo "Using database: $DB_NAME_TO_USE"

cat > "$CONFIG_DIR/database.json" << EOF
{
    "database": "$DB_NAME_TO_USE",
    "host": "${DB_HOST:-localhost}",
    "port": ${DB_PORT:-5432},
    "username": "${DB_USER:-rdws_user}",
    "password": "${DB_PASSWORD:-rdws_password}",
    "connection_timeout": 10,
    "max_connections": 20
}
EOF

# Generate auth configuration JSON
cat > "$CONFIG_DIR/auth.json" << EOF
{
    "jwt_secret": "${JWT_SECRET:-default_secret_change_me}",
    "jwt_expiry": 3600,
    "bcrypt_rounds": 12
}
EOF

# Generate server configuration JSON
cat > "$CONFIG_DIR/server.json" << EOF
{
    "users_service": {
        "host": "${USERS_SERVICE_HOST:-0.0.0.0}",
        "port": ${USERS_SERVICE_PORT:-8081}
    },
    "orders_service": {
        "host": "${ORDERS_SERVICE_HOST:-0.0.0.0}",
        "port": ${ORDERS_SERVICE_PORT:-8082}
    },
    "api_gateway": {
        "host": "${API_GATEWAY_HOST:-0.0.0.0}",
        "port": ${API_GATEWAY_PORT:-8080}
    }
}
EOF

echo "Configuration files generated successfully!"
echo "Database: $DB_NAME_TO_USE for environment: $ENVIRONMENT"

# Set proper permissions
chmod 600 "$CONFIG_DIR/database.json" "$CONFIG_DIR/auth.json" "$CONFIG_DIR/server.json"

# Verify configuration
if [ -f "$CONFIG_DIR/database.json" ]; then
    echo "Database configuration created"
    echo "Preview (password hidden):"
    sed 's/"password": "[^"]*"/"password": "***"/' "$CONFIG_DIR/database.json"
else
    echo "Failed to create database configuration"
    exit 1
fi