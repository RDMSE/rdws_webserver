# Deprecated Files

The following files contain references to the old Pistache-based architecture and should be updated or removed:

## Scripts (marked as deprecated)
- `scripts/setup_github_runner.sh` - Old GitHub runner setup for Pistache
- `scripts/setup_linux_mint.sh` - Old Linux Mint setup for Pistache

## Docker
- `Dockerfile` - Contains Pistache installation steps (cleaned up)

## Current Architecture
This project now uses:
- **C++ Microservices**: Independent services with simple command-line interfaces
- **TypeScript API Gateway**: Handles HTTP routing and service orchestration
- **PostgreSQL Database**: With libpqxx for C++ database connectivity
- **Modern Configuration**: dotenv-cpp for .env file support

## Migration Notes
The transition from Pistache to microservices provides:
- Better service isolation
- Easier testing and deployment
- Language flexibility (C++ for performance, TypeScript for HTTP handling)
- Cleaner separation of concerns
