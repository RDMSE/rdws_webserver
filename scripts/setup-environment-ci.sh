#!/usr/bin/env bash

# CI/CD Environment Setup Script
# Designed for automated deployment environments (GitHub Actions, etc.)

set -e

ENVIRONMENT="${1:-production}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "RDWS CI/CD Environment Setup"
echo "============================"
echo "Target environment: $ENVIRONMENT"
echo "Current user: $(whoami)"
echo "Home directory: $HOME"
echo ""

# Validate environment
case "$ENVIRONMENT" in
    "development"|"production"|"staging")
        ;;
    *)
        echo "Error: Invalid environment '$ENVIRONMENT'"
        echo "Valid options: development, production, staging"
        exit 1
        ;;
esac

# Function to update .bashrc
update_bashrc() {
    local env="$1"
    
    # Create .bashrc if it doesn't exist
    touch ~/.bashrc
    
    # Remove existing RDWS_ENVIRONMENT setting
    sed -i '/export RDWS_ENVIRONMENT=/d' ~/.bashrc 2>/dev/null || true
    sed -i '/# RDWS Webserver Environment/d' ~/.bashrc 2>/dev/null || true
    
    # Add new environment setting
    echo "" >> ~/.bashrc
    echo "# RDWS Webserver Environment (CI/CD configured)" >> ~/.bashrc
    echo "export RDWS_ENVIRONMENT=$env" >> ~/.bashrc
    
    echo "Updated ~/.bashrc with RDWS_ENVIRONMENT=$env"
}

# Check current configuration
if [ -n "$RDWS_ENVIRONMENT" ]; then
    echo "RDWS_ENVIRONMENT already set in current session: $RDWS_ENVIRONMENT"
    if [ "$RDWS_ENVIRONMENT" != "$ENVIRONMENT" ]; then
        echo "Updating from $RDWS_ENVIRONMENT to $ENVIRONMENT"
        update_bashrc "$ENVIRONMENT"
    else
        echo "Environment already correctly set"
    fi
else
    echo "RDWS_ENVIRONMENT not set in current session"
    
    # Check .bashrc
    if grep -q "export RDWS_ENVIRONMENT=" ~/.bashrc 2>/dev/null; then
        CURRENT_ENV=$(grep "export RDWS_ENVIRONMENT=" ~/.bashrc | cut -d'=' -f2 | tr -d '"'"'"'')
        echo "Found RDWS_ENVIRONMENT in ~/.bashrc: $CURRENT_ENV"
        
        if [ "$CURRENT_ENV" != "$ENVIRONMENT" ]; then
            echo "Updating ~/.bashrc from $CURRENT_ENV to $ENVIRONMENT"
            update_bashrc "$ENVIRONMENT"
        else
            echo "~/.bashrc already correctly configured"
        fi
    else
        echo "No RDWS_ENVIRONMENT found in ~/.bashrc, adding it"
        update_bashrc "$ENVIRONMENT"
    fi
fi

# Source .bashrc to make changes available immediately
echo "Sourcing ~/.bashrc to apply changes..."
source ~/.bashrc

# Force set in current session for CI/CD
export RDWS_ENVIRONMENT="$ENVIRONMENT"

# Verify the change
echo ""
echo "Verification:"
echo "  RDWS_ENVIRONMENT: ${RDWS_ENVIRONMENT:-'(not set)'}"

if [ "$RDWS_ENVIRONMENT" = "$ENVIRONMENT" ]; then
    echo "  Status: Success"
else
    echo "  Status: Warning - Configuration mismatch"
    exit 1
fi

echo ""
echo "Environment setup completed for: $ENVIRONMENT"

# Show next steps based on environment
case "$ENVIRONMENT" in
    "production")
        echo ""
        echo "Production environment configured:"
        echo "- C++ microservices will use production database configuration"
        echo "- Configuration priority: Environment vars > .env.production > JSON config"
        ;;
    "development") 
        echo ""
        echo "Development environment configured:"
        echo "- C++ microservices will use development database configuration"
        echo "- Configuration priority: Environment vars > .env.development > JSON config"
        ;;
    "staging")
        echo ""
        echo "Staging environment configured:"
        echo "- C++ microservices will use staging database configuration"
        echo "- Configuration priority: Environment vars > .env.staging > JSON config"
        ;;
esac