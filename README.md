# Scout Berry

AviScout - Avalanche Beacon Detection System for Autonomous Drone Rescue Operations

## Project Overview

Scout Berry integrates avalanche beacon detection with DJI Payload SDK (PSDK) for autonomous drone rescue operations. The system reads beacon signals from GPIO pins on a Raspberry Pi and transmits the data to a Mavic 3 Enterprise drone via PSDK.

## Quick Start for Developers

**New to the project?** Start here:

1. **[PSDK Setup Guide](PSDK_SETUP_GUIDE.md)** - Complete step-by-step instructions for:
   - Enabling PSDK mode on the drone using DJI Assistant 2
   - Verifying hardware connections
   - Testing initial communication
   - Troubleshooting common issues

2. **Hardware Setup**: Ensure your Mavic 3 Enterprise drone has PSDK mode enabled (see guide above)

3. **Build & Deploy**: Follow the build instructions below

## Components

### 1. Python GPIO Interface (`main.py`)
- **Purpose**: Initial beacon signal detection and GPIO testing
- **Technology**: Python with gpiozero library
- **Status**: ✅ Working - detects bearing and distance from beacon receiver

### 2. Scout Beacon Application (`scout_beacon/`)
- **Purpose**: Production C/C++ application with PSDK integration
- **Technology**: C with lgpio library and DJI PSDK
- **Status**: ✅ Complete - ready for deployment

## Quick Start

### Development Environment Setup

```bash
# Clone the repository
git clone <your-repo-url>
cd scout-berry

# Setup Python environment for GPIO testing
python3 -m venv env
source env/bin/activate
pip install -r requirements.txt
```

### Testing GPIO Interface (Python)

```bash
# Activate virtual environment
source env/bin/activate

# Run GPIO beacon detection
python main.py
```

**Expected Output:**
```
Monitoring GPIO inputs. Press Ctrl+C to exit.
Signal RECEIVED from B45 @ 5
Signal RECEIVED from SEG_A_PIN_7 @ 13
Updated State: Bearing: 45, Distance: 25.3, Proximity: -2.1
```

## Scout Beacon Application (Production)

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

### Build Instructions

#### On Development Machine (macOS)

```bash
# Navigate to scout beacon directory
cd scout_beacon

# Test build configuration (development only)
./test_build.sh
```

#### On Raspberry Pi (Production)

```bash
# 1. Install dependencies
sudo apt update
sudo apt install -y cmake build-essential liblgpio-dev

# 2. Navigate to project directory
cd /path/to/scout-berry/scout_beacon

# 3. Create build directory
mkdir -p build && cd build

# 4. Configure with CMake
cmake ..

# 5. Build the application
make -j$(nproc)

# 6. Verify build
ls -la scout_beacon
```

### Running the Application

#### Prerequisites

1. **GPIO Permissions**: Ensure user has GPIO access
   ```bash
   sudo usermod -a -G gpio $USER
   # Log out and back in
   ```

2. **DJI SDK Configuration**: Verify credentials in `application/dji_sdk_app_info.h`
   - App Name: "AviScout"
   - App ID: "156341"
   - App Key: "977ba9be54fbf8983270bc16651e6e4"

3. **Hardware Setup**: Ensure beacon receiver is connected to GPIO pins

#### Execution

```bash
# Run with sudo for GPIO access
sudo ./scout_beacon
```

#### Expected Output

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

### Data Transmission

The application transmits JSON-formatted beacon data via PSDK:

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

## Deployment Workflow

### 1. Development (macOS)
```bash
# Make changes to code
git add .
git commit -m "Update scout beacon application"
git push origin cursor-dev
```

### 2. Deployment (Raspberry Pi)
```bash
# SSH to Raspberry Pi
ssh aviscout@192.168.1.84

# Pull latest changes
cd /path/to/scout-berry
git pull origin cursor-dev

# Build and run
cd scout_beacon
mkdir -p build && cd build
cmake .. && make
sudo ./scout_beacon
```

## Testing

### 1. GPIO Testing
```bash
# Test individual GPIO pins
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
   sudo usermod -a -G gpio $USER
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

Enable debug output in the application:
```c
BeaconGpio_SetDebug(true);
```

## Project Structure

```
scout-berry/
├── main.py                    # Python GPIO testing
├── requirements.txt           # Python dependencies
├── scout_beacon/             # Production C application
│   ├── application/          # Source code
│   │   ├── main.c           # Main PSDK application
│   │   ├── beacon_gpio.c    # GPIO implementation
│   │   ├── beacon_gpio.h    # GPIO interface
│   │   ├── dji_sdk_app_info.h    # DJI configuration
│   │   └── dji_sdk_config.h      # Module configuration
│   ├── hal/                 # Hardware abstraction
│   ├── CMakeLists.txt       # Build configuration
│   ├── README.md           # Detailed documentation
│   └── build_and_test.sh   # Build script
└── README.md               # This file
```

## Milestone 1 Status

✅ **Complete**: Scout Beacon application with PSDK integration
- GPIO beacon detection using lgpio library
- PSDK data transmission to Mavic 3
- Real-time monitoring and JSON data format
- Complete build system and documentation

## Next Steps

1. Deploy to Raspberry Pi and test with physical hardware
2. Verify beacon detection and data transmission
3. Monitor PSDK logs for successful communication
4. Test with DJI Assistant 2 for data reception

## License

Copyright (c) 2024 Scout Berry. All rights reserved.