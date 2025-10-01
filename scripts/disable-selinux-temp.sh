#!/bin/bash
# Temporarily disable SELinux for deployment

echo "=== Temporary SELinux Solution ==="

# Check current status
echo "Current SELinux status:"
getenforce

# Set to permissive temporarily
echo "Setting SELinux to permissive mode temporarily..."
sudo setenforce 0

echo "New SELinux status:"
getenforce

echo "Note: This is temporary and will revert on reboot"
echo "For permanent solution, configure proper SELinux contexts"
