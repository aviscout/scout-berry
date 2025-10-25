# Scout Berry - API Documentation

## Overview

This document provides comprehensive API documentation for the Scout Berry avalanche beacon detection system. The API follows DJI PSDK standards and provides extensive functionality for beacon detection, data processing, and drone communication.

## Core API Functions

### Beacon GPIO Interface

#### `BeaconGpio_Init()`
```c
int BeaconGpio_Init(void);
```
**Description**: Initialize beacon GPIO interface using lgpio library.

**Parameters**: None

**Returns**: 
- `0`: Success
- `-1`: Initialization failure

**Features**:
- Configures all required pins for bearing detection
- Sets up 7-segment display pins for distance measurement
- Configures RSSI pin for signal strength monitoring
- Enables edge detection for signal changes

**Error Handling**:
- GPIO chip open failures
- Pin configuration errors
- Resource allocation failures
- Automatic cleanup on failure

#### `BeaconGpio_ReadBeaconData()`
```c
int BeaconGpio_ReadBeaconData(BeaconData_t* beacon_data);
```
**Description**: Read current beacon data from GPIO interface.

**Parameters**:
- `beacon_data`: Pointer to BeaconData_t structure to fill with current data

**Returns**:
- `0`: Success
- `-1`: Invalid parameters or GPIO not initialized

**Data Structure**:
```c
typedef struct {
    uint8_t bearing;           // Bearing in degrees (0, 45, 90, 270, 325)
    float distance;            // Distance in meters from 7-segment display
    int8_t signal_strength;    // RSSI value (-dBm)
    uint32_t timestamp;       // Unix timestamp
    bool signal_detected;      // Beacon detection flag
    bool bearing_valid;        // Bearing measurement validity
    bool distance_valid;       // Distance measurement validity
} BeaconData_t;
```

**Operation Modes**:
- Real Mode: Reads actual GPIO pins for bearing, distance, and RSSI
- Mock Mode: Generates realistic test data with configurable patterns

#### `BeaconGpio_RegisterCallback()`
```c
int BeaconGpio_RegisterCallback(BeaconCallback_t callback);
```
**Description**: Register callback for beacon detection events.

**Parameters**:
- `callback`: Function to call when beacon is detected

**Returns**:
- `0`: Success
- `-1`: GPIO not initialized

**Callback Function Type**:
```c
typedef void (*BeaconCallback_t)(BeaconData_t* beacon_data);
```

**Callback Behavior**:
- Validates input parameters
- Logs beacon detection event with bearing and distance
- Immediately sends beacon data via PSDK
- Provides real-time response to beacon detection

### Mock Mode Functions

#### `BeaconGpio_EnableMockMode()`
```c
int BeaconGpio_EnableMockMode(void);
```
**Description**: Enable mock mode for testing without physical hardware.

**Parameters**: None

**Returns**:
- `0`: Success
- `-1`: GPIO not initialized

**Features**:
- 8 different beacon patterns for realistic testing
- Configurable update intervals
- Random variations for realism
- Automatic pattern cycling

#### `BeaconGpio_DisableMockMode()`
```c
int BeaconGpio_DisableMockMode(void);
```
**Description**: Disable mock mode and return to real GPIO reading.

**Parameters**: None

**Returns**:
- `0`: Success

**Features**:
- Returns to real hardware operation
- Clears mock data cache
- Restores normal GPIO reading

### Utility Functions

#### `BeaconGpio_IsBeaconDetected()`
```c
bool BeaconGpio_IsBeaconDetected(void);
```
**Description**: Check if beacon signal is currently detected.

**Parameters**: None

**Returns**:
- `true`: Beacon detected
- `false`: No beacon detected

#### `BeaconGpio_GetBearing()`
```c
int BeaconGpio_GetBearing(void);
```
**Description**: Get current bearing from GPIO pins.

**Parameters**: None

**Returns**:
- Bearing in degrees (0, 45, 90, 270, 325) or -1 if no valid bearing

#### `BeaconGpio_GetDistance()`
```c
float BeaconGpio_GetDistance(void);
```
**Description**: Get current distance from 7-segment display.

**Parameters**: None

**Returns**:
- Distance in meters, or -1.0 if invalid

#### `BeaconGpio_GetSignalStrength()`
```c
int8_t BeaconGpio_GetSignalStrength(void);
```
**Description**: Get current signal strength (RSSI).

**Parameters**: None

**Returns**:
- Signal strength in dBm, or -999 if invalid

#### `BeaconGpio_SetDebug()`
```c
void BeaconGpio_SetDebug(bool enable);
```
**Description**: Enable or disable debug output.

**Parameters**:
- `enable`: true to enable debug output, false to disable

**Returns**: None

## Main Application API

### `main()`
```c
int main(int argc, char **argv);
```
**Description**: Main entry point for Scout Beacon Application.

**Parameters**:
- `argc`: Number of command line arguments
- `argv`: Array of command line argument strings

**Returns**:
- `0`: Success
- `1`: Command line argument error
- `DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR`: System initialization error

**Command Line Options**:
- `--mock`: Enable mock mode for testing without physical hardware
- `--debug`: Enable debug output for GPIO operations
- `--help`: Show usage information

**Initialization Sequence**:
1. Parse command line arguments
2. Setup signal handlers for graceful shutdown
3. Prepare system environment (OSAL, HAL, Logger)
4. Fill in user information (App ID, Key, etc.)
5. Initialize DJI PSDK core
6. Get aircraft information
7. Set application alias and version
8. Initialize beacon GPIO interface
9. Register beacon detection callback
10. Initialize PSDK modules
11. Start SDK application
12. Enter main application loop

### Private Functions

#### `ScoutBeacon_InitializeModules()`
```c
static void ScoutBeacon_InitializeModules(void);
```
**Description**: Initialize PSDK modules for beacon data transmission.

**Parameters**: None

**Returns**: None

**Enabled Modules**:
- `CONFIG_MODULE_SAMPLE_DATA_TRANSMISSION_ON`: Low-speed data channel for beacon data
- `CONFIG_MODULE_SAMPLE_FC_SUBSCRIPTION_ON`: Flight controller data subscription
- `CONFIG_MODULE_SAMPLE_POWER_MANAGEMENT_ON`: Power management and monitoring

#### `ScoutBeacon_MainLoop()`
```c
static void ScoutBeacon_MainLoop(void);
```
**Description**: Main application loop for beacon detection and data transmission.

**Parameters**: None

**Returns**: None

**Loop Operation**:
1. Read current beacon data from GPIO interface
2. Check if beacon signal is detected
3. If beacon detected: Log event, send data via PSDK, log data
4. If beacon lost: Log signal loss, update detection state
5. Sleep for update interval

#### `ScoutBeacon_SendBeaconData()`
```c
static T_DjiReturnCode ScoutBeacon_SendBeaconData(const BeaconData_t* beacon_data);
```
**Description**: Send beacon data via DJI PSDK low-speed data channel.

**Parameters**:
- `beacon_data`: Pointer to beacon data structure to transmit

**Returns**:
- `DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS`: Transmission successful
- `DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER`: Invalid input data
- Other DJI error codes: Transmission failure

**JSON Data Format**:
```json
{
  "event": "BEACON_DETECTED",
  "timestamp": 1234567890,
  "bearing": 45,
  "distance": 25.3,
  "signal_strength": -67,
  "bearing_valid": true,
  "distance_valid": true
}
```

#### `ScoutBeacon_BeaconCallback()`
```c
static void ScoutBeacon_BeaconCallback(BeaconData_t* beacon_data);
```
**Description**: Callback function for beacon detection events.

**Parameters**:
- `beacon_data`: Pointer to beacon data structure

**Returns**: None

**Callback Behavior**:
- Validates input parameters
- Logs beacon detection event with bearing and distance
- Immediately sends beacon data via PSDK
- Provides real-time response to beacon detection

## Data Structures

### `BeaconData_t`
```c
typedef struct {
    uint8_t bearing;           // Bearing in degrees (0, 45, 90, 270, 325)
    float distance;            // Distance in meters from 7-segment display
    int8_t signal_strength;    // RSSI value (-dBm)
    uint32_t timestamp;       // Unix timestamp
    bool signal_detected;      // Beacon detection flag
    bool bearing_valid;        // Bearing measurement validity
    bool distance_valid;       // Distance measurement validity
} BeaconData_t;
```

**Data Fields**:
- `bearing`: Bearing in degrees (0, 45, 90, 270, 325) or -1 if invalid
- `distance`: Distance in meters from 7-segment display or -1.0 if invalid
- `signal_strength`: RSSI value in dBm or -999 if invalid
- `timestamp`: Unix timestamp of last update
- `signal_detected`: True if any beacon signal is detected
- `bearing_valid`: True if bearing measurement is valid
- `distance_valid`: True if distance measurement is valid

**Data Validation**:
- Bearing values are validated against known directions
- Distance values are checked for reasonable ranges (0.1m to 99.9m)
- Signal strength is bounded to typical RSSI ranges (-30 to -100 dBm)
- Timestamps are updated on each read operation

### `BeaconCallback_t`
```c
typedef void (*BeaconCallback_t)(BeaconData_t* beacon_data);
```

**Description**: GPIO callback function type for beacon detection events.

**Parameters**:
- `beacon_data`: Pointer to beacon data structure containing all beacon information

**Callback Behavior**:
- Validates input parameters
- Logs beacon detection event with bearing and distance
- Immediately sends beacon data via PSDK
- Provides real-time response to beacon detection

## Error Handling

### Return Codes

#### GPIO Interface Errors
- `0`: Success
- `-1`: General error (initialization failure, invalid parameters)

#### DJI PSDK Errors
- `DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS`: Operation successful
- `DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER`: Invalid input parameters
- `DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR`: System-level error
- `DJI_ERROR_SYSTEM_MODULE_CODE_MEMORY_ALLOC_FAILED`: Memory allocation failure

### Error Handling Strategy

1. **Input Validation**: All functions validate input parameters
2. **Resource Management**: Proper cleanup on errors
3. **Logging**: Comprehensive error logging for troubleshooting
4. **Graceful Degradation**: System continues operation when possible
5. **Recovery**: Automatic recovery from transient errors

## Performance Considerations

### Optimization Features

- **Non-blocking GPIO reads** for real-time operation
- **Efficient data structures** for minimal memory usage
- **Optimized JSON formatting** for fast transmission
- **Minimal CPU usage** during sleep intervals

### Resource Management

- **Memory management** for dynamic allocations
- **File handle management** for proper cleanup
- **GPIO resource management** for pin operations
- **Thread management** for concurrent operations

## Usage Examples

### Basic Initialization
```c
#include "beacon_gpio.h"

int main() {
    // Initialize GPIO interface
    if (BeaconGpio_Init() != 0) {
        printf("Failed to initialize GPIO\n");
        return -1;
    }
    
    // Register callback
    BeaconGpio_RegisterCallback(my_beacon_callback);
    
    // Enable debug output
    BeaconGpio_SetDebug(true);
    
    // Main loop
    while (1) {
        BeaconData_t data;
        if (BeaconGpio_ReadBeaconData(&data) == 0) {
            if (data.signal_detected) {
                printf("Beacon detected: %d°, %.1fm\n", 
                       data.bearing, data.distance);
            }
        }
        usleep(1000000); // 1 second
    }
    
    return 0;
}
```

### Mock Mode Testing
```c
// Enable mock mode for testing
BeaconGpio_EnableMockMode();

// Read mock data
BeaconData_t data;
BeaconGpio_ReadBeaconData(&data);
printf("Mock data: %d°, %.1fm, %ddBm\n", 
       data.bearing, data.distance, data.signal_strength);
```

### Callback Implementation
```c
void my_beacon_callback(BeaconData_t* beacon_data) {
    if (!beacon_data) return;
    
    printf("Beacon callback: %d°, %.1fm, %ddBm\n",
           beacon_data->bearing, 
           beacon_data->distance, 
           beacon_data->signal_strength);
    
    // Send data via PSDK
    send_beacon_data_via_psdk(beacon_data);
}
```

## Best Practices

### Initialization
1. Always check return values from initialization functions
2. Enable debug output during development
3. Register callbacks before starting main loop
4. Use mock mode for testing without hardware

### Data Handling
1. Validate beacon data before processing
2. Check validity flags before using measurements
3. Handle invalid data gracefully
4. Log important events for debugging

### Error Handling
1. Implement comprehensive error checking
2. Provide meaningful error messages
3. Clean up resources on errors
4. Use appropriate return codes

### Performance
1. Use non-blocking operations where possible
2. Minimize processing in callbacks
3. Optimize data structures for your use case
4. Monitor system resources

## Conclusion

The Scout Berry API provides a comprehensive interface for avalanche beacon detection with DJI PSDK integration. The API follows industry standards and provides extensive functionality for real-time beacon detection, data processing, and drone communication. The extensive documentation, comprehensive error handling, and adherence to DJI PSDK standards ensure reliable operation in critical rescue scenarios.
