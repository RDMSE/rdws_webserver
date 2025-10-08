#!/bin/bash
# Database Migration Script
# Runs migrations and seeds for specified environment

set -e

ENVIRONMENT=${1:-development}
ACTION=${2:-migrate}


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


echo "Database Migration Script"
echo "Environment: $ENVIRONMENT"
echo "Action: $ACTION"

source "$(dirname "$0")/load_env.sh" "$ENVIRONMENT"

echo "Target database: $DB_NAME"

# Export for psql
export PGPASSWORD=$DB_PASS

# Functions
run_migrations() {
    echo "Running migrations..."

    # Create migrations table if not exists
    psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -c "
        CREATE TABLE IF NOT EXISTS migrations (
            id SERIAL PRIMARY KEY,
            filename VARCHAR(255) NOT NULL UNIQUE,
            executed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    "

    # Run each migration file
    for migration in database/migrations/*.sql; do
        if [ -f "$migration" ]; then
            filename=$(basename "$migration")

            # Check if migration already executed
            exists=$(psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -t -c \
                "SELECT COUNT(*) FROM migrations WHERE filename='$filename'")

            if [ "$(echo $exists | tr -d ' ')" = "0" ]; then
                echo "  Executing: $filename"
                psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -f "$migration"
                psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -c \
                    "INSERT INTO migrations (filename) VALUES ('$filename')"
                echo "  Completed: $filename"
            else
                echo "  Skipping: $filename (already executed)"
            fi
        fi
    done
}

run_seeds() {
    echo "Running seeds..."
    for seed in database/seeds/*.sql; do
        if [ -f "$seed" ]; then
            filename=$(basename "$seed")
            echo "  Executing: $filename"
            psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -f "$seed"
            echo "  Completed: $filename"
        fi
    done
}

reset_database() {
    echo "Resetting database..."

    # Drop all tables
    psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -c "
        DROP SCHEMA public CASCADE;
        CREATE SCHEMA public;
        GRANT ALL ON SCHEMA public TO rdws_user;
        GRANT ALL ON SCHEMA public TO public;
    "

    echo "Database reset complete"
}

show_status() {
    echo "Database Status:"
    echo "Tables:"
    psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -c "\dt"
    echo ""
    echo "Executed Migrations:"
    psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -c \
        "SELECT filename, executed_at FROM migrations ORDER BY executed_at;" 2>/dev/null || echo "No migrations table yet"
}

# Execute action
case $ACTION in
    "migrate")
        run_migrations
        show_status
        ;;
    "seed")
        run_seeds
        ;;
    "reset")
        reset_database
        run_migrations
        run_seeds
        show_status
        ;;
    "status")
        show_status
        ;;
    *)
        echo "Invalid action: $ACTION"
        echo "Usage: $0 [environment] [migrate|seed|reset|status]"
        exit 1
        ;;
esac

echo "Database operation completed successfully!"
