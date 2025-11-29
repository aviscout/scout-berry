# Connection Verification Summary

## Quick Start

### Run Connection Verification Script

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
../scripts/verify_connection.sh
```

This script will check:
- ✅ UART device availability and permissions
- ✅ USB device detection
- ✅ PSDK configuration
- ✅ Build artifacts
- ✅ Log directories
- ✅ System permissions
- ✅ UART connection test
- ✅ Optional PSDK connection test

### Verify PSDK Mode Settings

See detailed guide: `docs/VERIFY_PSDK_MODE.md`

Quick checklist:
1. Power ON drone
2. Connect to DJI Assistant 2
3. Enable "API Control" / "PSDK Mode"
4. Apply settings
5. **Power cycle drone** (OFF → wait 10s → ON)
6. Verify settings are active

---

## What Was Created

### 1. Connection Verification Script
**File**: `scout_beacon/scripts/verify_connection.sh`

A comprehensive bash script that checks all aspects of the PSDK connection setup:

**Features**:
- Checks UART device availability (`/dev/ttyUSB0`, `/dev/ttyACM0`)
- Verifies device permissions
- Detects USB-to-UART bridges (CP210x)
- Validates PSDK configuration
- Checks build artifacts
- Tests system permissions
- Optional PSDK connection test

**Usage**:
```bash
# Basic usage
cd scout_beacon/build
../scripts/verify_connection.sh

# Verbose output
../scripts/verify_connection.sh --verbose
```

**Output**: Color-coded pass/fail/warning indicators with detailed diagnostics

### 2. PSDK Mode Verification Guide
**File**: `docs/VERIFY_PSDK_MODE.md`

Step-by-step guide to verify PSDK mode is enabled on the drone:

**Contents**:
- Installation instructions for DJI Assistant 2
- Step-by-step PSDK mode enablement
- Troubleshooting common issues
- Verification commands
- Expected behavior when working correctly

---

## Typical Workflow

### Initial Setup Verification

1. **Run verification script**:
   ```bash
   cd /home/aviscout/MVP/scout-berry/scout_beacon/build
   ../scripts/verify_connection.sh
   ```

2. **Fix any issues** reported by the script:
   - Fix UART permissions if needed
   - Add user to dialout group if needed
   - Rebuild if executables missing

3. **Verify PSDK mode** (see `VERIFY_PSDK_MODE.md`):
   - Connect drone to computer
   - Enable PSDK mode in DJI Assistant 2
   - Power cycle drone

4. **Test connection**:
   ```bash
   sudo ./bin/get_drone_heartbeat
   ```

### Ongoing Verification

When connection issues occur:

1. **Quick check**: Run `verify_connection.sh`
2. **Check PSDK mode**: Verify in DJI Assistant 2
3. **Check logs**: `tail -f Logs/DJI/psdk_log_*.txt`
4. **Test connection**: Run heartbeat script

---

## Common Issues and Solutions

### Issue: UART Device Not Found

**Solution**:
```bash
# Check if device exists
ls -la /dev/ttyUSB* /dev/ttyACM*

# If missing, check USB connection
lsusb

# Check dmesg for device detection
dmesg | tail -20 | grep -i "tty\|uart"
```

### Issue: Permission Denied

**Solution**:
```bash
# Fix permissions
sudo chmod 666 /dev/ttyUSB0

# Add user to dialout group
sudo usermod -a -G dialout $USER
# Then log out and back in
```

### Issue: PSDK Connection Timeout

**Possible Causes**:
1. PSDK mode not enabled → Verify in DJI Assistant 2
2. Drone not powered on → Power on drone
3. Drone not restarted after enabling PSDK → Power cycle drone
4. Wrong UART device → Check which device is actually connected

**Solution**:
1. Follow `VERIFY_PSDK_MODE.md` guide
2. Power cycle drone after enabling PSDK mode
3. Run verification script to check hardware
4. Check logs for specific error messages

### Issue: "Access adapter init error" (Error 225)

**Meaning**: PSDK cannot establish communication

**Solution**:
1. Verify PSDK mode is enabled and active
2. Power cycle drone
3. Check UART device permissions
4. Verify physical connections
5. Check baud rate matches (460800)

---

## Script Output Examples

### Successful Verification

```
✓ All critical checks passed!

Passed:  15
Failed:  0
Warnings: 0

Next steps:
1. Ensure drone is powered ON
2. Verify PSDK mode is enabled in DJI Assistant 2
3. Run: sudo ./bin/get_drone_heartbeat
```

### Issues Found

```
✗ Some checks failed - please fix issues above

Passed:  12
Failed:  3
Warnings: 2

Common fixes:
- Fix UART permissions: sudo chmod 666 /dev/ttyUSB0
- Add user to dialout group: sudo usermod -a -G dialout $USER
- Rebuild if needed: cd build && cmake .. && make get_drone_heartbeat
```

---

## Files Reference

| File | Purpose |
|------|---------|
| `scripts/verify_connection.sh` | Connection verification script |
| `docs/VERIFY_PSDK_MODE.md` | PSDK mode verification guide |
| `scripts/get_drone_heartbeat.c` | Heartbeat monitoring script |
| `PSDK_SETUP_GUIDE.md` | Complete PSDK setup guide |
| `docs/HOW_TO_RUN_HEARTBEAT_SCRIPT.md` | Heartbeat script usage guide |

---

## Next Steps

1. ✅ **Run verification script** to check current status
2. ✅ **Follow PSDK mode guide** to enable PSDK on drone
3. ✅ **Test connection** with heartbeat script
4. ✅ **Monitor logs** for connection status

---

**Last Updated**: 2024  
**Script Version**: 1.0.0
