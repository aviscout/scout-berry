# Scout Beacon - Monitoring and Testing Guide

## Overview

This guide provides detailed instructions for monitoring PSDK logs, testing mock beacon data transmission, and verifying the Scout Beacon application functionality.

## Table of Contents

1. [Mock Beacon Data Testing](#mock-beacon-data-testing)
2. [PSDK Log Monitoring](#psdk-log-monitoring)
3. [Data Transmission Verification](#data-transmission-verification)
4. [DJI Assistant 2 Monitoring](#dji-assistant-2-monitoring)
5. [Troubleshooting](#troubleshooting)

---

## Mock Beacon Data Testing

### Running with Mock Data

The Scout Beacon application includes comprehensive mock beacon data for testing without physical hardware:

```bash
# Run with mock data (recommended for testing)
sudo ./scout_beacon --mock

# Run with mock data and debug output
sudo ./scout_beacon --mock --debug

# Show help
./scout_beacon --help
```

### Mock Data Patterns

The application includes 8 realistic beacon patterns that cycle every 5 seconds:

| Pattern | Bearing | Distance | RSSI | Description |
|---------|---------|----------|------|-------------|
| 1 | 0° | 15.5m | -65 dBm | Close beacon - North |
| 2 | 45° | 32.1m | -72 dBm | Medium range - Northeast |
| 3 | 90° | 8.7m | -58 dBm | Very close - East |
| 4 | 135° | 45.2m | -78 dBm | Far range - Southeast |
| 5 | 180° | 22.3m | -69 dBm | Medium range - South |
| 6 | 225° | 67.8m | -85 dBm | Far range - Southwest |
| 7 | 270° | 12.4m | -61 dBm | Close range - West |
| 8 | 315° | 38.9m | -74 dBm | Medium range - Northwest |

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

---

## PSDK Log Monitoring

### Log File Locations

PSDK logs are automatically created in the following locations:

```bash
# Main PSDK logs
./Logs/DJI/psdk_log_<timestamp>.txt

# Log index file
./Logs/index

# Application logs (stdout/stderr)
./scout_beacon.log  # If redirected
```

### Monitoring Logs in Real-Time

```bash
# Monitor PSDK logs in real-time
tail -f ./Logs/DJI/psdk_log_*.txt

# Monitor all log files
tail -f ./Logs/DJI/psdk_log_*.txt ./Logs/index

# Filter for beacon-specific logs
tail -f ./Logs/DJI/psdk_log_*.txt | grep -E "(BEACON|PSDK|Data.*transmission)"
```

### Key Log Messages to Look For

#### Successful Initialization
```
[INFO] Scout Beacon Application started successfully
[INFO] Data transmission module initialized
[INFO] Mock mode enabled - generating simulated beacon data
```

#### Beacon Detection
```
[INFO] [BEACON] Time: 1703123456, Bearing: 0°, Distance: 15.5m, RSSI: -65dBm, Valid: B=Y D=Y
[INFO] Beacon data transmitted successfully
```

#### PSDK Communication
```
[INFO] Aircraft type: 1, Mount position: 1
[INFO] Aircraft version is V01.00.00.00
[INFO] Data transmission module initialized
```

#### Error Messages
```
[ERROR] Failed to transmit beacon data: 0x08000001
[ERROR] Beacon GPIO not initialized
[ERROR] Data transmission module init error
```

---

## Data Transmission Verification

### JSON Data Format

The application transmits beacon data in JSON format via PSDK's low-speed data channel:

```json
{
  "event": "BEACON_DETECTED",
  "timestamp": 1703123456,
  "bearing": 0,
  "distance": 15.5,
  "signal_strength": -65,
  "bearing_valid": true,
  "distance_valid": true
}
```

### Data Transmission Frequency

- **Mock Mode**: Every 5 seconds (configurable)
- **Real Mode**: When beacon signal is detected
- **Update Interval**: 1 second (main loop)

### Verifying Data Transmission

#### 1. Check PSDK Logs
```bash
# Look for successful transmission messages
grep "Beacon data transmitted successfully" ./Logs/DJI/psdk_log_*.txt
```

#### 2. Monitor Data Channel
```bash
# Check for data channel activity
grep "DjiLowSpeedDataChannel_SendData" ./Logs/DJI/psdk_log_*.txt
```

#### 3. Verify JSON Format
```bash
# Extract and validate JSON data
grep -o '{"event":"BEACON_DETECTED".*}' ./Logs/DJI/psdk_log_*.txt | jq .
```

---

## DJI Assistant 2 Monitoring

### Installation and Setup

1. **Download DJI Assistant 2** from DJI's official website
2. **Install on your development machine** (Windows/macOS)
3. **Connect to the drone** via USB or network

### Monitoring Data Reception

#### 1. Launch DJI Assistant 2
- Open DJI Assistant 2 application
- Connect to your Mavic 3 Enterprise drone
- Navigate to "Payload SDK" section

#### 2. Monitor Low-Speed Data Channel
- Look for "Low-Speed Data Channel" tab
- Monitor incoming data from the Scout Beacon application
- Verify JSON data format matches expected structure

#### 3. Check Data Logs
- Navigate to "Logs" section
- Look for payload SDK communication logs
- Verify data transmission timestamps

### Expected DJI Assistant 2 Output

```
Payload SDK Status: Connected
Low-Speed Data Channel: Active
Data Reception: {"event":"BEACON_DETECTED","timestamp":1703123456,"bearing":0,"distance":15.5,"signal_strength":-65,"bearing_valid":true,"distance_valid":true}
```

---

## Testing Procedures

### 1. Mock Data Testing (Recommended)

```bash
# Step 1: Build the application
cd scout_beacon
mkdir -p build && cd build
cmake .. && make

# Step 2: Run with mock data
sudo ./scout_beacon --mock --debug

# Step 3: Monitor logs
tail -f ../Logs/DJI/psdk_log_*.txt

# Step 4: Verify data transmission
grep "Beacon data transmitted successfully" ../Logs/DJI/psdk_log_*.txt
```

### 2. Real Hardware Testing

```bash
# Step 1: Connect beacon receiver to GPIO pins
# Step 2: Run without mock mode
sudo ./scout_beacon --debug

# Step 3: Monitor for real beacon signals
tail -f ../Logs/DJI/psdk_log_*.txt | grep BEACON
```

### 3. PSDK Integration Testing

```bash
# Step 1: Connect to Mavic 3 via E-Port
# Step 2: Run with mock data
sudo ./scout_beacon --mock

# Step 3: Monitor DJI Assistant 2 for data reception
# Step 4: Verify bidirectional communication
```

---

## Troubleshooting

### Common Issues

#### 1. GPIO Permission Errors
```bash
# Error: Permission denied accessing GPIO
# Solution: Run with sudo
sudo ./scout_beacon --mock
```

#### 2. PSDK Initialization Failures
```bash
# Error: Core init error
# Check: E-Port connection and drone power
# Verify: PSDK credentials in dji_sdk_app_info.h
```

#### 3. Data Transmission Failures
```bash
# Error: Failed to transmit beacon data
# Check: PSDK module initialization
# Verify: Low-speed data channel configuration
```

#### 4. Mock Mode Not Working
```bash
# Error: Mock mode not generating data
# Check: BeaconGpio_EnableMockMode() return value
# Verify: Mock patterns array initialization
```

### Debug Commands

```bash
# Enable verbose logging
sudo ./scout_beacon --mock --debug

# Check GPIO permissions
ls -la /dev/gpiochip*

# Verify PSDK logs
ls -la ./Logs/DJI/

# Monitor system resources
top -p $(pgrep scout_beacon)
```

### Log Analysis

```bash
# Extract all beacon detections
grep "BEACON DETECTED" ./Logs/DJI/psdk_log_*.txt

# Count successful transmissions
grep -c "Beacon data transmitted successfully" ./Logs/DJI/psdk_log_*.txt

# Check for errors
grep -i error ./Logs/DJI/psdk_log_*.txt

# Monitor data format
grep -o '{"event":"BEACON_DETECTED".*}' ./Logs/DJI/psdk_log_*.txt
```

---

## Performance Monitoring

### System Resource Usage

```bash
# Monitor CPU usage
top -p $(pgrep scout_beacon)

# Monitor memory usage
ps aux | grep scout_beacon

# Monitor GPIO activity
sudo cat /sys/kernel/debug/gpio
```

### Data Transmission Metrics

```bash
# Count transmissions per minute
grep -c "Beacon data transmitted successfully" ./Logs/DJI/psdk_log_*.txt

# Monitor transmission frequency
grep "Beacon data transmitted successfully" ./Logs/DJI/psdk_log_*.txt | tail -10
```

---

## Next Steps

1. **Deploy to Raspberry Pi** and test with physical hardware
2. **Connect to Mavic 3** via E-Port and verify data reception
3. **Monitor DJI Assistant 2** for successful data transmission
4. **Test with real avalanche beacon** for end-to-end validation
5. **Optimize transmission frequency** based on real-world performance

---

## Support

For additional support or questions:
- Check the main README.md for build instructions
- Review PSDK documentation for advanced configuration
- Monitor system logs for detailed error information
- Test with mock data before deploying to hardware
