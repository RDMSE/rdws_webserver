#!/bin/bash
# Script to fix SELinux contexts for C++ REST Server

echo "Fixing SELinux contexts for C++ REST Server..."

EXECUTABLE_PATH="/home/rdias/cpp-rest-server/rest_server"

if command -v getenforce >/dev/null && [ "$(getenforce)" = "Enforcing" ]; then
    echo "SELinux is enforcing - configuring contexts..."

    # Set executable context for systemd
    echo "Setting binary context..."
    sudo semanage fcontext -a -t bin_t "$EXECUTABLE_PATH" 2>/dev/null || echo "Context rule already exists"

    # Restore context
    echo "Restoring file context..."
    sudo restorecon -v "$EXECUTABLE_PATH"

    # Enable required SELinux booleans
    echo "Setting SELinux booleans..."
    sudo setsebool -P httpd_exec_enable 1 2>/dev/null || echo "Could not set httpd_exec_enable"

    # Show final context
    echo "Final file context:"
    ls -laZ "$EXECUTABLE_PATH" 2>/dev/null || ls -la "$EXECUTABLE_PATH"

    echo "SELinux configuration completed successfully!"
else
    echo "SELinux is not enforcing or not available - no action needed"
fi
