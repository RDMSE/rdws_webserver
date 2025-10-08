#!/usr/bin/env bash

# exit on error
set -e

# -- Usage --
# source ./load_env.sh development
# source ./load_env.sh production 

# --- Parameters ---
ENVIRONMENT="$1"
if [ -z "$ENVIRONMENT" ]; then
    echo "Usage: source $0 [development|production]"
    exit 1
fi

# -- set env file path --
ENV_FILE="$(dirname "$0")/../.env.$ENVIRONMENT"

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