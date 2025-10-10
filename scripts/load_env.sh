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

    # In CI/CD, set DB_NAME based on environment using DB_NAME_DEV or DB_NAME_PROD
    case $ENVIRONMENT in
        "production")
            if [ -z "$DB_NAME_PROD" ]; then
                echo "ERROR: DB_NAME_PROD not found in CI/CD environment"
                if is_sourced; then
                    return 1
                else
                    exit 1
                fi
            fi
            export DB_NAME="$DB_NAME_PROD"
            echo "Using production database: $DB_NAME"
            ;;
        "development")
            if [ -z "$DB_NAME_DEV" ]; then
                echo "ERROR: DB_NAME_DEV not found in CI/CD environment"
                if is_sourced; then
                    return 1
                else
                    exit 1
                fi
            fi
            export DB_NAME="$DB_NAME_DEV"
            echo "Using development database: $DB_NAME"
            ;;
    esac
    
    # For CI/CD, prefer localhost for PostgreSQL connections to avoid IPv6 issues
    if [ -n "$GITHUB_ACTIONS" ] || [ -n "$CI" ]; then
        if [ "$DB_HOST" = "fedora-server.local" ] || [ "$DB_HOST" = "$(hostname)" ] || [ "$DB_HOST" = "$(hostname -f)" ]; then
            export DB_HOST="localhost"
            echo "Using localhost for PostgreSQL connection in CI/CD"
        fi
        
        # Set PGHOST to ensure psql uses the correct host
        export PGHOST="$DB_HOST"
        export PGPORT="$DB_PORT"
        export PGUSER="$DB_USER"
        export PGPASSWORD="$DB_PASS"
    fi

    # Verify other required variables exist
    if [ -z "$DB_HOST" ] || [ -z "$DB_USER" ] || [ -z "$DB_PASS" ] || [ -z "$DB_PORT" ]; then
        echo "ERROR: Required database environment variables not found in CI/CD"
        echo "Please set: DB_HOST, DB_USER, DB_PASS, DB_PORT, DB_NAME_DEV, DB_NAME_PROD"
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
