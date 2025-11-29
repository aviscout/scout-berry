#!/bin/bash
#
# Success Verification Script for Scout Beacon PSDK Setup
# This script checks logs and verifies successful communication
#

set -e

echo "=== Scout Beacon Communication Verification ==="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if log directory exists
if [ ! -d "Logs/DJI" ]; then
    echo -e "${RED}Error:${NC} Log directory not found"
    echo "   Run the application first to generate logs"
    exit 1
fi

# Find latest log file
LATEST_LOG=$(ls -t Logs/DJI/psdk_log_*.txt 2>/dev/null | head -1)

if [ -z "$LATEST_LOG" ]; then
    echo -e "${RED}Error:${NC} No log files found in Logs/DJI/"
    echo "   Run the application first to generate logs"
    exit 1
fi

echo "Checking log file: $LATEST_LOG"
echo ""

# Check for successful initialization
echo "1. Checking PSDK initialization..."
if grep -q "Core init\|Payload SDK Version" "$LATEST_LOG" 2>/dev/null; then
    echo -e "${GREEN}✓${NC} PSDK initialization found"
    grep "Payload SDK Version\|Core init" "$LATEST_LOG" | tail -3 | sed 's/^/   /'
else
    echo -e "${RED}✗${NC} PSDK initialization not found in logs"
fi

echo ""

# Check for successful connection
echo "2. Checking UART connection..."
if grep -q "Access adapter init success\|Try identify UART0 connection" "$LATEST_LOG" 2>/dev/null; then
    if grep -q "Access adapter init success" "$LATEST_LOG" 2>/dev/null; then
        echo -e "${GREEN}✓${NC} UART connection successful"
        grep "Access adapter init success" "$LATEST_LOG" | tail -1 | sed 's/^/   /'
    else
        echo -e "${RED}✗${NC} UART connection failed or timed out"
        echo "   Failed connection attempts:"
        grep "Try identify UART0 connection failed\|timeout" "$LATEST_LOG" | tail -3 | sed 's/^/   /'
    fi
else
    echo -e "${YELLOW}⚠${NC} No UART connection information found"
fi

echo ""

# Check for aircraft info
echo "3. Checking aircraft information..."
if grep -q "Aircraft type\|Aircraft version" "$LATEST_LOG" 2>/dev/null; then
    echo -e "${GREEN}✓${NC} Aircraft information retrieved"
    grep "Aircraft type\|Aircraft version" "$LATEST_LOG" | tail -2 | sed 's/^/   /'
else
    echo -e "${YELLOW}⚠${NC} Aircraft information not found"
fi

echo ""

# Check for data transmission
echo "4. Checking data transmission..."
if grep -q "Beacon data transmitted successfully" "$LATEST_LOG" 2>/dev/null; then
    COUNT=$(grep -c "Beacon data transmitted successfully" "$LATEST_LOG")
    echo -e "${GREEN}✓${NC} Data transmission successful ($COUNT transmissions)"
    echo "   Recent transmissions:"
    grep "Beacon data transmitted successfully" "$LATEST_LOG" | tail -3 | sed 's/^/   /'
else
    echo -e "${YELLOW}⚠${NC} No successful data transmissions found"
fi

echo ""

# Check for errors
echo "5. Checking for errors..."
ERROR_COUNT=$(grep -ci "error\|failed\|timeout" "$LATEST_LOG" 2>/dev/null || echo "0")
if [ "$ERROR_COUNT" -eq 0 ]; then
    echo -e "${GREEN}✓${NC} No errors found in logs"
else
    echo -e "${YELLOW}⚠${NC} Found $ERROR_COUNT error/warning messages"
    echo "   Recent errors:"
    grep -i "error\|failed\|timeout" "$LATEST_LOG" | tail -5 | sed 's/^/   /'
fi

echo ""

# Summary
echo "=== Verification Summary ==="
echo ""

# Count success indicators
SUCCESS=0
TOTAL=5

if grep -q "Payload SDK Version" "$LATEST_LOG" 2>/dev/null; then ((SUCCESS++)); fi
if grep -q "Access adapter init success" "$LATEST_LOG" 2>/dev/null; then ((SUCCESS++)); fi
if grep -q "Aircraft type" "$LATEST_LOG" 2>/dev/null; then ((SUCCESS++)); fi
if grep -q "Beacon data transmitted successfully" "$LATEST_LOG" 2>/dev/null; then ((SUCCESS++)); fi
if [ "$ERROR_COUNT" -eq 0 ]; then ((SUCCESS++)); fi

if [ "$SUCCESS" -eq "$TOTAL" ]; then
    echo -e "${GREEN}✓${NC} All checks passed! Communication is successful."
    echo ""
    echo "Next steps:"
    echo "  1. Verify payload appears in DJI Assistant 2"
    echo "  2. Monitor low-speed data channel in DJI Assistant 2"
    echo "  3. Test with real beacon hardware"
elif [ "$SUCCESS" -ge 3 ]; then
    echo -e "${YELLOW}⚠${NC} Partial success ($SUCCESS/$TOTAL checks passed)"
    echo "   Review errors above and check DJI Assistant 2 configuration"
else
    echo -e "${RED}✗${NC} Communication not successful ($SUCCESS/$TOTAL checks passed)"
    echo "   Please review the troubleshooting guide in PSDK_SETUP_GUIDE.md"
fi

echo ""

# Show recent log entries
echo "=== Recent Log Entries ==="
tail -20 "$LATEST_LOG" | sed 's/^/   /'

echo ""
echo "Full log file: $LATEST_LOG"
echo "View with: tail -f $LATEST_LOG"

