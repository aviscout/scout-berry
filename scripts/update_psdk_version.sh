#!/bin/bash
#
# PSDK Version Update Script
# Updates DJI Payload SDK to the latest or specified version
#
# Usage:
#   ./update_psdk_version.sh [version]
#   ./update_psdk_version.sh 3.14.0
#   ./update_psdk_version.sh latest
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PSDK_DIR="/home/aviscout/MVP/Payload-SDK"
BUILD_DIR="/home/aviscout/MVP/scout-berry/scout_beacon/build"
BACKUP_DIR="/home/aviscout/MVP/Payload-SDK-backups"

# Print functions
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

# Check if running from correct directory
check_prerequisites() {
    print_header "Checking Prerequisites"
    
    if [ ! -d "$PSDK_DIR" ]; then
        print_error "PSDK directory not found: $PSDK_DIR"
        exit 1
    fi
    
    if [ ! -d "$PSDK_DIR/.git" ]; then
        print_error "PSDK directory is not a git repository"
        exit 1
    fi
    
    print_success "Prerequisites check passed"
}

# Get current version
get_current_version() {
    cd "$PSDK_DIR"
    CURRENT_VERSION=$(git describe --tags 2>/dev/null || echo "unknown")
    print_info "Current PSDK version: $CURRENT_VERSION"
    echo "$CURRENT_VERSION"
}

# Get latest available version
get_latest_version() {
    cd "$PSDK_DIR"
    print_info "Fetching latest version information..."
    git fetch --tags --quiet 2>/dev/null || true
    LATEST_VERSION=$(git tag --sort=-version:refname | head -1)
    print_info "Latest available version: $LATEST_VERSION"
    echo "$LATEST_VERSION"
}

# List available versions
list_versions() {
    print_header "Available PSDK Versions"
    cd "$PSDK_DIR"
    git fetch --tags --quiet 2>/dev/null || true
    echo "Recent versions:"
    git tag --sort=-version:refname | head -10
    echo ""
}

# Backup current version
backup_current() {
    print_header "Backing Up Current Version"
    
    mkdir -p "$BACKUP_DIR"
    BACKUP_NAME="Payload-SDK-backup-$(date +%Y%m%d-%H%M%S)"
    BACKUP_PATH="$BACKUP_DIR/$BACKUP_NAME"
    
    print_info "Creating backup: $BACKUP_PATH"
    
    cd "$PSDK_DIR"
    CURRENT_VERSION=$(get_current_version)
    
    # Create a git tag for easy rollback
    git tag "backup-$CURRENT_VERSION-$(date +%Y%m%d)" 2>/dev/null || true
    
    # Also create a directory backup
    print_info "Copying files to backup directory..."
    cp -r "$PSDK_DIR" "$BACKUP_PATH" 2>/dev/null || {
        print_warning "Could not create full backup (may be large), git tag created instead"
    }
    
    print_success "Backup created: $BACKUP_PATH"
    print_info "To rollback: cd $PSDK_DIR && git checkout backup-$CURRENT_VERSION-*"
}

# Update to specified version
update_version() {
    local TARGET_VERSION=$1
    
    print_header "Updating to Version $TARGET_VERSION"
    
    cd "$PSDK_DIR"
    
    # Fetch latest tags
    print_info "Fetching tags from remote..."
    git fetch --tags --quiet 2>/dev/null || true
    
    # Check if version exists
    if ! git tag | grep -q "^$TARGET_VERSION$"; then
        print_error "Version $TARGET_VERSION not found"
        print_info "Available versions:"
        git tag --sort=-version:refname | head -10
        exit 1
    fi
    
    # Checkout the version
    print_info "Checking out version $TARGET_VERSION..."
    git checkout "$TARGET_VERSION" 2>/dev/null || {
        print_error "Failed to checkout version $TARGET_VERSION"
        exit 1
    }
    
    print_success "Successfully updated to version $TARGET_VERSION"
    
    # Verify library files exist
    print_info "Verifying library files..."
    if [ -f "$PSDK_DIR/psdk_lib/lib/aarch64-linux-gnu-gcc/libpayloadsdk.a" ]; then
        print_success "Library files verified"
    else
        print_warning "Library file not found - may need to rebuild SDK"
    fi
}

# Rebuild application
rebuild_application() {
    print_header "Rebuilding Application"
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found: $BUILD_DIR"
        return 1
    fi
    
    cd "$BUILD_DIR"
    
    print_info "Cleaning previous build..."
    rm -rf CMakeFiles CMakeCache.txt *.o bin/* 2>/dev/null || true
    
    print_info "Running cmake..."
    cmake .. || {
        print_error "CMake configuration failed"
        return 1
    }
    
    print_info "Building application..."
    make -j$(nproc) || {
        print_error "Build failed"
        return 1
    }
    
    print_success "Application rebuilt successfully"
}

# Verify new version
verify_version() {
    print_header "Verifying New Version"
    
    if [ ! -f "$BUILD_DIR/bin/get_drone_heartbeat" ]; then
        print_warning "get_drone_heartbeat executable not found - skipping version check"
        return
    fi
    
    print_info "Checking SDK version in application..."
    VERSION_OUTPUT=$("$BUILD_DIR/bin/get_drone_heartbeat" 2>&1 | grep "Payload SDK Version" || echo "")
    
    if [ -n "$VERSION_OUTPUT" ]; then
        print_success "Version check:"
        echo "  $VERSION_OUTPUT"
    else
        print_warning "Could not extract version from application output"
    fi
}

# Main execution
main() {
    print_header "PSDK Version Update Script"
    
    # Parse arguments
    TARGET_VERSION=${1:-"latest"}
    
    # Check prerequisites
    check_prerequisites
    
    # Show current version
    CURRENT=$(get_current_version)
    
    # Determine target version
    if [ "$TARGET_VERSION" = "latest" ]; then
        TARGET_VERSION=$(get_latest_version)
    fi
    
    # Show available versions if requested
    if [ "$TARGET_VERSION" = "list" ]; then
        list_versions
        exit 0
    fi
    
    # Check if update is needed
    if [ "$CURRENT" = "$TARGET_VERSION" ]; then
        print_warning "Already on version $TARGET_VERSION"
        read -p "Continue anyway? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 0
        fi
    fi
    
    # Backup current version
    backup_current
    
    # Update version
    update_version "$TARGET_VERSION"
    
    # Rebuild application
    read -p "Rebuild application now? (Y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Nn]$ ]]; then
        rebuild_application
        verify_version
    else
        print_info "Skipping rebuild - run manually:"
        print_info "  cd $BUILD_DIR"
        print_info "  rm -rf CMakeFiles CMakeCache.txt *.o bin/*"
        print_info "  cmake .."
        print_info "  make -j\$(nproc)"
    fi
    
    print_header "Update Complete"
    print_success "PSDK updated to version $TARGET_VERSION"
    print_info "Next steps:"
    print_info "  1. Test connection: cd $BUILD_DIR && ./bin/get_drone_heartbeat"
    print_info "  2. Review release notes for breaking changes"
    print_info "  3. Update application code if needed"
}

# Run main function
main "$@"
