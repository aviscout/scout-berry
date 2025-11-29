# =============================================================================
# Scout Beacon Application - CMake Build Configuration
# =============================================================================
# 
# This CMakeLists.txt file configures the build system for the Scout Beacon
# application, which integrates avalanche beacon detection with DJI PSDK for
# autonomous drone rescue operations.
#
# Features:
# - Cross-platform build support (x86_64, aarch64)
# - DJI PSDK integration with proper library linking
# - GPIO interface using lgpio library
# - Optional dependencies (OPUS, LIBUSB)
# - Comprehensive error handling and validation
#
# Author: Scout Berry Development Team
# Date: 2024
# Version: 1.0.0
#
# =============================================================================

cmake_minimum_required(VERSION 3.5)
project(scout_beacon C)

# =============================================================================
# Compiler Configuration
# =============================================================================
# 
# Configure compiler flags and settings for optimal performance and compatibility
# with the DJI PSDK and GPIO interface requirements.
#
# Compiler Flags:
# - -pthread: Enable POSIX threads support for multi-threading
# - -std=gnu99: Use GNU C99 standard for enhanced features
# - -std=c++11: Use C++11 standard for C++ compatibility
# - -D_GNU_SOURCE: Enable GNU extensions for enhanced functionality
#
# =============================================================================

set(CMAKE_C_FLAGS "-pthread -std=gnu99")
set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")
set(CMAKE_EXE_LINKER_FLAGS "-pthread")
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
add_definitions(-D_GNU_SOURCE)

if (NOT USE_SYSTEM_ARCH)
    add_definitions(-DSYSTEM_ARCH_LINUX)
endif ()

if (BUILD_TEST_CASES_ON MATCHES TRUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
endif ()

set(PACKAGE_NAME payloadsdk)

# =============================================================================
# Platform Architecture Detection
# =============================================================================
# 
# Automatically detect the target platform architecture and configure the
# appropriate toolchain and library paths for DJI PSDK compatibility.
#
# Supported Architectures:
# - x86_64: Intel/AMD 64-bit processors (development machines)
# - aarch64: ARM 64-bit processors (Raspberry Pi 4, Jetson)
#
# Toolchain Selection:
# - x86_64-linux-gnu-gcc: For x86_64 development and testing
# - aarch64-linux-gnu-gcc: For ARM64 production deployment
#
# =============================================================================

## "uname -m" to auto distinguish platform architecture
execute_process(COMMAND uname -m
        OUTPUT_VARIABLE DEVICE_SYSTEM_ID)

if (DEVICE_SYSTEM_ID MATCHES x86_64)
    set(TOOLCHAIN_NAME x86_64-linux-gnu-gcc)
    add_definitions(-DPLATFORM_ARCH_x86_64=1)
elseif (DEVICE_SYSTEM_ID MATCHES aarch64)
    set(TOOLCHAIN_NAME aarch64-linux-gnu-gcc)
    add_definitions(-DPLATFORM_ARCH_aarch64=1)
elseif (DEVICE_SYSTEM_ID MATCHES arm64)
    set(TOOLCHAIN_NAME aarch64-linux-gnu-gcc)
    add_definitions(-DPLATFORM_ARCH_aarch64=1)
else ()
    message(STATUS "Platform: ${DEVICE_SYSTEM_ID}")
    message(STATUS "Using default x86_64 toolchain")
    set(TOOLCHAIN_NAME x86_64-linux-gnu-gcc)
    add_definitions(-DPLATFORM_ARCH_x86_64=1)
endif ()

# =============================================================================
# Source File Collection
# =============================================================================
# 
# Collect all source files from the project structure for compilation.
# The files are organized into logical modules for better maintainability.
#
# Source Categories:
# - MODULE_COMMON_SRC: Common utility functions and shared code
# - MODULE_HAL_SRC: Hardware abstraction layer (UART, Network, USB)
# - MODULE_APP_SRC: Application-specific source files
# - MODULE_SAMPLE_SRC: DJI PSDK sample code and examples
#
# =============================================================================

# Source files - manually specify to avoid Linux-specific files
set(MODULE_COMMON_SRC
    common/lgpio_stub.c
    common/monitor/sys_monitor.c
    common/osal/osal_macos_stub.c
    common/utils/util_misc.c
    common/psdk_stub.c
)
# Use HAL stubs for macOS development
set(MODULE_HAL_SRC
    hal/hal_uart_stub.c
    hal/hal_network_stub.c
    hal/hal_usb_bulk_stub.c
)
file(GLOB_RECURSE MODULE_APP_SRC application/*.c)

# =============================================================================
# Include Directories Configuration
# =============================================================================
# 
# Configure include directories for header file resolution during compilation.
# These directories contain the necessary header files for the application.
#
# Include Directories:
# - ../../../module_sample: DJI PSDK sample code headers
# - ../common: Common utility headers and shared definitions
# - application: Application-specific header files
# - ../../../../../psdk_lib/include: DJI PSDK library headers
#
# =============================================================================

# Include directories
include_directories(common)
include_directories(common/osal)
include_directories(common/monitor)
include_directories(common/utils)
include_directories(application)

# =============================================================================
# DJI PSDK Library Configuration
# =============================================================================
# 
# Configure DJI PSDK library paths and linking for the target architecture.
# The library paths are automatically selected based on the detected platform.
#
# Library Configuration:
# - Include path: PSDK library header files
# - Link directory: Architecture-specific library directory
# - Library file: Static library for linking
#
# =============================================================================

# PSDK library paths - using stubs for macOS development
include_directories(/Users/mikeneqa/Dev/Payload-SDK/psdk_lib/include)
# Note: PSDK library linking disabled for macOS development - using stubs instead

# Output directory
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif ()

# =============================================================================
# Executable Creation
# =============================================================================
# 
# Create the main executable for the Scout Beacon application by combining
# all source files from the different modules into a single binary.
#
# Executable Components:
# - MODULE_APP_SRC: Application-specific source files (main.c, beacon_gpio.c)
# - MODULE_SAMPLE_SRC: DJI PSDK sample code and examples
# - MODULE_COMMON_SRC: Common utility functions and shared code
# - MODULE_HAL_SRC: Hardware abstraction layer (UART, Network, USB)
#
# =============================================================================

# Create executable
add_executable(${PROJECT_NAME}
        ${MODULE_APP_SRC}
        ${MODULE_SAMPLE_SRC}
        ${MODULE_COMMON_SRC}
        ${MODULE_HAL_SRC})

# =============================================================================
# GPIO Library Configuration
# =============================================================================
# 
# Configure and link the lgpio library for GPIO interface operations.
# The lgpio library provides the interface for reading GPIO pins on
# Raspberry Pi for beacon detection and distance measurement.
#
# Library Features:
# - GPIO pin reading and writing
# - Edge detection for signal changes
# - Pull-up/pull-down resistor configuration
# - Multiplexed pin operations
#
# =============================================================================

# Use lgpio stub for development (lgpio only available on Raspberry Pi)
message(STATUS "Using lgpio stub for development")

# =============================================================================
# OPUS Audio Codec Library Configuration
# =============================================================================
# 
# Configure and link the OPUS audio codec library for audio processing
# capabilities in the DJI PSDK. OPUS provides high-quality audio compression
# and is used for audio transmission and processing.
#
# Library Features:
# - High-quality audio compression
# - Low latency audio processing
# - Wide range of bitrates and sample rates
# - Real-time audio encoding/decoding
#
# =============================================================================

# Find and link OPUS library
find_library(OPUS_LIBRARY opus
    PATHS
    /usr/local/lib
    /usr/lib
    /usr/lib/aarch64-linux-gnu
)

find_path(OPUS_INCLUDE_DIR opus.h
    PATHS
    /usr/local/include/opus
    /usr/include/opus
    /opt/local/include
    /opt/include
)

if (OPUS_LIBRARY AND OPUS_INCLUDE_DIR)
    message(STATUS "Found OPUS installed in the system")
    message(STATUS " - Includes: ${OPUS_INCLUDE_DIR}")
    message(STATUS " - Libraries: ${OPUS_LIBRARY}")

    add_definitions(-DOPUS_INSTALLED)
    target_link_libraries(${PROJECT_NAME} ${OPUS_LIBRARY})
else ()
    message(STATUS "Cannot Find OPUS")
endif ()

# =============================================================================
# LIBUSB Library Configuration
# =============================================================================
# 
# Configure and link the LIBUSB library for USB device communication
# capabilities in the DJI PSDK. LIBUSB provides the interface for USB
# device access and communication with the drone's E-Port interface.
#
# Library Features:
# - USB device enumeration and access
# - USB bulk transfer operations
# - USB device descriptor reading
# - Cross-platform USB support
#
# =============================================================================

# Find and link LIBUSB library
find_library(LIBUSB_LIBRARY usb-1.0
    PATHS
    /usr/local/lib
    /usr/lib
    /usr/lib/aarch64-linux-gnu
)

find_path(LIBUSB_INCLUDE_DIR libusb.h
    PATHS
    /usr/local/include
    /usr/include
    /opt/local/include
    /opt/include
)

if (LIBUSB_LIBRARY AND LIBUSB_INCLUDE_DIR)
    message(STATUS "Found LIBUSB installed in the system")
    message(STATUS " - Includes: ${LIBUSB_INCLUDE_DIR}")
    message(STATUS " - Libraries: ${LIBUSB_LIBRARY}")

    add_definitions(-DLIBUSB_INSTALLED)
    target_link_libraries(${PROJECT_NAME} ${LIBUSB_LIBRARY})
else ()
    message(STATUS "Cannot Find LIBUSB")
endif ()

# =============================================================================
# System Library Configuration
# =============================================================================
# 
# Link essential system libraries required for the application to function
# properly. These libraries provide basic system functionality and are
# required for the application to run correctly.
#
# System Libraries:
# - m: Math library for mathematical operations
#
# =============================================================================

# Link math library
target_link_libraries(${PROJECT_NAME} m)

# =============================================================================
# Build Configuration and Information
# =============================================================================
# 
# Configure build process and display comprehensive build information
# for debugging and verification purposes. This section provides
# detailed information about the build configuration.
#
# Build Information:
# - Project name and version
# - Target architecture and toolchain
# - Output directory and executable path
# - Library dependencies and status
#
# =============================================================================

# Add custom command for build
add_custom_command(TARGET ${PROJECT_NAME}
        PRE_LINK COMMAND cmake ..
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# Print build information
message(STATUS "=== Scout Beacon Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Architecture: ${DEVICE_SYSTEM_ID}")
message(STATUS "Toolchain: ${TOOLCHAIN_NAME}")
message(STATUS "Output: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "==========================================")
