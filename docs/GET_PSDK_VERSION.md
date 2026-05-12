# Getting DJI Payload SDK Version

This document explains how to retrieve the DJI Payload SDK version that is actually installed on the Raspberry Pi.

## Quick Method (Recommended)

### Option 1: Use the Shell Script

```bash
# On Raspberry Pi
cd /home/aviscout/MVP/scout-berry
./scripts/get_psdk_version.sh
```

This script will:
1. Check the most recent PSDK log file for the version
2. If no log file exists, run the `get_psdk_version` program
3. Fall back to other methods if needed

### Option 2: Use the C Program Directly

```bash
# On Raspberry Pi
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
./bin/get_psdk_version
```

This program initializes PSDK and extracts the version string directly.

### Option 3: Check Log Files

```bash
# On Raspberry Pi
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
grep "Payload SDK Version" Logs/DJI/psdk_log_*.txt | tail -1
```

## From Remote Machine (SSH)

If you want to check the version from your development machine:

```bash
# SSH to Raspberry Pi and run the script
sshpass -p 'scout123' ssh aviscout@192.168.1.84 \
  "cd /home/aviscout/MVP/scout-berry && ./scripts/get_psdk_version.sh"
```

Or get it in one command:

```bash
sshpass -p 'scout123' ssh aviscout@192.168.1.84 \
  "cd /home/aviscout/MVP/scout-berry/scout_beacon/build && \
   grep 'Payload SDK Version' Logs/DJI/psdk_log_*.txt 2>/dev/null | tail -1 | \
   sed 's/.*Payload SDK Version : \(V[^ ]*\).*/\1/'"
```

## Building the Version Checker Program

If the `get_psdk_version` program doesn't exist yet, build it:

```bash
# On Raspberry Pi
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
cmake ..
make get_psdk_version
```

## Expected Output

The version should be displayed in one of these formats:

```
V3.14.0-beta.0-build.2296
```

Or with build date:

```
PSDK Version (from logs): V3.14.0-beta.0-build.2296
Build Date: Nov  3
```

## Current Version

Based on the most recent log file, the current PSDK version on the Raspberry Pi is:

**V3.14.0-beta.0-build.2296** (Build date: Nov 3, 2025)

**Important Note**: The "-beta.0-" in the version string is part of DJI's standard version string format and does NOT indicate a beta release. Even stable releases show "-beta.0-" in the version string when `DJI_VERSION_BETA` is 0. This is the stable V3.14.0 release as confirmed by:
- The `DJI_VERSION_BETA` value in `dji_version.h` is 0 (stable)
- The README.md confirms "The latest release version of PSDK is 3.14.0" (stable)

## Troubleshooting

### No Log Files Found

If there are no log files, run the application once to generate them:

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
sudo ./bin/scout_beacon --mock
# Press Ctrl+C after a few seconds
```

### Program Not Found

Build the program:

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
cmake ..
make get_psdk_version
```

### Permission Issues

Make sure you have read access to log files:

```bash
sudo chmod -R 755 /home/aviscout/MVP/scout-berry/scout_beacon/build/Logs
```

## Notes

- The SDK version is printed automatically by PSDK during `DjiCore_Init()`
- The version is logged to both console and log files
- The version string format is: `V<major>.<minor>.<patch>-<tag>-build.<build_number>`
- The build number and date indicate when the SDK library was compiled
