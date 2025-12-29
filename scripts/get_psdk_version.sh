#!/bin/bash
#
# get_psdk_version.sh
# Script to retrieve the DJI Payload SDK version installed on the Raspberry Pi
#
# This script checks the PSDK version by:
# 1. Looking for the version in the most recent PSDK log file
# 2. If no log file exists, initializing PSDK briefly to get the version
#
# Usage:
#   ./get_psdk_version.sh
#   ./get_psdk_version.sh --verbose
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/scout_beacon/build"
LOG_DIR="$BUILD_DIR/Logs/DJI"

VERBOSE=false

# Parse command line arguments
if [[ "$1" == "--verbose" ]] || [[ "$1" == "-v" ]]; then
    VERBOSE=true
fi

# Function to print verbose output
print_verbose() {
    if [[ "$VERBOSE" == "true" ]]; then
        echo "$@"
    fi
}

# Function to extract version from log line
extract_version() {
    local line="$1"
    # Extract version string (e.g., "V3.14.0-beta.0-build.2296")
    echo "$line" | sed -n 's/.*Payload SDK Version : \(V[0-9.]*-[^ ]*\).*/\1/p'
}

echo "=== DJI Payload SDK Version Check ==="
echo ""

# Method 1: Check most recent log file
print_verbose "Checking for PSDK log files in: $LOG_DIR"

if [[ -d "$LOG_DIR" ]]; then
    # Find the most recent log file
    LATEST_LOG=$(find "$LOG_DIR" -name "psdk_log_*.txt" -type f -printf '%T@ %p\n' 2>/dev/null | sort -rn | head -1 | cut -d' ' -f2-)
    
    if [[ -n "$LATEST_LOG" ]] && [[ -f "$LATEST_LOG" ]]; then
        print_verbose "Found log file: $LATEST_LOG"
        
        # Extract version from log file
        VERSION_LINE=$(grep "Payload SDK Version" "$LATEST_LOG" | head -1)
        
        if [[ -n "$VERSION_LINE" ]]; then
            VERSION=$(extract_version "$VERSION_LINE")
            
            if [[ -n "$VERSION" ]]; then
                echo "PSDK Version (from logs): $VERSION"
                
                # Extract build date if available
                BUILD_DATE=$(echo "$VERSION_LINE" | sed -n 's/.*build\.\([0-9]*\) \([A-Za-z]* [0-9]* [0-9:]*\).*/\2/p')
                if [[ -n "$BUILD_DATE" ]]; then
                    echo "Build Date: $BUILD_DATE"
                fi
                
                echo ""
                echo "Source: Log file ($(basename "$LATEST_LOG"))"
                exit 0
            fi
        fi
    fi
fi

print_verbose "No log file found or version not in log file"
print_verbose "Attempting to get version by initializing PSDK..."

# Method 2: Use dedicated version checker program (preferred method)
if [[ -f "$BUILD_DIR/bin/get_psdk_version" ]]; then
    print_verbose "Running get_psdk_version program..."
    
    # Run the version checker program
    # Use timeout to prevent hanging if there are issues
    VERSION=$(timeout 5 "$BUILD_DIR/bin/get_psdk_version" 2>/dev/null || echo "")
    
    if [[ -n "$VERSION" ]] && [[ "$VERSION" =~ ^V[0-9] ]]; then
        echo "PSDK Version (from program): $VERSION"
        echo ""
        echo "Source: PSDK version checker program"
        exit 0
    fi
fi

# Method 3: Run get_drone_heartbeat briefly to get version (fallback)
if [[ -f "$BUILD_DIR/bin/get_drone_heartbeat" ]]; then
    print_verbose "Running get_drone_heartbeat to get SDK version..."
    
    # Run the heartbeat script briefly and capture version
    # Use timeout to prevent hanging if drone is not connected
    OUTPUT=$(timeout 5 "$BUILD_DIR/bin/get_drone_heartbeat" 2>&1 || true)
    
    VERSION_LINE=$(echo "$OUTPUT" | grep "Payload SDK Version" | head -1)
    
    if [[ -n "$VERSION_LINE" ]]; then
        VERSION=$(extract_version "$VERSION_LINE")
        
        if [[ -n "$VERSION" ]]; then
            echo "PSDK Version (from initialization): $VERSION"
            echo ""
            echo "Source: PSDK initialization output"
            exit 0
        fi
    fi
fi

# Method 4: Check if we can find version in PSDK library files
print_verbose "Checking PSDK library files..."

# Check if PSDK directory exists (common locations)
PSDK_DIRS=(
    "/home/aviscout/MVP/Payload-SDK"
    "$PROJECT_ROOT/../Payload-SDK"
    "$PROJECT_ROOT/Payload-SDK"
)

for PSDK_DIR in "${PSDK_DIRS[@]}"; do
    if [[ -d "$PSDK_DIR" ]]; then
        print_verbose "Found PSDK directory: $PSDK_DIR"
        
        # Check for version in git tags or version files
        if [[ -d "$PSDK_DIR/.git" ]]; then
            # Try to get version from git tags
            cd "$PSDK_DIR"
            GIT_VERSION=$(git describe --tags --always 2>/dev/null || echo "")
            if [[ -n "$GIT_VERSION" ]]; then
                echo "PSDK Version (from git): $GIT_VERSION"
                echo ""
                echo "Source: Git repository at $PSDK_DIR"
                exit 0
            fi
        fi
        
        # Check for version file
        if [[ -f "$PSDK_DIR/VERSION" ]]; then
            VERSION=$(cat "$PSDK_DIR/VERSION" | head -1)
            if [[ -n "$VERSION" ]]; then
                echo "PSDK Version (from VERSION file): $VERSION"
                echo ""
                echo "Source: VERSION file at $PSDK_DIR"
                exit 0
            fi
        fi
    fi
done

# If we get here, we couldn't find the version
echo "ERROR: Could not determine PSDK version"
echo ""
echo "Tried methods:"
echo "  1. Checking log files in $LOG_DIR"
echo "  2. Running get_psdk_version program"
echo "  3. Running get_drone_heartbeat initialization"
echo "  4. Checking PSDK library directories"
echo ""
echo "Suggestions:"
echo "  - Run the application once to generate a log file"
echo "  - Ensure PSDK is properly installed"
echo "  - Check that log directory exists: $LOG_DIR"
exit 1
