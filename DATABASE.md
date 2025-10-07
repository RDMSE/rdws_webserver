# Database Setup & Management

## PostgreSQL Database Integration

This project uses PostgreSQL for data persistence with separate databases for development, production, and testing.

## Quick Start

### Automatic Setup (GitHub Actions)

The database is automatically set up during deployment:

```bash
# GitHub Actions handles this automatically on deploy
git push origin main  # Triggers deploy with database setup
```

### Manual Setup

```bash
# 1. Install PostgreSQL (if not installed)
./scripts/setup-postgresql.sh

# 2. Setup development database
npm run db:setup

# 3. Setup production database
npm run db:setup:prod
```

## Database Structure

### **Environments**
- **Development**: `rdws_development`
- **Production**: `rdws_production`
- **Test**: `rdws_test`

### **Tables**
- `users` - User accounts
- `orders` - Order transactions
- `products` - Product catalog
- `categories` - Product categories
- `migrations` - Migration tracking

## Available Commands

### Development
```bash
npm run db:setup          # Setup development database
npm run db:migrate        # Run migrations
npm run db:seed           # Run seed data
npm run db:reset          # Reset database
npm run db:status         # Show database status
```

### Production
```bash
npm run db:setup:prod     # Setup production database
npm run db:migrate:prod   # Run production migrations
npm run db:seed:prod      # Run production seeds
```

### Manual Scripts
```bash
# PostgreSQL installation
./scripts/setup-postgresql.sh

# Database operations
./scripts/migrate-database.sh [environment] [action]
# Actions: migrate, seed, reset, status
# Environments: development, production, test
```

## Connection Information

### Development
```
Host: localhost
Port: 5432
Database: rdws_development
User: rdws_user
Password: rdws_pass123
```

### Production
```
Host: localhost (on server)
Port: 5432
Database: rdws_production
User: rdws_user
Password: rdws_pass123 (change in production)
```

### Connection String
```
postgresql://rdws_user:rdws_pass123@localhost:5432/rdws_development
```

## Database Files Structure

```
database/
├── config.js                 # Environment configurations
├── connection.js             # Connection helper
├── migrations/               # Database migrations
│   ├── 001_create_users_table.sql
│   ├── 002_create_orders_table.sql
│   └── 003_create_products_table.sql
└── seeds/                    # Seed data
    ├── development_data.sql  # Dev sample data
    └── production_data.sql   # Prod essential data
```

## Migration System

The migration system tracks executed migrations in a `migrations` table:

```sql
CREATE TABLE migrations (
    id SERIAL PRIMARY KEY,
    filename VARCHAR(255) NOT NULL UNIQUE,
    executed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Adding New Migrations

1. Create migration file: `database/migrations/004_your_migration.sql`
2. Write SQL commands
3. Run migration: `npm run db:migrate`

### Migration Example
```sql
-- Migration: Add new column
-- Created: 2025-10-07
-- Description: Add phone number to users

ALTER TABLE users ADD COLUMN phone VARCHAR(20);
CREATE INDEX idx_users_phone ON users(phone);
```

## Seed Data

### Development Seeds
- Sample users, products, orders
- Test categories and data
- Realistic development data

### Production Seeds
- Essential categories only
- Admin user (if needed)
- Minimal required data

## Security Considerations

### Production Setup
1. **Change default passwords**
2. **Use environment variables**
3. **Restrict database access**
4. **Enable SSL connections**
5. **Regular backups**

### Environment Variables
```bash
# Production overrides
export DB_PASS="your_secure_password"
export DB_HOST="your_db_host"
export DB_PORT="5432"
```

## Troubleshooting

### PostgreSQL Not Starting
```bash
sudo systemctl status postgresql
sudo systemctl start postgresql
sudo journalctl -u postgresql
```

### Connection Issues
```bash
# Test connection
psql -h localhost -U rdws_user -d rdws_development

# Check user permissions
sudo -u postgres psql -c "\du"
```

### Migration Failures
```bash
# Check migration status
./scripts/migrate-database.sh development status

# Reset database (DESTRUCTIVE)
./scripts/migrate-database.sh development reset
```

## GitHub Actions Integration

The deploy workflow automatically:

1. **Installs PostgreSQL** (if not present)
2. **Creates databases** (dev/prod/test)
3. **Runs migrations** (production environment)
4. **Applies seeds** (minimal production data)
5. **Verifies setup** (connection test)

### Workflow Steps
```yaml
- name: Setup PostgreSQL Database
  run: |
    chmod +x scripts/setup-postgresql.sh
    ./scripts/setup-postgresql.sh
    ./scripts/migrate-database.sh production migrate
    ./scripts/migrate-database.sh production seed
```

## Monitoring

### Database Status
```bash
# Show tables and migrations
npm run db:status

# PostgreSQL status
sudo systemctl status postgresql

# Database size
psql -U rdws_user -d rdws_production -c "SELECT pg_size_pretty(pg_database_size('rdws_production'));"
```

### Performance
```sql
-- Active connections
SELECT count(*) FROM pg_stat_activity;

-- Table sizes
SELECT schemaname,tablename,attname,n_distinct,correlation
FROM pg_stats WHERE tablename='users';
```

---

**Database setup is now fully automated and integrated with your CI/CD pipeline!**
