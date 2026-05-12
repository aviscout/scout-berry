#!/bin/bash

###############################################################################
# @file    verify_connection.sh
# @brief   Connection verification script for Scout Beacon PSDK setup
#
# @details This script performs comprehensive checks to verify that all
#          hardware and software components are properly configured for
#          PSDK communication with the drone.
#
# @copyright (c) 2024 Scout Berry. All rights reserved.
#
# @usage   ./verify_connection.sh [--verbose]
#
###############################################################################

# Don't use set -e as it can cause issues with arithmetic operations

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Flags
VERBOSE=false
if [[ "$1" == "--verbose" ]] || [[ "$1" == "-v" ]]; then
    VERBOSE=true
fi

# Counters (initialize as integers)
declare -i PASSED=0
declare -i FAILED=0
declare -i WARNINGS=0

###############################################################################
# Helper Functions
###############################################################################

print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

print_pass() {
    echo -e "${GREEN}✓${NC} $1"
    ((PASSED++))
}

print_fail() {
    echo -e "${RED}✗${NC} $1"
    ((FAILED++))
}

print_warn() {
    echo -e "${YELLOW}⚠${NC} $1"
    ((WARNINGS++))
}

print_info() {
    if [[ "$VERBOSE" == "true" ]]; then
        echo -e "${BLUE}ℹ${NC} $1"
    fi
}

###############################################################################
# Check Functions
###############################################################################

check_uart_device() {
    print_header "UART Device Check"
    
    local uart_devices=()
    
    # Check for ttyUSB devices
    if ls /dev/ttyUSB* 1> /dev/null 2>&1; then
        while IFS= read -r line; do
            uart_devices+=("$line")
        done < <(ls /dev/ttyUSB*)
    fi
    
    # Check for ttyACM devices
    if ls /dev/ttyACM* 1> /dev/null 2>&1; then
        while IFS= read -r line; do
            uart_devices+=("$line")
        done < <(ls /dev/ttyACM*)
    fi
    
    if [[ ${#uart_devices[@]} -eq 0 ]]; then
        print_fail "No UART devices found (/dev/ttyUSB* or /dev/ttyACM*)"
        echo "   Expected: /dev/ttyUSB0 or /dev/ttyACM0"
        return 1
    fi
    
    print_pass "Found ${#uart_devices[@]} UART device(s)"
    
    for device in "${uart_devices[@]}"; do
        echo "   Device: $device"
        
        # Check permissions
        local perms=$(stat -c "%a" "$device" 2>/dev/null || echo "000")
        if [[ "$perms" == "666" ]] || [[ "$perms" == "777" ]]; then
            print_pass "   Permissions: $perms (readable/writable)"
        else
            print_warn "   Permissions: $perms (may need: sudo chmod 666 $device)"
        fi
        
        # Check if device is accessible
        if [[ -r "$device" ]] && [[ -w "$device" ]]; then
            print_pass "   Device is readable and writable"
        else
            print_fail "   Device is not accessible (check permissions)"
        fi
        
        # Check if device is in use
        if lsof "$device" 1> /dev/null 2>&1; then
            print_warn "   Device is in use by another process"
            lsof "$device" | head -3
        else
            print_pass "   Device is not in use"
        fi
        
        # Check device type
        local device_type=$(stat -c "%F" "$device" 2>/dev/null || echo "unknown")
        print_info "   Type: $device_type"
    done
    
    return 0
}

check_usb_devices() {
    print_header "USB Device Check"
    
    if ! command -v lsusb &> /dev/null; then
        print_warn "lsusb not available, skipping USB device check"
        return 0
    fi
    
    # Check for CP210x (common UART bridge)
    if lsusb | grep -q "CP210x\|10c4:ea60"; then
        print_pass "CP210x UART Bridge detected"
        lsusb | grep "CP210x\|10c4:ea60"
    else
        print_warn "CP210x UART Bridge not detected"
        print_info "Other USB devices:"
        lsusb | head -5
    fi
    
    # Check for DJI devices (if any)
    if lsusb | grep -qi "dji"; then
        print_info "DJI-related USB device detected:"
        lsusb | grep -i "dji"
    fi
    
    return 0
}

check_psdk_config() {
    print_header "PSDK Configuration Check"
    
    local config_file="../application/dji_sdk_app_info.h"
    
    if [[ ! -f "$config_file" ]]; then
        print_fail "PSDK config file not found: $config_file"
        return 1
    fi
    
    print_pass "PSDK config file found"
    
    # Check for required configuration values
    if grep -q "USER_APP_NAME" "$config_file"; then
        local app_name=$(grep "USER_APP_NAME" "$config_file" | head -1 | sed 's/.*"\(.*\)".*/\1/')
        print_pass "App Name: $app_name"
    fi
    
    if grep -q "USER_APP_ID" "$config_file"; then
        local app_id=$(grep "USER_APP_ID" "$config_file" | head -1 | sed 's/.*"\(.*\)".*/\1/')
        print_pass "App ID: $app_id"
    fi
    
    if grep -q "USER_BAUD_RATE" "$config_file"; then
        local baud_rate=$(grep "USER_BAUD_RATE" "$config_file" | head -1 | sed 's/.*"\(.*\)".*/\1/')
        print_pass "Baud Rate: $baud_rate"
    fi
    
    return 0
}

check_build_artifacts() {
    print_header "Build Artifacts Check"
    
    local heartbeat_bin="./bin/get_drone_heartbeat"
    local main_bin="./bin/scout_beacon"
    
    if [[ -f "$heartbeat_bin" ]]; then
        print_pass "Heartbeat script executable found"
        local size=$(stat -c "%s" "$heartbeat_bin" 2>/dev/null || echo "0")
        print_info "   Size: $((size / 1024)) KB"
    else
        print_fail "Heartbeat script executable not found: $heartbeat_bin"
        echo "   Run: cd build && cmake .. && make get_drone_heartbeat"
    fi
    
    if [[ -f "$main_bin" ]]; then
        print_pass "Main application executable found"
    else
        print_warn "Main application executable not found (optional for connection test)"
    fi
    
    return 0
}

check_log_directory() {
    print_header "Log Directory Check"
    
    local log_dir="./Logs/DJI"
    
    if [[ -d "$log_dir" ]]; then
        print_pass "Log directory exists: $log_dir"
        
        local log_count=$(ls -1 "$log_dir"/*.txt 2>/dev/null | wc -l)
        if [[ $log_count -gt 0 ]]; then
            print_info "   Found $log_count log file(s)"
            local latest_log=$(ls -t "$log_dir"/*.txt 2>/dev/null | head -1)
            if [[ -n "$latest_log" ]]; then
                local log_size=$(stat -c "%s" "$latest_log" 2>/dev/null || echo "0")
                print_info "   Latest: $(basename "$latest_log") ($((log_size / 1024)) KB)"
            fi
        fi
    else
        print_warn "Log directory does not exist: $log_dir"
        echo "   Will be created automatically when script runs"
    fi
    
    return 0
}

check_system_permissions() {
    print_header "System Permissions Check"
    
    # Check if user is in dialout group (for UART access)
    if groups | grep -q "dialout"; then
        print_pass "User is in 'dialout' group"
    else
        print_warn "User is not in 'dialout' group"
        echo "   Add user: sudo usermod -a -G dialout $USER"
        echo "   Then log out and back in"
    fi
    
    # Check sudo access
    if sudo -n true 2>/dev/null; then
        print_pass "Sudo access available (passwordless)"
    elif sudo -v 2>/dev/null; then
        print_pass "Sudo access available (password required)"
    else
        print_fail "Sudo access not available (required for UART access)"
    fi
    
    return 0
}

test_uart_connection() {
    print_header "UART Connection Test"
    
    local uart_device=""
    
    # Find first available UART device
    if [[ -c "/dev/ttyUSB0" ]]; then
        uart_device="/dev/ttyUSB0"
    elif [[ -c "/dev/ttyACM0" ]]; then
        uart_device="/dev/ttyACM0"
    else
        print_fail "No UART device available for testing"
        return 1
    fi
    
    print_info "Testing device: $uart_device"
    
    # Test if we can open the device
    if sudo timeout 0.1 cat "$uart_device" > /dev/null 2>&1; then
        print_pass "Device is readable"
    else
        print_warn "Device read test inconclusive (may be normal if drone not connected)"
    fi
    
    # Check current baud rate
    if command -v stty &> /dev/null; then
        local current_baud=$(sudo stty -F "$uart_device" 2>/dev/null | grep -oP 'speed \K[0-9]+' || echo "unknown")
        print_info "Current baud rate: $current_baud"
        
        # Expected baud rate from config
        local expected_baud="921600"
        if [[ "$current_baud" != "$expected_baud" ]]; then
            print_info "   Expected: $expected_baud (will be set by PSDK)"
        fi
    fi
    
    return 0
}

run_quick_psdk_test() {
    print_header "Quick PSDK Connection Test"
    
    local heartbeat_bin="./bin/get_drone_heartbeat"
    
    if [[ ! -f "$heartbeat_bin" ]]; then
        print_warn "Heartbeat executable not found, skipping PSDK test"
        echo "   Build it first: cd build && cmake .. && make get_drone_heartbeat"
        return 0
    fi
    
    echo "Running quick connection test (5 second timeout)..."
    echo ""
    
    if timeout 5 sudo "$heartbeat_bin" 2>&1 | head -20; then
        local exit_code=${PIPESTATUS[0]}
        if [[ $exit_code -eq 124 ]]; then
            print_warn "PSDK test timed out (drone may not be responding)"
            echo "   This is normal if:"
            echo "   - Drone is not powered on"
            echo "   - PSDK mode is not enabled"
            echo "   - Physical connection issue"
        elif [[ $exit_code -eq 0 ]]; then
            print_pass "PSDK connection successful!"
        else
            print_warn "PSDK test completed with exit code: $exit_code"
        fi
    else
        print_warn "PSDK test failed or timed out"
    fi
    
    return 0
}

print_summary() {
    print_header "Verification Summary"
    
    echo "Passed:  $PASSED"
    echo "Failed:  $FAILED"
    echo "Warnings: $WARNINGS"
    echo ""
    
    if [[ $FAILED -eq 0 ]]; then
        echo -e "${GREEN}✓ All critical checks passed!${NC}"
        if [[ $WARNINGS -gt 0 ]]; then
            echo -e "${YELLOW}⚠ Some warnings were found - review above${NC}"
        fi
        echo ""
        echo "Next steps:"
        echo "1. Ensure drone is powered ON"
        echo "2. Verify PSDK mode is enabled in DJI Assistant 2"
        echo "3. Run: sudo ./bin/get_drone_heartbeat"
    else
        echo -e "${RED}✗ Some checks failed - please fix issues above${NC}"
        echo ""
        echo "Common fixes:"
        echo "- Fix UART permissions: sudo chmod 666 /dev/ttyUSB0"
        echo "- Add user to dialout group: sudo usermod -a -G dialout $USER"
        echo "- Rebuild if needed: cd build && cmake .. && make get_drone_heartbeat"
    fi
}

###############################################################################
# Main Execution
###############################################################################

main() {
    echo ""
    echo -e "${BLUE}╔═══════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║     Scout Beacon - PSDK Connection Verification         ║${NC}"
    echo -e "${BLUE}║     Copyright (c) 2024 Scout Berry                      ║${NC}"
    echo -e "${BLUE}╚═══════════════════════════════════════════════════════════╝${NC}"
    echo ""
    
    # Change to script directory
    cd "$(dirname "$0")/../build" || {
        echo "Error: Cannot access build directory"
        exit 1
    }
    
    # Run all checks
    check_uart_device
    check_usb_devices
    check_psdk_config
    check_build_artifacts
    check_log_directory
    check_system_permissions
    test_uart_connection
    
    # Ask before running PSDK test (it may take time)
    echo ""
    read -p "Run quick PSDK connection test? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        run_quick_psdk_test
    else
        print_info "Skipping PSDK connection test"
    fi
    
    # Print summary
    print_summary
}

# Run main function
main "$@"
