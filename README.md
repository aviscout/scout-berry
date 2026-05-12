# Scout Berry

AviScout - Avalanche Beacon Detection System for Autonomous Drone Rescue Operations

## Project Overview

Scout Berry integrates avalanche beacon detection with DJI Payload SDK (PSDK) for autonomous drone rescue operations. The system reads beacon signals from GPIO pins on a Raspberry Pi and transmits the data to a Mavic 3 Enterprise drone via PSDK.

## Project Rules

> **Stable PSDK only — no beta builds.**
> The `Payload-SDK/` tree on `main` and on every flight build **must** be a DJI GA/stable release. Beta and RC drops are not permitted, even for "quick fixes". Specifically:
>
> - Do **not** check in or deploy any PSDK whose DJI release notes label it as `beta`, `rc`, `preview`, or `nightly`.
> - Treat the runtime banner with caution: PSDK's logger always prints `V<maj>.<min>.<mod>-beta.<N>-build.<B>`. `beta.0` is DJI's marker for a GA release (`DJI_VERSION_BETA = 0` in `psdk_lib/include/dji_version.h`); anything `beta.1` or higher is a real beta and is **not** allowed.
> - When upgrading PSDK, snapshot the current tree into `Payload-SDK-backups/Payload-SDK-backup-<version>-preupgrade-<UTC-stamp>/` first so we can roll back without re-downloading from DJI.
> - Pin the in-use version in `scout_beacon/CMakeLists.txt` and call it out in the PR description; CI / heartbeat smoke test must pass against that exact version before merge.
>
> Current pinned PSDK: **V3.14.0 (stable, build 2296)**.

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

---

# PSDK Hardware Connection Modes (Raspberry Pi 4)

This section documents how to use the **DJI PSDK hardware-connection macros** inside this project, how to produce **one binary per mode**, how to **launch them and bring up the drone**, and where to **find the error logs** for debugging.

> Hardware target for this milestone: **Raspberry Pi 4 only.** No CM4 / Waveshare carrier work is in scope.

The macros and port selections below are taken directly from the upstream DJI sample at:
<https://github.com/dji-sdk/Payload-SDK/tree/master/samples/sample_c/platform/linux/raspberry_pi>

---

## 1. The PSDK Macros

PSDK selects the physical link to the drone with one preprocessor macro: **`CONFIG_HARDWARE_CONNECTION`**. The upstream sample defines five possible values:

```c
#define DJI_USE_ONLY_UART                  (0)
#define DJI_USE_UART_AND_USB_BULK_DEVICE   (1)
#define DJI_USE_UART_AND_NETWORK_DEVICE    (2)
#define DJI_USE_ONLY_USB_BULK_DEVICE       (3)
#define DJI_USE_ONLY_NETWORK_DEVICE        (4)
```

| Value | Macro | Physical link on Pi 4 | When to use |
|-------|-------|-----------------------|-------------|
| 0 | `DJI_USE_ONLY_UART` | E-Port serial (USB-UART adapter on a Pi USB-A port, or Pi GPIO UART) | Lowest-friction handshake test. Required for Step 1 (UART binary). |
| 1 | `DJI_USE_UART_AND_USB_BULK_DEVICE` | UART + raw USB bulk endpoint | Needed if you later want camera / media streaming over USB bulk. Not used in Step 1. |
| 2 | `DJI_USE_UART_AND_NETWORK_DEVICE` | UART + USB-network gadget (USB-C on Pi 4) | Required for Step 1 (network-over-USB binary). Enables the high-speed channel. |
| 3 | `DJI_USE_ONLY_USB_BULK_DEVICE` | USB bulk only, no UART | Advanced. Not exposed in scout-berry's local `dji_sdk_config.h`. |
| 4 | `DJI_USE_ONLY_NETWORK_DEVICE` | USB-network only, no UART | Advanced. Not exposed in scout-berry's local `dji_sdk_config.h`. |

**The two macros you need for Step 1 are `DJI_USE_ONLY_UART` (value 0) and `DJI_USE_UART_AND_NETWORK_DEVICE` (value 2).** Both are already supported by `scout_beacon`.

The conditional registration of the matching HAL handlers lives in [`scout_beacon/application/main.c`](scout_beacon/application/main.c) around lines 767–789.

---

## 2. Where to Switch the Macro

Edit exactly one line in [`scout_beacon/application/dji_sdk_config.h`](scout_beacon/application/dji_sdk_config.h):

```c
#define CONFIG_HARDWARE_CONNECTION         DJI_USE_ONLY_UART
```

Replace `DJI_USE_ONLY_UART` with whichever macro you want to build for. **The change requires a full rebuild** — `cmake .. && make` will pick it up automatically.

---

## 3. Pi 4 Port / Device Selection

These are the physical device names PSDK will try to open. Confirm them **before** building each binary.

### 3.1 UART mode (`DJI_USE_ONLY_UART`)

File: [`scout_beacon/hal/hal_uart.h`](scout_beacon/hal/hal_uart.h)

```c
#define LINUX_UART_DEV1    "/dev/ttyUSB0"   // FT232 USB-UART adapter on E-Port
#define LINUX_UART_DEV2    "/dev/ttyACM0"   // CDC ACM fallback
```

The upstream Pi sample defaults to `/dev/ttyAMA1` and `/dev/ttyAMA2` (Pi 4 GPIO UARTs). scout-berry uses `/dev/ttyUSB0` because we connect through the E-Port USB-UART adapter on a USB-A port. Verify the device exists before running:

```bash
ls -la /dev/ttyUSB* /dev/ttyACM*
sudo chmod 666 /dev/ttyUSB0
```

### 3.2 Network-over-USB mode (`DJI_USE_UART_AND_NETWORK_DEVICE`)

File: [`scout_beacon/hal/hal_network.h`](scout_beacon/hal/hal_network.h)

```c
#define LINUX_NETWORK_DEV   "l4tbr0"   // <-- Jetson default; CHANGE to "pi4br0" on Pi 4
```

**Required one-line code edit before building the network binary on Pi 4:**

```bash
sed -i 's|"l4tbr0"|"pi4br0"|' scout_beacon/hal/hal_network.h
```

After the drone boots and the USB-network gadget enumerates, verify the bridge interface is present:

```bash
ip link show pi4br0
lsusb | grep -i 0955:7020   # NVIDIA-style USB-net adapter VID:PID used by the drone
```

If `pi4br0` is missing, the drone has not enumerated as a USB-network device on the Pi's USB-C port — check cable, power, and that the Pi 4 USB-C is in host mode (default).

---

## 4. Build Two Separate Binaries (Step 1)

The same source tree builds both binaries. The recipe below toggles the macro, builds, and renames the output so you end up with **`scout_beacon_uart`** and **`scout_beacon_net`** side by side.

Run from `scout-berry/scout_beacon/`:

```bash
# --- Binary 1: UART only ---
sed -i 's|^#define CONFIG_HARDWARE_CONNECTION.*|#define CONFIG_HARDWARE_CONNECTION         DJI_USE_ONLY_UART|' \
    application/dji_sdk_config.h

mkdir -p build && cd build
cmake .. && make -j$(nproc)
cp bin/scout_beacon bin/scout_beacon_uart
cd ..

# --- Binary 2: UART + network-over-USB ---
sed -i 's|^#define CONFIG_HARDWARE_CONNECTION.*|#define CONFIG_HARDWARE_CONNECTION         DJI_USE_UART_AND_NETWORK_DEVICE|' \
    application/dji_sdk_config.h

# Network mode also needs the Pi 4 bridge name (one-time edit, see 3.2)
sed -i 's|"l4tbr0"|"pi4br0"|' hal/hal_network.h

cd build
cmake .. && make -j$(nproc)
cp bin/scout_beacon bin/scout_beacon_net
cd ..

# --- Confirm both binaries exist ---
file build/bin/scout_beacon_uart build/bin/scout_beacon_net
```

You should now have two distinct executables in `scout_beacon/build/bin/`.

---

## 5. Run Each Binary, Then Connect the Drone (Step 1 cont.)

For each run: **start the binary first, then power on the drone** so PSDK sees the link come up live.

### 5.1 Prep (do once per session)

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon
sudo mkdir -p Logs/DJI
sudo chown -R "$USER":"$USER" Logs
```

### 5.2 Run the UART binary

```bash
sudo ./build/bin/scout_beacon_uart 2>&1 | tee Logs/run_uart_$(date +%s).log
# now power on the Mavic 3 Enterprise
```

Look for these lines on stdout (success):

```
[0.001][core]-[Info]-[DjiCore_Init] Payload SDK Version : V3.x.x ...
[2.5xx][core]-[Info]-[DjiCore_Init] Access adapter init success
[2.5xx][core]-[Info]-[DjiAircraftInfo_GetBaseInfo] Aircraft type: ...
```

Failure pattern (UART not seeing the drone):

```
[adapter]-[Info]-[DjiAccessAdapter_Init] Try identify UART0 connection failed
[core]-[Error]-[DjiCore_Init] Access adapter init error, stat:225
```

### 5.3 Run the network-over-USB binary

```bash
# Optional: keep the previous run's logs separate
mv Logs/DJI Logs/DJI_uart && mkdir -p Logs/DJI

sudo ./build/bin/scout_beacon_net 2>&1 | tee Logs/run_net_$(date +%s).log
# now power on the Mavic 3 Enterprise
```

Network mode runs `ifconfig`/`route` under the hood (see [`scout_beacon/hal/hal_network.c`](scout_beacon/hal/hal_network.c)) — `sudo` is mandatory. Look for:

```
[hal_network] ifconfig pi4br0 up
[hal_network] ifconfig pi4br0 192.168.x.x netmask ...
[core]-[Info]-[DjiCore_Init] Access adapter init success
```

If you see `Can't open the network` or `cmd failed: ifconfig pi4br0 up`, the bridge interface does not yet exist — re-check section 3.2.

---

## 6. Error Logging (Step 2)

Logging is wired up in two places and both run automatically every time a binary starts.

### 6.1 Terminal output (printf console)

Registered in [`main.c`](scout_beacon/application/main.c) lines 755–759. Everything PSDK emits also lands on stdout/stderr. The `tee` snippets above already capture it to `Logs/run_<mode>_<timestamp>.log`.

### 6.2 PSDK file log

Registered in [`main.c`](scout_beacon/application/main.c) lines 761–765. The file is created by `DjiUser_LocalWriteFsInit` ([`main.c`](scout_beacon/application/main.c) lines 852–873) and written to:

```
scout_beacon/Logs/DJI/psdk_log_<unix_timestamp>.txt
```

Tail it live in a second terminal:

```bash
tail -f scout_beacon/Logs/DJI/psdk_log_*.txt
```

### 6.3 Targeted grep recipes

Run these against the most recent `psdk_log_*.txt` to triage common failures:

```bash
# Did the handshake complete?
grep -E "Try identify|Access adapter init|Core init" Logs/DJI/psdk_log_*.txt

# Network-bring-up errors (network binary only)
grep -E "ifconfig|route add|cmd failed" Logs/DJI/psdk_log_*.txt

# Permission / device-access errors
grep -i "permission\|chmod\|/dev/tty" Logs/DJI/psdk_log_*.txt

# Any error or warning at all
grep -E "\[Error\]|\[Warn\]" Logs/DJI/psdk_log_*.txt
```

### 6.4 Keep per-mode logs separate

Because both binaries write to the same `Logs/DJI/` folder, rename it between runs to avoid mixing sessions:

```bash
mv Logs/DJI Logs/DJI_uart   # after the UART run
mv Logs/DJI Logs/DJI_net    # after the network run
```

---

## 7. Validation Checklist (Step 3 — Pi 4 only)

The handshake is considered confirmed for a given binary when **all four** of the following are true in the latest `Logs/DJI/psdk_log_*.txt`:

- [ ] A line `Payload SDK Version : V...` is printed.
- [ ] A line `Access adapter init success` is printed (no `init error, stat:225`).
- [ ] `DjiAircraftInfo_GetBaseInfo` returns a non-empty aircraft type and version.
- [ ] At least one `Beacon data transmitted successfully` line appears (or the equivalent low-speed channel ack).

When both `scout_beacon_uart` and `scout_beacon_net` pass this checklist, Step 3 is complete for the Pi 4 hardware target.

---

## 8. Quick Reference

| Task | Command |
|------|---------|
| Switch macro | `sed -i 's\|^#define CONFIG_HARDWARE_CONNECTION.*\|#define CONFIG_HARDWARE_CONNECTION         DJI_USE_ONLY_UART\|' scout_beacon/application/dji_sdk_config.h` |
| Build | `cd scout_beacon/build && cmake .. && make -j$(nproc)` |
| Rename binary | `cp bin/scout_beacon bin/scout_beacon_uart` (or `_net`) |
| Run UART | `sudo ./build/bin/scout_beacon_uart 2>&1 \| tee Logs/run_uart_$(date +%s).log` |
| Run network | `sudo ./build/bin/scout_beacon_net 2>&1 \| tee Logs/run_net_$(date +%s).log` |
| Tail PSDK log | `tail -f scout_beacon/Logs/DJI/psdk_log_*.txt` |
| Grep errors | `grep -E "\[Error\]\|\[Warn\]" scout_beacon/Logs/DJI/psdk_log_*.txt` |
