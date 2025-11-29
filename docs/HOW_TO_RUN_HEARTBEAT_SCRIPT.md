# How to Build and Run the Drone Heartbeat Script

## Quick Answer

**Yes, you MUST build the script first** - it's a C program that needs to be compiled before running.

## Complete Workflow

### 1. Build the Script (One-Time Setup)

```bash
# Navigate to project directory
cd /home/aviscout/MVP/scout-berry/scout_beacon

# Create build directory (if it doesn't exist)
mkdir -p build
cd build

# Configure CMake (first time or after changes)
cmake ..

# Build the heartbeat script
make get_drone_heartbeat

# Verify it was created
ls -la bin/get_drone_heartbeat
```

### 2. Run the Script

```bash
# From the build directory
sudo ./bin/get_drone_heartbeat
```

**Note**: Requires `sudo` for UART device access.

### 3. One-Line Build and Run

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build && \
cmake .. && \
make get_drone_heartbeat && \
sudo ./bin/get_drone_heartbeat
```

## Prerequisites

Before running, ensure:

1. ✅ **PSDK Mode Enabled** on drone (via DJI Assistant 2)
   - See `PSDK_SETUP_GUIDE.md` for instructions

2. ✅ **Hardware Connected**
   - E-Port adapter connected to drone
   - Raspberry Pi connected via UART/USB
   - Drone powered on

3. ✅ **UART Permissions**
   ```bash
   sudo chmod 666 /dev/ttyUSB0  # Adjust if your device is different
   ```

4. ✅ **Log Directory** (usually auto-created)
   ```bash
   mkdir -p Logs/DJI
   ```

## Expected Output

```
=== Drone Heartbeat Monitor ===
Initializing PSDK...
PSDK initialized successfully
Aircraft Type: Mavic 3 Enterprise
Mount Position: Payload Port

Subscribing to heartbeat...
Heartbeat subscription active

[Heartbeat] Status: Connected | Mode: GPS | Battery: 85% | GPS: 12 satellites | Update Rate: 10 Hz
[Heartbeat] Status: Connected | Mode: GPS | Battery: 85% | GPS: 12 satellites | Update Rate: 10 Hz
...
```

Press `Ctrl+C` to exit gracefully.

## Troubleshooting

### Build Errors

**Problem**: `make get_drone_heartbeat` fails
- **Solution**: Ensure you're in the `build` directory and have run `cmake ..` first

**Problem**: CMake can't find PSDK libraries
- **Solution**: Check PSDK library paths in `CMakeLists.txt`

### Runtime Errors

**Problem**: "Access adapter init error" or "UART timeout"
- **Solution**: 
  1. Verify PSDK mode is enabled on drone
  2. Check UART device: `ls -la /dev/ttyUSB*`
  3. Fix permissions: `sudo chmod 666 /dev/ttyUSB0`
  4. Power cycle the drone

**Problem**: "Permission denied" when accessing UART
- **Solution**: Run with `sudo` or add user to dialout group:
  ```bash
  sudo usermod -a -G dialout $USER
  # Log out and back in
  ```

**Problem**: No heartbeat data displayed
- **Solution**: 
  1. Check logs: `tail -f Logs/DJI/psdk_log_*.txt`
  2. Verify drone is powered on and PSDK mode is active
  3. Check hardware connections

## File Locations

- **Source Code**: `scout_beacon/scripts/get_drone_heartbeat.c`
- **Executable**: `scout_beacon/build/bin/get_drone_heartbeat`
- **Logs**: `scout_beacon/build/Logs/DJI/psdk_log_*.txt`

## Rebuilding After Changes

If you modify the source code:

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
make get_drone_heartbeat
sudo ./bin/get_drone_heartbeat
```

No need to run `cmake ..` again unless you change `CMakeLists.txt`.

## Purpose

This script is designed for:
- ✅ **Initial Communication Testing**: Verify PSDK connection works
- ✅ **Quick Diagnostics**: Check if drone is responding
- ✅ **Standalone Testing**: Test without running full beacon application
- ✅ **Debugging**: Isolate connection issues

## Next Steps

Once heartbeat is working:
1. Verify connection is stable
2. Check heartbeat update rate
3. Proceed with full beacon application testing
