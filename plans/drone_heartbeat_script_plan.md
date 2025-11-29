# Plan: Simple Script to Get Drone Heartbeat

## Overview
Create a simple, standalone C script that initializes DJI PSDK and subscribes to flight controller heartbeat data to verify communication with the drone.

## Objectives
1. **Verify PSDK Connection**: Confirm that the PSDK can successfully connect to the drone
2. **Display Heartbeat Data**: Continuously display drone heartbeat/status information
3. **Simple Testing Tool**: Provide a lightweight tool for testing drone connectivity without running the full beacon application

## Technical Approach

### 1. Script Structure
- **File**: `scripts/get_drone_heartbeat.c`
- **Language**: C (compatible with existing PSDK codebase)
- **Dependencies**: DJI PSDK libraries, OSAL, HAL modules

### 2. Key Components

#### A. PSDK Initialization
- Initialize system environment (OSAL, HAL, Logger)
- Fill in user information (App ID, Key, etc.)
- Initialize DJI PSDK core
- Get aircraft information
- Start SDK application

#### B. Flight Controller Subscription
- Subscribe to flight controller heartbeat topic
- Register callback for heartbeat updates
- Display heartbeat data:
  - Connection status
  - Aircraft type
  - Flight mode
  - Battery status (if available)
  - GPS status
  - Timestamp

#### C. Main Loop
- Continuously monitor heartbeat updates
- Display formatted heartbeat information
- Handle graceful shutdown (Ctrl+C)

### 3. Implementation Details

#### Required PSDK APIs
- `DjiCore_Init()` - Initialize PSDK core
- `DjiFcSubscription_Init()` - Initialize FC subscription module
- `DjiFcSubscription_SubscribeTopic()` - Subscribe to heartbeat topic
- `DjiFcSubscription_SetTopicUpdateCallback()` - Register callback

#### Heartbeat Data Structure
The script will display:
- **Connection Status**: Connected/Disconnected
- **Aircraft Type**: Mavic 3 Enterprise, etc.
- **Flight Mode**: Manual, GPS, etc.
- **Battery Level**: Percentage (if available)
- **GPS Status**: Number of satellites, fix status
- **Update Rate**: Frequency of heartbeat updates

#### Error Handling
- PSDK initialization failures
- Subscription failures
- Connection timeouts
- Graceful cleanup on exit

### 4. Build Configuration

#### CMakeLists.txt Addition
- Add new executable target: `get_drone_heartbeat`
- Link against PSDK libraries
- Include necessary source files from existing project

#### Compilation
```bash
cd scout_beacon/build
cmake ..
make get_drone_heartbeat
```

### 5. How to Build and Run (Step-by-Step)

**Yes, you need to BUILD the script first** - it's a C program that must be compiled before running.

#### On Raspberry Pi (Production)

**Step 1: Navigate to project directory**
```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon
```

**Step 2: Create/enter build directory**
```bash
mkdir -p build
cd build
```

**Step 3: Configure with CMake** (only needed first time or after changes)
```bash
cmake ..
```

**Step 4: Build the heartbeat script**
```bash
make get_drone_heartbeat
```

**Step 5: Verify executable was created**
```bash
ls -la bin/get_drone_heartbeat
```

**Step 6: Run the script** (requires sudo for UART access)
```bash
sudo ./bin/get_drone_heartbeat
```

#### Quick Build and Run (One Command)
```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build && \
cmake .. && \
make get_drone_heartbeat && \
sudo ./bin/get_drone_heartbeat
```

#### Prerequisites Before Running

1. **PSDK Mode Enabled**: Ensure drone has PSDK mode enabled in DJI Assistant 2
   - See `PSDK_SETUP_GUIDE.md` for detailed instructions

2. **Hardware Connected**: 
   - E-Port adapter connected to drone
   - Raspberry Pi connected via UART/USB
   - Drone powered on

3. **UART Permissions**:
   ```bash
   sudo chmod 666 /dev/ttyUSB0  # Adjust device name if different
   ```

4. **Log Directory** (created automatically, but verify):
   ```bash
   mkdir -p Logs/DJI
   ```

#### Running the Script

**Basic Usage:**
```bash
sudo ./bin/get_drone_heartbeat
```

**Expected Behavior:**
- Script initializes PSDK connection (takes 2-4 seconds)
- Subscribes to heartbeat topic
- Continuously displays heartbeat updates
- Press `Ctrl+C` to exit gracefully

**If Connection Fails:**
- Check PSDK mode is enabled on drone
- Verify UART device exists: `ls -la /dev/ttyUSB*`
- Check permissions: `sudo chmod 666 /dev/ttyUSB0`
- Review logs: `tail -f Logs/DJI/psdk_log_*.txt`

### 6. Usage Examples

#### Basic Usage
```bash
sudo ./bin/get_drone_heartbeat
```

#### With Log Monitoring (Separate Terminal)
```bash
# Terminal 1: Run heartbeat script
sudo ./bin/get_drone_heartbeat

# Terminal 2: Monitor PSDK logs
tail -f Logs/DJI/psdk_log_*.txt
```

#### Expected Output
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

### 6. File Structure

```
scout_beacon/
├── scripts/
│   └── get_drone_heartbeat.c    # New heartbeat script
├── application/
│   ├── main.c                    # Existing main application
│   └── ...                       # Other application files
└── CMakeLists.txt                # Updated build config
```

### 7. Implementation Steps

1. **Create Script File**
   - Create `scout_beacon/scripts/get_drone_heartbeat.c`
   - Include necessary PSDK headers
   - Implement basic structure

2. **Implement PSDK Initialization**
   - Copy initialization code from `main.c`
   - Simplify for heartbeat-only functionality
   - Add error handling

3. **Implement FC Subscription**
   - Subscribe to heartbeat topic
   - Implement callback function
   - Format and display heartbeat data

4. **Implement Main Loop**
   - Continuous monitoring loop
   - Signal handling for graceful exit
   - Cleanup on shutdown

5. **Update Build System**
   - Add executable target to CMakeLists.txt
   - Configure dependencies
   - Test compilation

6. **Testing**
   - Test with drone connected
   - Verify heartbeat data display
   - Test graceful shutdown
   - Verify error handling

### 8. Success Criteria

- ✅ Script compiles without errors
- ✅ Successfully initializes PSDK connection
- ✅ Subscribes to heartbeat topic
- ✅ Displays heartbeat data continuously
- ✅ Handles disconnection gracefully
- ✅ Clean shutdown on Ctrl+C

### 9. Future Enhancements (Optional)

- Add command-line options (verbose mode, output format)
- Log heartbeat data to file
- Display additional telemetry (altitude, speed, etc.)
- JSON output format option
- Integration with existing monitoring scripts

## Notes

- The script will reuse existing PSDK initialization code from `main.c`
- Minimal dependencies - only FC subscription module required
- Can be run independently of main beacon application
- Useful for debugging PSDK connection issues
- Provides foundation for more advanced telemetry monitoring

## Dependencies

- DJI PSDK libraries (already in project)
- OSAL modules (already in project)
- HAL modules (already in project)
- Flight Controller Subscription module (enabled in config)
