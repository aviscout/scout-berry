#!/bin/bash
#
# Hardware Verification Script for Scout Beacon PSDK Setup
# This script verifies UART device, permissions, and hardware connections
#

set -e

echo "=== Scout Beacon Hardware Verification ==="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check UART device
echo "1. Checking UART device..."
if [ -e /dev/ttyUSB0 ]; then
    echo -e "${GREEN}✓${NC} UART device /dev/ttyUSB0 exists"
    
    # Check permissions
    PERMS=$(stat -c "%a" /dev/ttyUSB0)
    if [ "$PERMS" = "666" ] || [ "$PERMS" = "777" ]; then
        echo -e "${GREEN}✓${NC} UART device permissions: $PERMS (correct)"
    else
        echo -e "${YELLOW}⚠${NC} UART device permissions: $PERMS (should be 666 or 777)"
        echo "   Fix with: sudo chmod 666 /dev/ttyUSB0"
    fi
    
    # Check if device is in use
    if lsof /dev/ttyUSB0 >/dev/null 2>&1; then
        echo -e "${YELLOW}⚠${NC} UART device is in use by another process"
        echo "   Processes using device:"
        lsof /dev/ttyUSB0
    else
        echo -e "${GREEN}✓${NC} UART device is not in use"
    fi
else
    echo -e "${RED}✗${NC} UART device /dev/ttyUSB0 does not exist"
    echo "   Check E-Port adapter connection"
fi

echo ""

# Check alternative UART device
echo "2. Checking alternative UART device..."
if [ -e /dev/ttyACM0 ]; then
    echo -e "${GREEN}✓${NC} Alternative UART device /dev/ttyACM0 exists"
    PERMS=$(stat -c "%a" /dev/ttyACM0)
    echo "   Permissions: $PERMS"
else
    echo -e "${YELLOW}⚠${NC} Alternative UART device /dev/ttyACM0 does not exist"
fi

echo ""

# Check system messages for UART
echo "3. Checking system messages for UART device..."
UART_MSG=$(dmesg | tail -50 | grep -i "ttyUSB\|usb.*serial" | tail -5)
if [ -n "$UART_MSG" ]; then
    echo -e "${GREEN}✓${NC} Recent UART system messages:"
    echo "$UART_MSG" | sed 's/^/   /'
else
    echo -e "${YELLOW}⚠${NC} No recent UART system messages found"
fi

echo ""

# Check log directory
echo "4. Checking log directory..."
if [ -d "Logs/DJI" ]; then
    echo -e "${GREEN}✓${NC} Log directory exists: Logs/DJI"
    
    # Check ownership
    OWNER=$(stat -c "%U:%G" Logs/DJI)
    if [ "$OWNER" = "aviscout:aviscout" ]; then
        echo -e "${GREEN}✓${NC} Log directory ownership: $OWNER (correct)"
    else
        echo -e "${YELLOW}⚠${NC} Log directory ownership: $OWNER (should be aviscout:aviscout)"
        echo "   Fix with: sudo chown -R aviscout:aviscout Logs"
    fi
else
    echo -e "${YELLOW}⚠${NC} Log directory does not exist"
    echo "   Creating with: sudo mkdir -p Logs/DJI && sudo chown -R aviscout:aviscout Logs"
    sudo mkdir -p Logs/DJI
    sudo chown -R aviscout:aviscout Logs
    echo -e "${GREEN}✓${NC} Log directory created"
fi

echo ""

# Check executable
echo "5. Checking application executable..."
if [ -f "bin/scout_beacon" ]; then
    echo -e "${GREEN}✓${NC} Application executable exists: bin/scout_beacon"
    
    # Check if executable
    if [ -x "bin/scout_beacon" ]; then
        echo -e "${GREEN}✓${NC} Application is executable"
    else
        echo -e "${YELLOW}⚠${NC} Application is not executable"
        echo "   Fix with: chmod +x bin/scout_beacon"
    fi
else
    echo -e "${RED}✗${NC} Application executable not found: bin/scout_beacon"
    echo "   Build the application first: cd build && cmake .. && make"
fi

echo ""

# Summary
echo "=== Verification Summary ==="
echo ""
echo "Physical verification checklist:"
echo "  [ ] E-Port adapter is properly connected to drone"
echo "  [ ] Payload board LED is active (green light)"
echo "  [ ] All cables are securely connected"
echo "  [ ] Raspberry Pi is receiving power from drone"
echo "  [ ] No loose connections or damaged cables"
echo ""
echo "Next steps:"
echo "  1. Verify PSDK mode is enabled in DJI Assistant 2"
echo "  2. Run: sudo ./bin/scout_beacon --mock"
echo "  3. Monitor output for successful PSDK initialization"
echo ""

