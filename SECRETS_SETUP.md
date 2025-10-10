# GitHub Secrets Configuration

To configure sensitive environment variables in GitHub:

## 1. Access GitHub Secrets

1. Go to your repository on GitHub
2. Click on **Settings** → **Secrets and variables** → **Actions**
3. Click on **New repository secret**

## 2. Add the following secrets:

### Database Configuration:
- `DB_HOST` - PostgreSQL server hostname (e.g., `fedora-server.local`)
- `DB_PORT` - PostgreSQL port (e.g., `5432`) 
- `DB_USER` - Database user (e.g., `rdws_user`)
- `DB_PASS` - Database password (e.g., `your_secure_password`)
- `DB_NAME_PROD` - Production database name (e.g., `rdws_production`)
- `DB_NAME_DEV` - Development database name (e.g., `rdws_development`)

## 3. Local Development

For local development, create the files:
- `.env.development` (based on `.env.development.example`)
- `.env.production` (based on `.env.production.example`)

**IMPORTANT**: Never commit the actual `.env.*` files!

## 4. How it works

- **CI/CD**: Uses GitHub Secrets automatically
- **Local**: Uses `.env.*` files (not committed)
- **`load_env.sh` script**: Automatically detects the environment

## 5. Verification

To test if it's working:

```bash
# Local (requires .env.development)
./scripts/test-postgresql.sh development

# In GitHub Actions, variables are injected automatically
```