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

if [ ! -f "$ENV_FILE" ]; then
    echo "Environment file not found: $ENV_FILE"
    return 1
fi

# -- Load environment variables --
# 1. Ignore comments and empty lines
# 2. Allow qouted values and spaces
# 3. Export all valid key=value pairs
set -a
# shellcheck disable=SC1090
source "$ENV_FILE"
set +a  

echo "Environment variables loaded from $ENV_FILE"