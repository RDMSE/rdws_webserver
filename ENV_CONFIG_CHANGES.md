# Environment Configuration Changes

## What Changed

1. **Removed the `ENVIRONMENT=` variable from `.env` files** since we now use `RDWS_ENVIRONMENT` from `.bashrc` for environment detection.

2. **Simplified database naming** - Each environment now uses a single `DB_NAME` variable instead of `DB_NAME_DEV` and `DB_NAME_PROD`.

## Before (deprecated):
```bash
# .env.development
DB_HOST=localhost
DB_PORT=5432
DB_USER=postgres
DB_PASS=password
DB_NAME_DEV=rdws_dev
DB_NAME_PROD=rdws_prod
ENVIRONMENT=development  # ← REMOVED (redundant)
```

## After (current):
```bash
# .env.development
DB_HOST=localhost
DB_PORT=5432
DB_USER=postgres
DB_PASS=password
DB_NAME=rdws_dev  # ← SIMPLIFIED (environment-specific)

# .env.production
DB_HOST=localhost
DB_PORT=5432
DB_USER=postgres
DB_PASS=password
DB_NAME=rdws_prod  # ← SIMPLIFIED (environment-specific)
```

## How Environment Detection Works Now

1. **RDWS_ENVIRONMENT** (from `~/.bashrc`) - Primary source
2. **ENVIRONMENT** (from environment variables) - Fallback
3. **"development"** - Default fallback

## Database Selection Logic

```cpp
// In config.h - SIMPLIFIED
database = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "postgres";
```

## Benefits

- **No redundancy** - Environment set once in `.bashrc`
- **Cleaner .env files** - Only relevant configuration per environment
- **Simpler logic** - No complex environment-specific variable selection
- **Automatic detection** - No manual environment specification
- **Consistent behavior** - Same environment across all services

## Migration

If you have existing `.env` files with:
- `ENVIRONMENT=` lines - they will be ignored
- `DB_NAME_DEV` or `DB_NAME_PROD` - should be replaced with `DB_NAME`

The system now relies on:
1. `RDWS_ENVIRONMENT` from `.bashrc` (set via `scripts/setup-environment.sh`)
2. Environment-specific `DB_NAME` in each `.env` file

## Example Usage

```bash
# Set up environment (once per machine)
./scripts/setup-environment.sh production

# All services automatically use production environment
# and load .env.production with DB_NAME=rdws_prod
```
