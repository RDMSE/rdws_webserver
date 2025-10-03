# DEPRECATED: This script was designed for Pistache server - needs update for microservices architecture
#!/bin/bash

# simple script to create a compressed package of the project
# useful for backup or manual transfer

echo "Creating package of C++ REST Server project..."

cd /home/rdias/sources/lab/server

# Create compressed tar file excluding build files
tar czf /tmp/cpp-rest-server-$(date +%Y%m%d-%H%M).tar.gz \
    --exclude='build' \
    --exclude='.git' \
    --exclude='*.log' \
    --exclude='server.pid' \
    --exclude='test_results.xml' \
    .

PACKAGE_FILE=$(ls -t /tmp/cpp-rest-server-*.tar.gz | head -1)

echo "Package created: $PACKAGE_FILE"
echo "Size: $(du -h "$PACKAGE_FILE" | cut -f1)"

echo ""
echo "To use:"
echo "  scp $PACKAGE_FILE your-machine:~/"
