#!/bin/bash

# Scout Beacon Build and Test Script
# This script builds the scout beacon application and provides testing instructions

set -e  # Exit on any error

echo "=== Scout Beacon Build and Test Script ==="
echo "Copyright (c) 2024 Scout Berry"
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the scout_beacon directory."
    exit 1
fi

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "Building scout beacon application..."
make -j$(nproc)

# Check if build was successful
if [ -f "scout_beacon" ]; then
    echo ""
    echo "✓ Build successful! Executable created: scout_beacon"
    echo ""
    echo "=== Deployment Instructions ==="
    echo "1. Commit and push changes to git:"
    echo "   git add ."
    echo "   git commit -m 'Add Scout Beacon application'"
    echo "   git push"
    echo ""
    echo "2. SSH to Raspberry Pi:"
    echo "   ssh aviscout@192.168.1.84"
    echo ""
    echo "3. Pull latest changes on Raspberry Pi:"
    echo "   cd /home/aviscout/MVP/Payload-SDK"
    echo "   git pull"
    echo ""
    echo "4. Build on Raspberry Pi:"
    echo "   cd samples/sample_c/platform/linux/scout_beacon"
    echo "   mkdir -p build && cd build"
    echo "   cmake .. && make"
    echo ""
    echo "5. Run the application:"
    echo "   sudo ./scout_beacon"
    echo ""
    echo "=== Testing Instructions ==="
    echo "1. Ensure beacon receiver is connected to GPIO pins"
    echo "2. Power on avalanche beacon transmitter"
    echo "3. Monitor application output for beacon detection"
    echo "4. Check DJI Assistant 2 for received data"
    echo "5. Verify logs show successful PSDK transmission"
    echo ""
    echo "=== GPIO Pin Configuration ==="
    echo "Bearing pins: 17, 27, 22, 5, 6 (BCM numbering)"
    echo "7-segment pins: 13, 19, 26, 21, 20, 16, 12, 25, 8, 7"
    echo "RSSI pin: 18"
    echo ""
    echo "=== Troubleshooting ==="
    echo "- If GPIO access fails, ensure user is in 'gpio' group:"
    echo "  sudo usermod -a -G gpio aviscout"
    echo "- If lgpio library not found, install:"
    echo "  sudo apt update && sudo apt install liblgpio-dev"
    echo "- Check PSDK configuration in dji_sdk_app_info.h"
    echo ""
else
    echo "✗ Build failed! Check error messages above."
    exit 1
fi
