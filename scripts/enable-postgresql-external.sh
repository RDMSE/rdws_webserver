#!/bin/bash
# Enable PostgreSQL External Connections
# Allows DBeaver and other external clients to connect

set -e

show_usage() {
    echo "Usage: $0 [development|production]"
    echo ""
    echo "Enables external PostgreSQL connections for the specified environment."
    echo "This allows tools like DBeaver to connect to the database remotely."
    echo ""
    echo "Examples:"
    echo "  $0 development   # Enable for development database"
    echo "  $0 production    # Enable for production database"
    echo ""
}

# Check for help
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    show_usage
    exit 0
fi

ENVIRONMENT=${1:-production}

# Validate environment
case $ENVIRONMENT in
    "production"|"development") ;;
    *)
        echo "Error: Invalid environment '$ENVIRONMENT'"
        echo ""
        show_usage
        exit 1
        ;;
esac

echo "Enabling PostgreSQL External Connections for DBeaver"
echo "======================================================"
echo "Environment: $ENVIRONMENT"

# Load environment variables
source "$(dirname "$0")/load_env.sh" "$ENVIRONMENT"

echo "Database: $DB_NAME"
echo "User: $DB_USER"
echo "Host: $DB_HOST"
echo "Port: $DB_PORT"
echo ""

# Backup current config
sudo cp /var/lib/pgsql/data/postgresql.conf /var/lib/pgsql/data/postgresql.conf.backup
sudo cp /var/lib/pgsql/data/pg_hba.conf /var/lib/pgsql/data/pg_hba.conf.backup

# Enable listening on all addresses
echo "Configuring postgresql.conf..."
sudo sed -i "s/#listen_addresses = 'localhost'/listen_addresses = '*'/" /var/lib/pgsql/data/postgresql.conf
sudo sed -i "s/listen_addresses = 'localhost'/listen_addresses = '*'/" /var/lib/pgsql/data/postgresql.conf

# Add external connection rule to pg_hba.conf
echo "Adding external access rule for $DB_USER..."
sudo tee -a /var/lib/pgsql/data/pg_hba.conf > /dev/null << EOF

# Allow external connections for $DB_USER ($ENVIRONMENT environment)
host    $DB_NAME    $DB_USER       0.0.0.0/0               md5
EOF

# Configure firewall
echo "Configuring firewall..."
sudo firewall-cmd --permanent --add-port=5432/tcp
sudo firewall-cmd --reload

# Restart PostgreSQL
echo "Restarting PostgreSQL..."
sudo systemctl restart postgresql

# Show status
echo "PostgreSQL Status:"
sudo systemctl status postgresql --no-pager

echo ""
echo "External connections enabled for $ENVIRONMENT!"
echo ""
echo "DBeaver Connection Info:"
echo "  Host: fedora-server.local (or IP address)"
echo "  Port: $DB_PORT"
echo "  Database: $DB_NAME"
echo "  Username: $DB_USER"
echo "  Password: $DB_PASS"
echo ""
echo "Test connection:"
echo "  nmap -p $DB_PORT fedora-server.local"
