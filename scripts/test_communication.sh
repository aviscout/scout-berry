#!/bin/bash
#
# Communication Test Script for Scout Beacon PSDK Setup
# This script prepares the environment and runs the application in mock mode
#

set -e

echo "=== Scout Beacon Communication Test ==="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if we're in the right directory
if [ ! -f "bin/scout_beacon" ]; then
    echo -e "${RED}Error:${NC} Application executable not found"
    echo "   Please run this script from the build directory:"
    echo "   cd /home/aviscout/MVP/scout-berry/scout_beacon/build"
    exit 1
fi

# Prepare environment
echo "1. Preparing environment..."

# Create log directory if it doesn't exist
if [ ! -d "Logs/DJI" ]; then
    echo "   Creating log directory..."
    sudo mkdir -p Logs/DJI
    sudo chown -R aviscout:aviscout Logs
    echo -e "${GREEN}✓${NC} Log directory created"
else
    echo -e "${GREEN}✓${NC} Log directory exists"
fi

# Check UART device permissions
if [ -e /dev/ttyUSB0 ]; then
    PERMS=$(stat -c "%a" /dev/ttyUSB0)
    if [ "$PERMS" != "666" ] && [ "$PERMS" != "777" ]; then
        echo "   Fixing UART device permissions..."
        sudo chmod 666 /dev/ttyUSB0
        echo -e "${GREEN}✓${NC} UART device permissions fixed"
    else
        echo -e "${GREEN}✓${NC} UART device permissions OK"
    fi
else
    echo -e "${YELLOW}⚠${NC} UART device /dev/ttyUSB0 not found"
    echo "   Make sure E-Port adapter is connected"
fi

echo ""

# Run application
echo "2. Starting Scout Beacon application in mock mode..."
echo "   Press Ctrl+C to stop"
echo ""
echo "--- Application Output ---"
echo ""

# Run with mock mode
sudo ./bin/scout_beacon --mock

