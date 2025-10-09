#!/usr/bin/env bash

# Detects if the script is being sourced or executed
is_sourced() {
  # if BASH_SOURCE[0] != $0, means the script is being 'sourced'
  [[ "${BASH_SOURCE[0]}" != "$0" ]]
}

show_usage() {
  local script_name
  script_name="$(basename "$0")"
  echo "Usage: $script_name [development|production]"
}

# exit on error
set -e

# -- Usage --
# source ./load_env.sh development
# source ./load_env.sh production 

# --- Parameters ---
ENVIRONMENT="$1"
if [ -z "$ENVIRONMENT" ]; then
    show_usage

    if is_sourced; then
        return 1
    else
        exit 1
    fi
fi

# validate $ENVIRONMENT
case $ENVIRONMENT in
    "production") ;;
    "development") ;;
    *)
        echo "Invalid environment: $ENVIRONMENT"
        echo "Usage: $0 [development|production] [migrate|seed|reset]"
        exit 1
        ;;
esac


# -- set env file path --
ENV_FILE="$(dirname "${BASH_SOURCE[0]}")/../.env.$ENVIRONMENT"

# Check if running in CI/CD environment (GitHub Actions, etc.)
if [ -n "$GITHUB_ACTIONS" ] || [ -n "$CI" ]; then
    echo "CI/CD environment detected - using environment variables"
    
    # In CI/CD, variables should be already set by GitHub Secrets
    # Just verify they exist
    if [ -z "$DB_HOST" ] || [ -z "$DB_USER" ] || [ -z "$DB_PASS" ] || [ -z "$DB_NAME" ] || [ -z "$DB_PORT" ]; then
        echo "ERROR: Required database environment variables not found in CI/CD"
        echo "Please set: DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT"
        if is_sourced; then
            return 1
        else
            exit 1
        fi
    fi
    
    echo "Environment variables loaded from CI/CD environment"
    
elif [ -f "$ENV_FILE" ]; then
    # Local development - load from .env file
    echo "Local environment detected - loading from $ENV_FILE"
    
    # -- Load environment variables --
    # 1. Ignore comments and empty lines
    # 2. Allow quoted values and spaces
    # 3. Export all valid key=value pairs
    set -a
    # shellcheck disable=SC1090
    source "$ENV_FILE"
    set +a  
    
    echo "Environment variables loaded from $ENV_FILE"
    
else
    echo "ERROR: Environment file not found: $ENV_FILE"
    echo "For local development, create $ENV_FILE with database configuration"
    echo "For CI/CD, ensure environment variables are set via GitHub Secrets"
    
    if is_sourced; then
        return 1
    else
        exit 1
    fi
fi