cmake_minimum_required(VERSION 3.5)
project(scout_beacon)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set build type to Release for production
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags for production
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

# =============================================================================
# Platform Detection and Configuration
# =============================================================================

# Detect if we're on Raspberry Pi (ARM64)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(IS_RASPBERRY_PI TRUE)
    message(STATUS "Detected Raspberry Pi platform")
else()
    set(IS_RASPBERRY_PI FALSE)
    message(STATUS "Detected development platform")
endif()

# =============================================================================
# Source File Configuration
# =============================================================================

# Common source files
set(MODULE_COMMON_SRC
    common/monitor/sys_monitor.c
    common/utils/util_misc.c
    common/osal/osal.c
    common/osal/osal_fs.c
    common/osal/osal_socket.c
)

# Application source files
file(GLOB_RECURSE MODULE_APP_SRC application/*.c)

# Platform-specific HAL source files
set(SCOUT_USE_REAL_LGPIO FALSE)
if(IS_RASPBERRY_PI)
    # Use real HAL implementations on Raspberry Pi
    set(MODULE_HAL_SRC
        hal/hal_uart.c
        hal/hal_network.c
        hal/hal_usb_bulk.c
    )

    find_library(LGPIO_LIB lgpio)
    if(LGPIO_LIB)
        # Link only liblgpio — do not mix lgpio_stub.c or ClaimOutput comes from the
        # real library while ClaimInput resolves to the stub (broken hybrid).
        set(MODULE_GPIO_SRC "")
        set(SCOUT_USE_REAL_LGPIO TRUE)
        message(STATUS "-- Found lgpio: ${LGPIO_LIB} (beacon GPIO uses system lgpio, no stub)")
    else()
        set(MODULE_GPIO_SRC common/lgpio_stub.c)
        message(WARNING "-- lgpio library not found; beacon GPIO uses stub only")
    endif()
else()
    # Use stub implementations for development
    set(MODULE_HAL_SRC
        hal/hal_uart_stub.c
        hal/hal_network_stub.c
        hal/hal_usb_bulk_stub.c
    )
    
    set(MODULE_GPIO_SRC
        common/lgpio_stub.c
    )
    
    message(STATUS "-- Using lgpio stub for development")
endif()

# =============================================================================
# Include Directories
# =============================================================================

include_directories(common)
include_directories(common/osal)
include_directories(common/monitor)
include_directories(common/utils)
include_directories(application)

# PSDK library paths
if(IS_RASPBERRY_PI)
    # Use actual PSDK library on Raspberry Pi
    include_directories(/home/aviscout/MVP/Payload-SDK/psdk_lib/include)
    link_directories(/home/aviscout/MVP/Payload-SDK/psdk_lib/lib/aarch64-linux-gnu-gcc)
else()
    # Use PSDK stubs for development (macOS paths)
    include_directories(/Users/mikeneqa/Dev/Payload-SDK/psdk_lib/include)
    message(STATUS "-- Using PSDK stubs for development")
endif()

# =============================================================================
# OPUS Library Configuration
# =============================================================================

# Find OPUS library
find_library(OPUS_LIB opus)
if(OPUS_LIB)
    message(STATUS "-- Found OPUS: ${OPUS_LIB}")
else()
    message(WARNING "-- OPUS library not found")
endif()

# =============================================================================
# Executable Creation
# =============================================================================

# Create main executable
add_executable(${PROJECT_NAME}
    ${MODULE_APP_SRC}
    ${MODULE_COMMON_SRC}
    ${MODULE_HAL_SRC}
    ${MODULE_GPIO_SRC}
)

if(SCOUT_USE_REAL_LGPIO)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SCOUT_BEACON_USE_REAL_LGPIO=1)
endif()

# Create heartbeat script executable (standalone tool for testing)
# Note: Uses same include directories as main executable (defined above)
add_executable(get_drone_heartbeat
    scripts/get_drone_heartbeat.c
    ${MODULE_COMMON_SRC}
    ${MODULE_HAL_SRC}
    # Note: GPIO sources excluded - heartbeat script doesn't need GPIO
)

# Create PSDK version checker executable (standalone tool for version checking)
add_executable(get_psdk_version
    scripts/get_psdk_version.c
    ${MODULE_COMMON_SRC}
    ${MODULE_HAL_SRC}
    # Note: GPIO sources excluded - version checker doesn't need GPIO
)

# Set output directory for heartbeat script (inherits from EXECUTABLE_OUTPUT_PATH)
# No need to set separately as it's already configured globally above

# =============================================================================
# Library Linking
# =============================================================================

# Link lgpio only when beacon code uses the real library (not the stub)
if(SCOUT_USE_REAL_LGPIO AND LGPIO_LIB)
    target_link_libraries(${PROJECT_NAME} ${LGPIO_LIB})
endif()

# Link OPUS library if found (for main executable only)
if(OPUS_LIB)
    target_link_libraries(${PROJECT_NAME} ${OPUS_LIB})
endif()

# Link PSDK library on Raspberry Pi (for all executables)
if(IS_RASPBERRY_PI)
    target_link_libraries(${PROJECT_NAME} payloadsdk)
    target_link_libraries(get_drone_heartbeat payloadsdk)
    target_link_libraries(get_psdk_version payloadsdk)
endif()

# Link math library for PSDK (must be linked after PSDK)
target_link_libraries(${PROJECT_NAME} m pthread)
target_link_libraries(get_drone_heartbeat m pthread)
target_link_libraries(get_psdk_version m pthread)

# =============================================================================
# Output Configuration
# =============================================================================

# Output directory
if(NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif()

# =============================================================================
# Build Information
# =============================================================================

message(STATUS "=== Scout Beacon Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Toolchain: ${CMAKE_C_COMPILER}")
message(STATUS "Output: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "==========================================")
