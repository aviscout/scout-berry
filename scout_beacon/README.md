# Scout Beacon Application

## Overview

The Scout Beacon application integrates avalanche beacon detection with DJI Payload SDK (PSDK) for autonomous drone rescue operations. This application reads beacon signals from GPIO pins on a Raspberry Pi and transmits the data to a Mavic 3 drone via PSDK.

## Features

- **GPIO Beacon Detection**: Reads bearing and distance from avalanche beacon receiver
- **PSDK Integration**: Transmits beacon data to DJI drone via low-speed data channel
- **Real-time Processing**: Continuous monitoring of beacon signals
- **Comprehensive Logging**: Detailed logging of beacon detection and transmission events
- **Raspberry Pi Compatible**: Optimized for Raspberry Pi 4 with lgpio library

## Hardware Requirements

### Raspberry Pi Setup
- Raspberry Pi 4 Model B
- E-Port Development Kit
- Beacon receiver connected to GPIO pins
- Power from drone battery via E-Port

### GPIO Pin Configuration

| Function | BCM Pin | Description |
|----------|---------|-------------|
| Bearing 270° | 17 | Bearing detection pin |
| Bearing 325° | 27 | Bearing detection pin |
| Bearing 0° | 22 | Bearing detection pin |
| Bearing 45° | 5 | Bearing detection pin |
| Bearing 90° | 6 | Bearing detection pin |
| Segment A | 13 | 7-segment display |
| Segment B | 19 | 7-segment display |
| Segment C | 26 | 7-segment display |
| Segment D | 21 | 7-segment display |
| Segment E | 20 | 7-segment display |
| Segment F | 16 | 7-segment display |
| Segment G | 12 | 7-segment display |
| Decimal Point | 25 | 7-segment display |
| Digit 1 Enable | 8 | 7-segment control |
| Digit 2 Enable | 7 | 7-segment control |
| RSSI | 18 | Signal strength input |

## Software Requirements

- Raspberry Pi OS (64-bit)
- lgpio library (`sudo apt install liblgpio-dev`)
- DJI Payload SDK
- CMake 3.5+
- GCC compiler

## Build Instructions

### On Development Machine (Laptop)

```bash
cd /Users/mikeneqa/Dev/Payload-SDK/samples/sample_c/platform/linux/scout_beacon
./build_and_test.sh
```

### On Raspberry Pi

```bash
cd /home/aviscout/MVP/Payload-SDK/samples/sample_c/platform/linux/scout_beacon
mkdir -p build && cd build
cmake .. && make
```

## Configuration

### DJI SDK Configuration

The application uses the following DJI SDK configuration:

- **App Name**: AviScout
- **App ID**: 156341
- **App Key**: 977ba9be54fbf8983270bc16651e6e4
- **Developer Account**: robert.preater@papuaventures.com
- **Baud Rate**: 921600 (required for Mavic 3 Enterprise)

### GPIO Permissions

Ensure the user has GPIO access:

```bash
sudo usermod -a -G gpio aviscout
```

## Usage

### Running the Application

#### Command Line Options

```bash
# Run with real GPIO hardware
sudo ./scout_beacon

# Run with mock data for testing (recommended)
sudo ./scout_beacon --mock

# Run with mock data and debug output
sudo ./scout_beacon --mock --debug

# Show help and options
./scout_beacon --help
```

#### Mock Mode Testing

The application includes comprehensive mock beacon data for testing without physical hardware:

- **8 realistic beacon patterns** cycling every 5 seconds
- **Realistic bearing, distance, and RSSI values**
- **Automatic data transmission** via PSDK
- **Debug output** for monitoring

```bash
# Test with mock data
sudo ./scout_beacon --mock --debug
```

### Expected Output

```
=== Scout Beacon Application Starting ===
AviScout - Avalanche Beacon Detection System
Copyright (c) 2024 Scout Berry

[BEACON_GPIO] Beacon GPIO initialized successfully
[BEACON_GPIO] Beacon callback registered
Scout Beacon Application started successfully
Beacon detection active. Monitoring for avalanche beacon signals...
Press Ctrl+C to exit.

[BEACON] Time: 1234567890, Bearing: 45°, Distance: 25.3m, RSSI: -67dBm, Valid: B=Y D=Y
```

### Beacon Data Format

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

## Monitoring and Testing

### Mock Data Testing (Recommended)

The application includes comprehensive mock beacon data for testing without physical hardware:

```bash
# Run with mock data
sudo ./scout_beacon --mock --debug

# Monitor logs in real-time
tail -f ./Logs/DJI/psdk_log_*.txt

# Check for successful transmissions
grep "Beacon data transmitted successfully" ./Logs/DJI/psdk_log_*.txt
```

### PSDK Log Monitoring

```bash
# Monitor PSDK logs
tail -f ./Logs/DJI/psdk_log_*.txt

# Filter for beacon-specific logs
tail -f ./Logs/DJI/psdk_log_*.txt | grep -E "(BEACON|PSDK|Data.*transmission)"

# Check data transmission format
grep -o '{"event":"BEACON_DETECTED".*}' ./Logs/DJI/psdk_log_*.txt
```

### DJI Assistant 2 Monitoring

1. **Install DJI Assistant 2** on your development machine
2. **Connect to Mavic 3** via USB or network
3. **Navigate to "Payload SDK" section**
4. **Monitor "Low-Speed Data Channel"** for incoming data
5. **Verify JSON data format** matches expected structure

### Expected Mock Mode Output

```
=== Scout Beacon Application Starting ===
AviScout - Avalanche Beacon Detection System
Copyright (c) 2024 Scout Berry

MOCK MODE: Generating simulated avalanche beacon signals
Mock data will be transmitted every 5 seconds

[BEACON_GPIO] Mock mode enabled - generating simulated beacon data
[BEACON_GPIO] Mock patterns available: 8
[BEACON_GPIO] Mock data: Close beacon - North - Bearing: 0°, Distance: 15.5m, RSSI: -65dBm
[BEACON] Time: 1703123456, Bearing: 0°, Distance: 15.5m, RSSI: -65dBm, Valid: B=Y D=Y
[PSDK] Beacon data transmitted successfully
```

## Next Steps

### Phase 1: Mock Data Testing (Current)
1. **Deploy to Raspberry Pi** and test with mock data:
   ```bash
   sudo ./scout_beacon --mock --debug
   ```
2. **Monitor PSDK logs** for successful data transmission:
   ```bash
   tail -f ./Logs/DJI/psdk_log_*.txt
   ```
3. **Verify JSON data format** in logs and DJI Assistant 2
4. **Test all 8 mock beacon patterns** for realistic behavior

### Phase 2: Hardware Integration
1. **Connect beacon receiver** to GPIO pins as configured
2. **Test with real GPIO hardware**:
   ```bash
   sudo ./scout_beacon --debug
   ```
3. **Verify physical beacon detection** and data transmission
4. **Monitor real-world beacon signals** and transmission

### Phase 3: Drone Integration
1. **Connect to Mavic 3** via E-Port Development Kit
2. **Test bidirectional communication** between Pi and drone
3. **Verify data reception** in DJI Assistant 2
4. **Test with real avalanche beacon** for end-to-end validation

### Phase 4: Production Deployment
1. **Optimize transmission frequency** based on real-world performance
2. **Implement error handling** for communication failures
3. **Add power management** for extended operation
4. **Create deployment scripts** for automated setup

### Development Workflow
```bash
# 1. Development (macOS)
git add .
git commit -m "Update scout beacon application"
git push origin cursor-dev

# 2. Deployment (Raspberry Pi)
ssh aviscout@192.168.1.84
cd /path/to/scout-berry
git pull origin cursor-dev
cd scout_beacon
mkdir -p build && cd build
cmake .. && make
sudo ./scout_beacon --mock --debug
```

## Testing

### 1. GPIO Testing

Test GPIO pins individually:

```bash
# Test bearing pin (example: pin 17)
echo 17 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio17/direction
cat /sys/class/gpio/gpio17/value
```

### 2. Beacon Detection Testing

1. Power on avalanche beacon transmitter
2. Monitor application output for detection events
3. Verify bearing and distance readings
4. Check signal strength measurements

### 3. PSDK Transmission Testing

1. Connect to DJI Assistant 2
2. Monitor data transmission logs
3. Verify JSON data format
4. Check transmission success rates

## Troubleshooting

### Common Issues

1. **GPIO Access Denied**
   ```bash
   sudo usermod -a -G gpio aviscout
   # Log out and back in
   ```

2. **lgpio Library Not Found**
   ```bash
   sudo apt update
   sudo apt install liblgpio-dev
   ```

3. **PSDK Connection Failed**
   - Check E-Port connection
   - Verify API credentials
   - Ensure drone is powered on

4. **Beacon Not Detected**
   - Check GPIO connections
   - Verify beacon transmitter is on
   - Test individual GPIO pins

### Debug Mode

Enable debug output:

```c
BeaconGpio_SetDebug(true);
```

## Architecture

### Component Overview

```
Avalanche Beacon (457 kHz)
         ↓
Beacon Receiver (Breadboard)
         ↓
GPIO Pins (Raspberry Pi)
         ↓
beacon_gpio.c (lgpio library)
         ↓
main.c (PSDK integration)
         ↓
DjiLowSpeedDataChannel_SendData()
         ↓
E-Port → Mavic 3 → OcuSync
         ↓
DJI Assistant 2 / Mobile App
```

### Key Files

- `main.c` - Main application with PSDK integration
- `beacon_gpio.c/h` - GPIO interface for beacon detection
- `dji_sdk_app_info.h` - DJI SDK configuration
- `dji_sdk_config.h` - Module configuration
- `CMakeLists.txt` - Build configuration

## Development Notes

### Code Structure

- **Modular Design**: Separate GPIO handling from PSDK integration
- **Error Handling**: Comprehensive error checking and logging
- **Real-time Processing**: Continuous beacon monitoring
- **Thread Safety**: Safe GPIO access and data transmission

### Performance Considerations

- **Update Rate**: 1Hz beacon data updates
- **GPIO Debouncing**: 50ms debounce time for bearing pins
- **Memory Usage**: Minimal memory footprint
- **CPU Usage**: Low CPU utilization

## License

Copyright (c) 2024 Scout Berry. All rights reserved.

## Support

For technical support or questions, contact the development team.
