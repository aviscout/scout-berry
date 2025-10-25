# Scout Berry - Architecture Overview

## Project Summary

Scout Berry is a comprehensive avalanche beacon detection system that integrates with DJI Payload SDK (PSDK) for autonomous drone rescue operations. The system reads beacon signals from GPIO pins on a Raspberry Pi and transmits the data to a Mavic 3 drone via PSDK.

## System Architecture

### Core Components

1. **Python GPIO Interface (`main.py`)**
   - Initial beacon signal detection and GPIO testing
   - Technology: Python with gpiozero library
   - Status: ✅ Working - detects bearing and distance from beacon receiver

2. **Scout Beacon Application (`scout_beacon/`)**
   - Production C/C++ application with PSDK integration
   - Technology: C with lgpio library and DJI PSDK
   - Status: ✅ Complete - ready for deployment

### Hardware Requirements

- **Raspberry Pi 4 Model B** with 64-bit OS
- **E-Port Development Kit** for drone connection
- **Beacon receiver** connected to GPIO pins
- **Mavic 3 Enterprise** drone

### GPIO Pin Configuration

| Function | BCM Pin | Description |
|----------|---------|-------------|
| Bearing 270° | 17 | Bearing detection |
| Bearing 325° | 27 | Bearing detection |
| Bearing 0° | 22 | Bearing detection |
| Bearing 45° | 5 | Bearing detection |
| Bearing 90° | 6 | Bearing detection |
| 7-Segment A-G | 13,19,26,21,20,16,12 | Distance display |
| Decimal Point | 25 | Distance decimal |
| Digit Enable | 8,7 | 7-segment control |
| RSSI | 18 | Signal strength |

## Software Architecture

### Application Structure

```
scout_beacon/
├── application/          # Source code
│   ├── main.c           # Main PSDK application
│   ├── beacon_gpio.c    # GPIO implementation
│   ├── beacon_gpio.h    # GPIO interface
│   ├── dji_sdk_app_info.h    # DJI configuration
│   └── dji_sdk_config.h      # Module configuration
├── hal/                 # Hardware abstraction
│   ├── hal_network.c    # Network interface
│   ├── hal_network.h    # Network interface header
│   ├── hal_uart.c       # UART interface
│   ├── hal_uart.h       # UART interface header
│   ├── hal_usb_bulk.c   # USB bulk interface
│   └── hal_usb_bulk.h    # USB bulk interface header
├── CMakeLists.txt       # Build configuration
├── README.md           # Detailed documentation
└── build_and_test.sh   # Build script
```

### Key Features

- **Real-time GPIO Reading**: Uses lgpio library for efficient GPIO operations
- **Bearing Detection**: Reads from 5 directional pins (0°, 45°, 90°, 270°, 325°)
- **Distance Measurement**: Decodes 7-segment display for distance reading
- **Signal Strength Monitoring**: RSSI measurement for signal quality
- **Mock Mode**: Testing without physical hardware
- **PSDK Integration**: DJI Payload SDK for drone communication
- **JSON Data Transmission**: Structured data format for drone communication

### Data Flow

1. **GPIO Reading**: Continuous monitoring of beacon receiver pins
2. **Data Processing**: Bearing, distance, and signal strength extraction
3. **Data Validation**: Ensuring data integrity and validity
4. **PSDK Transmission**: Sending JSON-formatted data to drone
5. **Logging**: Comprehensive logging for monitoring and debugging

### Data Format

The application transmits JSON-formatted beacon data:

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

## Build System

### CMake Configuration

The build system is configured with CMake and supports:

- **Cross-platform builds**: x86_64 and aarch64 architectures
- **DJI PSDK integration**: Automatic library linking
- **GPIO library support**: lgpio library for Raspberry Pi
- **Optional dependencies**: OPUS, LIBUSB for enhanced functionality
- **Comprehensive error handling**: Build validation and error reporting

### Dependencies

- **lgpio**: GPIO interface library for Raspberry Pi
- **DJI PSDK**: Payload SDK for drone communication
- **OPUS**: Audio codec library (optional)
- **LIBUSB**: USB device communication (optional)

## API Compliance

### DJI PSDK Standards

The application follows DJI PSDK API standards:

- **Module Configuration**: Proper module initialization and configuration
- **Data Transmission**: Low-speed data channel for beacon data
- **Error Handling**: Comprehensive error handling and logging
- **Resource Management**: Proper cleanup and resource management
- **Thread Safety**: Thread-safe operations for multi-threading

### Function Documentation

All functions are extensively documented with:

- **Purpose and functionality**
- **Parameter descriptions**
- **Return value explanations**
- **Error handling details**
- **Usage examples**
- **Performance considerations**

## Testing and Validation

### Mock Mode

The application includes a comprehensive mock mode for testing:

- **8 different beacon patterns** for realistic testing
- **Configurable update intervals** for different test scenarios
- **Random variations** for realistic data simulation
- **Pattern cycling** for comprehensive testing

### Debug Features

- **Debug output** for GPIO operations
- **Comprehensive logging** for troubleshooting
- **Error reporting** for system issues
- **Performance monitoring** for optimization

## Deployment

### Development Environment

- **macOS**: Development and testing
- **Cross-compilation**: ARM64 builds for Raspberry Pi
- **Version control**: Git-based development workflow

### Production Environment

- **Raspberry Pi 4**: Production deployment
- **E-Port connection**: Drone interface
- **GPIO permissions**: Proper system configuration
- **Service management**: Systemd service integration

## Security and Safety

### Data Validation

- **Input validation** for all GPIO readings
- **Data range checking** for bearing and distance values
- **Signal strength validation** for RSSI measurements
- **Timestamp verification** for data integrity

### Error Handling

- **Graceful degradation** when components fail
- **Resource cleanup** on application exit
- **Signal handling** for proper shutdown
- **Logging** for troubleshooting and monitoring

## Performance Optimization

### Efficiency Features

- **Non-blocking GPIO reads** for real-time operation
- **Efficient data structures** for minimal memory usage
- **Optimized JSON formatting** for fast transmission
- **Minimal CPU usage** during sleep intervals

### Resource Management

- **Memory management** for dynamic allocations
- **File handle management** for proper cleanup
- **GPIO resource management** for pin operations
- **Thread management** for concurrent operations

## Future Enhancements

### Planned Features

- **Enhanced error recovery** for system resilience
- **Advanced signal processing** for improved detection
- **Multi-beacon support** for multiple targets
- **Real-time analytics** for performance monitoring

### Scalability

- **Modular architecture** for easy extension
- **Plugin system** for additional functionality
- **Configuration management** for different deployments
- **API expansion** for third-party integration

## Conclusion

Scout Berry represents a comprehensive solution for avalanche beacon detection using DJI PSDK integration. The system provides real-time beacon detection, reliable data transmission, and comprehensive testing capabilities. The architecture is designed for scalability, maintainability, and production deployment in rescue operations.

The extensive documentation, comprehensive testing, and adherence to DJI PSDK standards ensure reliable operation in critical rescue scenarios. The system is ready for deployment and provides a solid foundation for future enhancements and improvements.
