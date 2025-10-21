#!/usr/bin/env bash

# Setup Environment Configuration Script
# Configures RDWS_ENVIRONMENT in .bashrc for different machines

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "RDWS Environment Setup"
echo "======================"

show_usage() {
    echo "Usage: $0 [development|production|staging]"
    echo ""
    echo "This script configures the RDWS_ENVIRONMENT variable in your ~/.bashrc"
    echo "to automatically detect the machine type for database configuration."
    echo ""
    echo "Options:"
    echo "  development  - Development machine (uses .env.development)"
    echo "  production   - Production server (uses .env.production or JSON config)"
    echo "  staging      - Staging server (uses .env.staging)"
    echo ""
    echo "Examples:"
    echo "  $0 development  # Set up dev machine"
    echo "  $0 production   # Set up production server"
}

if [ $# -eq 0 ]; then
    show_usage
    exit 1
fi

ENVIRONMENT="$1"

# Validate environment
case "$ENVIRONMENT" in
    "development"|"production"|"staging")
        ;;
    *)
        echo "Error: Invalid environment '$ENVIRONMENT'"
        echo ""
        show_usage
        exit 1
        ;;
esac

echo "Setting up environment: $ENVIRONMENT"
echo ""

# Check if RDWS_ENVIRONMENT is already set
if grep -q "export RDWS_ENVIRONMENT=" ~/.bashrc 2>/dev/null; then
    CURRENT_ENV=$(grep "export RDWS_ENVIRONMENT=" ~/.bashrc | cut -d'=' -f2)
    echo "Warning: RDWS_ENVIRONMENT is already set to: $CURRENT_ENV"
    echo ""
    
    # In CI/CD mode, auto-update if needed
    if [ -n "$CI" ] || [ -n "$GITHUB_ACTIONS" ]; then
        echo "CI/CD mode detected, auto-updating to '$ENVIRONMENT'"
        # Remove existing line
        sed -i '/export RDWS_ENVIRONMENT=/d' ~/.bashrc
        echo "Removed existing RDWS_ENVIRONMENT setting"
    else
        read -p "Do you want to update it to '$ENVIRONMENT'? (y/N): " -n 1 -r
        echo ""
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Cancelled."
            exit 0
        fi
        
        # Remove existing line
        sed -i '/export RDWS_ENVIRONMENT=/d' ~/.bashrc
        echo "Removed existing RDWS_ENVIRONMENT setting"
    fi
fi# Add new environment setting
echo "" >> ~/.bashrc
echo "# RDWS Webserver Environment (auto-configured)" >> ~/.bashrc
echo "export RDWS_ENVIRONMENT=$ENVIRONMENT" >> ~/.bashrc

echo "Added 'export RDWS_ENVIRONMENT=$ENVIRONMENT' to ~/.bashrc"
echo ""

# Show current configuration
echo ""
echo "Current configuration:"
echo "   Environment: $ENVIRONMENT"
echo "   Config file: ~/.bashrc"
if [ -n "$CI" ] || [ -n "$GITHUB_ACTIONS" ]; then
    echo "   Mode: CI/CD (automated)"
else
    echo "   Mode: Interactive"
fi
echo ""

# Provide guidance based on environment
case "$ENVIRONMENT" in
    "development")
        echo "Development setup complete!"
        echo ""
        echo "Next steps:"
        echo "1. Make sure you have .env.development file in project root"
        echo "2. Restart your terminal or run: source ~/.bashrc"
        echo "3. Test with: echo \$RDWS_ENVIRONMENT"
        echo ""
        echo "Your C++ microservices will automatically use development configuration."
        ;;
    "production")
        echo "Production setup complete!"
        echo ""
        echo "Next steps:"
        echo "1. Ensure .env.production exists or JSON config is deployed"
        echo "2. Restart your terminal or run: source ~/.bashrc"
        echo "3. Test with: echo \$RDWS_ENVIRONMENT"
        echo ""
        echo "Your C++ microservices will automatically use production configuration."
        ;;
    "staging")
        echo "Staging setup complete!"
        echo ""
        echo "Next steps:"
        echo "1. Create .env.staging file with staging database credentials"
        echo "2. Restart your terminal or run: source ~/.bashrc"
        echo "3. Test with: echo \$RDWS_ENVIRONMENT"
        echo ""
        echo "Your C++ microservices will automatically use staging configuration."
        ;;
esac

echo ""
echo "Tip: You can check your configuration anytime with:"
echo "   ./scripts/check-environment.sh"
