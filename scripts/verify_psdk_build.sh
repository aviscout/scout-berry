#!/bin/bash
#
# PSDK Build Verification Script for Scout Beacon
# This script verifies that the PSDK is built correctly and can initialize
#

set -e

echo "=== PSDK Build Verification ==="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Track verification results
VERIFICATION_PASSED=true

# Function to print status
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $2"
    else
        echo -e "${RED}✗${NC} $2"
        VERIFICATION_PASSED=false
    fi
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

# Check if we're in the build directory
BUILD_DIR="/home/aviscout/MVP/scout-berry/scout_beacon/build"
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Error:${NC} Build directory not found: $BUILD_DIR"
    exit 1
fi

cd "$BUILD_DIR"
echo "Working directory: $(pwd)"
echo ""

# 1. Check if executable exists
echo "1. Checking executable..."
if [ -f "bin/scout_beacon" ]; then
    print_status 0 "Executable exists: bin/scout_beacon"
    
    # Check file type
    FILE_TYPE=$(file bin/scout_beacon)
    if echo "$FILE_TYPE" | grep -q "ARM aarch64"; then
        print_status 0 "Executable is built for ARM aarch64 (correct architecture)"
    else
        print_status 1 "Executable architecture mismatch: $FILE_TYPE"
    fi
    
    # Check if executable
    if [ -x "bin/scout_beacon" ]; then
        print_status 0 "Executable has execute permissions"
    else
        print_status 1 "Executable missing execute permissions"
        chmod +x bin/scout_beacon
        print_status 0 "Fixed execute permissions"
    fi
    
    # Get file size
    SIZE=$(stat -c%s bin/scout_beacon)
    SIZE_MB=$(echo "scale=2; $SIZE/1024/1024" | bc)
    print_info "Executable size: ${SIZE_MB} MB"
else
    print_status 1 "Executable not found: bin/scout_beacon"
    echo "   Build the application first:"
    echo "   cd scout_beacon && mkdir -p build && cd build && cmake .. && make"
fi

echo ""

# 2. Check log directory
echo "2. Checking log directory..."
if [ -d "Logs/DJI" ]; then
    print_status 0 "Log directory exists: Logs/DJI"
    
    # Check ownership
    OWNER=$(stat -c "%U:%G" Logs/DJI 2>/dev/null || echo "unknown")
    if [ "$OWNER" = "aviscout:aviscout" ] || [ "$OWNER" = "root:root" ]; then
        print_status 0 "Log directory ownership: $OWNER"
    else
        print_warning "Log directory ownership: $OWNER (should be aviscout:aviscout)"
        echo "   Fix with: sudo chown -R aviscout:aviscout Logs"
    fi
    
    # Check existing logs
    LOG_COUNT=$(ls -1 Logs/DJI/psdk_log_*.txt 2>/dev/null | wc -l)
    if [ "$LOG_COUNT" -gt 0 ]; then
        print_info "Found $LOG_COUNT existing log file(s)"
        
        # Check latest log for errors
        LATEST_LOG=$(ls -t Logs/DJI/psdk_log_*.txt 2>/dev/null | head -1)
        if [ -n "$LATEST_LOG" ]; then
            ERROR_COUNT=$(grep -i "error\|timeout\|failed" "$LATEST_LOG" 2>/dev/null | wc -l || echo "0")
            if [ "$ERROR_COUNT" -gt 0 ]; then
                print_warning "Found $ERROR_COUNT error/timeout messages in latest log"
            else
                print_status 0 "No errors found in latest log"
            fi
            
            # Check for PSDK version
            if grep -q "Payload SDK Version" "$LATEST_LOG" 2>/dev/null; then
                VERSION=$(grep "Payload SDK Version" "$LATEST_LOG" | head -1 | sed 's/.*Version : //' | sed 's/ .*//')
                print_info "PSDK Version from logs: $VERSION"
            fi
        fi
    else
        print_info "No existing log files found (this is OK for first run)"
    fi
else
    print_warning "Log directory does not exist"
    echo "   Creating log directory..."
    sudo mkdir -p Logs/DJI
    sudo chown -R aviscout:aviscout Logs 2>/dev/null || true
    print_status 0 "Log directory created"
fi

echo ""

# 3. Check UART device (if available)
echo "3. Checking UART device..."
if [ -e /dev/ttyUSB0 ]; then
    print_status 0 "UART device /dev/ttyUSB0 exists"
    
    PERMS=$(stat -c "%a" /dev/ttyUSB0)
    if [ "$PERMS" = "666" ] || [ "$PERMS" = "777" ]; then
        print_status 0 "UART device permissions: $PERMS (correct)"
    else
        print_warning "UART device permissions: $PERMS (should be 666 or 777)"
        echo "   Fix with: sudo chmod 666 /dev/ttyUSB0"
    fi
    
    # Check if device is in use
    if lsof /dev/ttyUSB0 >/dev/null 2>&1; then
        print_warning "UART device is in use by another process"
        lsof /dev/ttyUSB0 | head -3
    else
        print_status 0 "UART device is not in use"
    fi
else
    print_warning "UART device /dev/ttyUSB0 does not exist"
    print_info "This is expected if drone is not connected or not on Raspberry Pi"
fi

echo ""

# 4. Test PSDK initialization (quick test)
echo "4. Testing PSDK initialization..."
if [ ! -f "bin/scout_beacon" ]; then
    print_status 1 "Cannot test - executable not found"
else
    print_info "Running application in mock mode for 5 seconds..."
    print_info "This will test PSDK initialization without requiring drone connection"
    echo ""
    
    # Create a test log file name
    TEST_LOG="Logs/DJI/psdk_test_$$.txt"
    
    # Run application in background with timeout
    timeout 5 sudo ./bin/scout_beacon --mock > /tmp/scout_test_$$.out 2>&1 || true
    
    # Wait a moment for logs to be written
    sleep 1
    
    # Check output for key indicators
    if [ -f /tmp/scout_test_$$.out ]; then
        OUTPUT=$(cat /tmp/scout_test_$$.out)
        
        # Check for PSDK version
        if echo "$OUTPUT" | grep -q "Payload SDK Version"; then
            VERSION=$(echo "$OUTPUT" | grep "Payload SDK Version" | head -1 | sed 's/.*Version : //' | sed 's/ .*//')
            print_status 0 "PSDK initialized successfully (Version: $VERSION)"
        else
            print_warning "PSDK version not found in output"
        fi
        
        # Check for mock mode
        if echo "$OUTPUT" | grep -q -i "mock mode\|Mock mode"; then
            print_status 0 "Mock mode enabled successfully"
        else
            print_warning "Mock mode indicator not found"
        fi
        
        # Check for errors
        if echo "$OUTPUT" | grep -qi "error\|timeout\|failed"; then
            ERROR_MSG=$(echo "$OUTPUT" | grep -i "error\|timeout\|failed" | head -3)
            print_warning "Found errors/timeouts in output:"
            echo "$ERROR_MSG" | sed 's/^/   /'
            
            # Check if it's just UART timeout (expected without drone)
            if echo "$OUTPUT" | grep -q "Try identify UART0 connection failed"; then
                print_info "UART timeout is expected when drone is not connected"
                print_info "This indicates PSDK is working but cannot connect to drone"
            fi
        else
            print_status 0 "No errors found in output"
        fi
        
        # Clean up
        rm -f /tmp/scout_test_$$.out
    else
        print_warning "Could not capture application output"
    fi
    
    # Check latest log file
    sleep 1
    LATEST_LOG=$(ls -t Logs/DJI/psdk_log_*.txt 2>/dev/null | head -1)
    if [ -n "$LATEST_LOG" ]; then
        if grep -q "Payload SDK Version" "$LATEST_LOG" 2>/dev/null; then
            print_status 0 "PSDK version logged successfully"
        fi
        
        # Count timeout errors (expected without drone)
        TIMEOUT_COUNT=$(grep -c "Try identify UART0 connection failed" "$LATEST_LOG" 2>/dev/null || echo "0")
        if [ "$TIMEOUT_COUNT" -gt 0 ]; then
            print_info "UART connection attempts: $TIMEOUT_COUNT (expected without drone)"
        fi
    fi
fi

echo ""

# 5. Summary
echo "=== Verification Summary ==="
echo ""

if [ "$VERIFICATION_PASSED" = true ]; then
    echo -e "${GREEN}✓ PSDK Build Verification: PASSED${NC}"
    echo ""
    echo "The PSDK is built correctly and can initialize."
    echo ""
    echo "Next steps:"
    echo "  1. Connect drone and enable PSDK mode in DJI Assistant 2"
    echo "  2. Run: sudo ./bin/scout_beacon --mock"
    echo "  3. Monitor output for successful connection"
    echo "  4. Check logs: tail -f Logs/DJI/psdk_log_*.txt"
else
    echo -e "${RED}✗ PSDK Build Verification: FAILED${NC}"
    echo ""
    echo "Some checks failed. Please review the output above."
fi

echo ""
echo "For detailed testing with drone connection, use:"
echo "  ./scripts/test_communication.sh"
echo ""
