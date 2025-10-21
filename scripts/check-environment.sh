#!/usr/bin/env bash

# Check Environment Configuration Script
# Shows current RDWS environment setup and validates configuration

set -e

echo "RDWS Environment Check"
echo "======================"

# Check RDWS_ENVIRONMENT variable
if [ -n "$RDWS_ENVIRONMENT" ]; then
    echo "RDWS_ENVIRONMENT: $RDWS_ENVIRONMENT"
else
    echo "RDWS_ENVIRONMENT: Not set"
fi

# Check other environment variables
echo ""
echo "Environment Variables:"
echo "   RDWS_ENVIRONMENT: ${RDWS_ENVIRONMENT:-'(not set)'}"
echo "   ENVIRONMENT: ${ENVIRONMENT:-'(not set)'}"
echo "   NODE_ENV: ${NODE_ENV:-'(not set)'}"
echo "   CI: ${CI:-'(not set)'}"
echo "   GITHUB_ACTIONS: ${GITHUB_ACTIONS:-'(not set)'}"

# Check database variables
echo ""
echo "Database Variables:"
echo "   DB_HOST: ${DB_HOST:-'(not set)'}"
echo "   DB_PORT: ${DB_PORT:-'(not set)'}"
echo "   DB_USER: ${DB_USER:-'(not set)'}"
echo "   DB_PASS: ${DB_PASS:+'(set)' || '(not set)'}"
echo "   DB_NAME: ${DB_NAME:-'(not set)'}"
echo "   DB_NAME_DEV: ${DB_NAME_DEV:-'(not set)'}"
echo "   DB_NAME_PROD: ${DB_NAME_PROD:-'(not set)'}"

# Check .bashrc configuration
echo ""
echo ".bashrc Configuration:"
if grep -q "export RDWS_ENVIRONMENT=" ~/.bashrc 2>/dev/null; then
    BASHRC_ENV=$(grep "export RDWS_ENVIRONMENT=" ~/.bashrc | cut -d'=' -f2)
    echo "   Found: export RDWS_ENVIRONMENT=$BASHRC_ENV"
    
    if [ "$RDWS_ENVIRONMENT" = "$BASHRC_ENV" ]; then
        echo "   Current session matches .bashrc"
    else
        echo "   Warning: Current session ($RDWS_ENVIRONMENT) differs from .bashrc ($BASHRC_ENV)"
        echo "   Tip: Run: source ~/.bashrc"
    fi
else
    echo "   No RDWS_ENVIRONMENT found in ~/.bashrc"
    echo "   Tip: Run: ./scripts/setup-environment.sh [development|production]"
fi

# Check .env files
echo ""
echo "Configuration Files:"
PROJECT_ROOT="$(dirname "$(dirname "${BASH_SOURCE[0]}")")"

for env in development production staging; do
    ENV_FILE="$PROJECT_ROOT/.env.$env"
    if [ -f "$ENV_FILE" ]; then
        echo "   .env.$env exists"
    else
        echo "   .env.$env missing"
    fi
done

# Check JSON config
JSON_CONFIG="/opt/rdws_webserver/config/database.json"
if [ -f "$JSON_CONFIG" ]; then
    echo "   JSON config exists: $JSON_CONFIG"
else
    echo "   JSON config missing: $JSON_CONFIG"
fi

# Provide recommendations
echo ""
echo "Recommendations:"

if [ -z "$RDWS_ENVIRONMENT" ]; then
    echo "   1. Set up environment: ./scripts/setup-environment.sh [development|production]"
fi

case "${RDWS_ENVIRONMENT:-development}" in
    "development")
        if [ ! -f "$PROJECT_ROOT/.env.development" ]; then
            echo "   2. Create .env.development file with development database credentials"
        fi
        ;;
    "production")
        if [ ! -f "$PROJECT_ROOT/.env.production" ] && [ ! -f "$JSON_CONFIG" ]; then
            echo "   2. Create .env.production file or deploy JSON configuration"
        fi
        ;;
    "staging")
        if [ ! -f "$PROJECT_ROOT/.env.staging" ]; then
            echo "   2. Create .env.staging file with staging database credentials"
        fi
        ;;
esac

if [ -n "$RDWS_ENVIRONMENT" ] && [ "$RDWS_ENVIRONMENT" != "${BASHRC_ENV:-}" ]; then
    echo "   3. Restart terminal or run: source ~/.bashrc"
fi

echo ""
echo "Configuration check complete!"