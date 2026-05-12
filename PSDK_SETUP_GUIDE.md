# PSDK Communication Setup Guide

This guide provides step-by-step instructions for setting up communication between the Scout Beacon application and the Mavic 3 Enterprise drone using DJI Payload SDK (PSDK).

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Enable PSDK Mode on Drone](#enable-psdk-mode-on-drone)
3. [Verify Hardware Connection](#verify-hardware-connection)
4. [Test Initial Communication](#test-initial-communication)
5. [Verify Communication Success](#verify-communication-success)
6. [Troubleshooting](#troubleshooting)

---

## Prerequisites

Before starting, ensure you have:

- **Mavic 3 Enterprise** drone with E-Port support
- **Raspberry Pi 4** (aarch64) with Scout Beacon application built
- **DJI Assistant 2** installed on your computer
  - Download: https://www.dji.com/downloads/softwares/assistant-dji-2-for-mavic
- **USB cable** for connecting drone to computer
- **E-Port adapter** properly connected to drone
- **Custom payload board** mounted on drone

---

## Enable PSDK Mode on Drone

**This is the critical first step** - the drone must have PSDK mode enabled before the application can communicate with it.

### Step 1: Connect Drone to Computer

1. Power on your Mavic 3 Enterprise drone
2. Connect the drone to your computer using a USB cable
3. Wait for your computer to recognize the device

### Step 2: Launch DJI Assistant 2

1. Open DJI Assistant 2 application on your computer
2. Select **"Mavic 3 Enterprise"** from the aircraft list
3. Click **"Connect"** when prompted

### Step 3: Navigate to Onboard SDK Settings

1. In DJI Assistant 2, look for the left-hand menu
2. Navigate to **"Onboard SDK"** tab (or **"Settings"** → **"Onboard SDK"**)
3. If you don't see "Onboard SDK", look for:
   - **"Payload SDK"** section
   - **"Extension Port"** settings
   - **"API Control"** settings

### Step 4: Enable API Control / PSDK Mode

1. Within the Onboard SDK settings, locate the **"Enable API Control"** option
2. **Check the box** to enable API Control
   - This activates PSDK mode and enables the E-Port for payload communication
3. If available, also verify:
   - **"Extension Port"** is enabled
   - **"Payload SDK"** mode is active
   - Payload type is set to **"Custom Payload"** or **"PSDK Payload"**

### Step 5: Configure Payload Information (Optional)

Some drone configurations may require payload information:

- **Application Name**: "AviScout"
- **Application ID**: "156341"
- **Baud Rate**: 921600 (required for Mavic 3 Enterprise)

### Step 6: Apply Settings

1. Click **"Apply"** or **"Save"** to save the configuration
2. Wait for confirmation that settings are applied
3. The drone may display a message indicating settings are saved

### Step 7: Restart the Drone

1. **Power off** the drone completely
2. Wait **10 seconds**
3. **Power on** the drone again
4. This ensures the PSDK mode settings take effect

### Step 8: Verify PSDK Mode is Active

1. Reconnect to the drone in DJI Assistant 2
2. Navigate back to Onboard SDK settings
3. Verify that:
   - **"API Control"** shows as **"Enabled"** or **"Active"**
   - **"Extension Port"** status shows **"Connected"** or **"Active"**
   - No error messages are displayed

### Important Notes

- The drone **must be powered on** during the configuration process
- Some drones require a restart after enabling PSDK mode
- Keep DJI Assistant 2 connected during initial testing to monitor status
- If PSDK mode doesn't appear in DJI Assistant 2, ensure:
  - Your drone firmware supports PSDK
  - You're using the correct version of DJI Assistant 2 for Mavic 3 Enterprise
  - The drone model supports E-Port payloads

---

## Verify Hardware Connection

### On Raspberry Pi (via SSH)

```bash
# SSH to Raspberry Pi
sshpass -p 'scout123' ssh aviscout@192.168.1.84

# Check UART device exists and permissions
ls -la /dev/ttyUSB*

# Expected output should show /dev/ttyUSB0 with read/write permissions
# If permissions are wrong, fix them:
sudo chmod 666 /dev/ttyUSB0

# Check if device is accessible
sudo dmesg | tail -20 | grep ttyUSB

# Verify device is not in use by another process
lsof /dev/ttyUSB0
```

### Physical Verification Checklist

- [ ] E-Port adapter is properly connected to drone
- [ ] Payload board LED is active (green light)
- [ ] All cables are securely connected
- [ ] Raspberry Pi is receiving power from drone (if applicable)
- [ ] No loose connections or damaged cables

---

## Test Initial Communication

### Step 1: Prepare Application Environment

```bash
# SSH to Raspberry Pi
sshpass -p 'scout123' ssh aviscout@192.168.1.84

# Navigate to build directory
cd /home/aviscout/MVP/scout-berry/scout_beacon/build

# Ensure log directory exists with proper permissions
sudo mkdir -p Logs/DJI
sudo chown -R aviscout:aviscout Logs

# Verify executable exists
ls -la bin/scout_beacon
```

### Step 2: Run Application in Mock Mode

```bash
# Run with mock mode (no physical beacon required)
sudo ./bin/scout_beacon --mock
```

### Expected Output (Success)

```
=== Scout Beacon Application Starting ===
AviScout - Avalanche Beacon Detection System
Copyright (c) 2024 Scout Berry

[0.001][core]-[Info]-[DjiCore_Init:106) Payload SDK Version : V3.9.2-beta.0-build.2125
[2.500][core]-[Info]-[DjiCore_Init:126) Access adapter init success
[2.501][core]-[Info]-[DjiAircraftInfo_GetBaseInfo] Aircraft type: X, Mount position: Y
[2.502][core]-[Info]-[DjiAircraftInfo_GetAircraftVersion] Aircraft version is VXX.XX.XX.XX
Mock mode enabled - generating simulated beacon data
MOCK MODE: Generating simulated avalanche beacon signals
Mock data will be transmitted every 5 seconds

Scout Beacon Application started successfully
Beacon detection active. Monitoring for avalanche beacon signals...
Press Ctrl+C to exit.

[BEACON] Time: 1234567890, Bearing: 45°, Distance: 25.3m, RSSI: -67dBm, Valid: B=Y D=Y
Beacon data transmitted successfully
```

### Expected Output (Failure - PSDK Mode Not Enabled)

```
=== Scout Beacon Application Starting ===
AviScout - Avalanche Beacon Detection System
Copyright (c) 2024 Scout Berry

[0.001][core]-[Info]-[DjiCore_Init:106) Payload SDK Version : V3.9.2-beta.0-build.2125
[2.390][adapter]-[Info]-[DjiAccessAdapter_Init:215) Try identify UART0 connection failed
[4.821][adapter]-[Info]-[DjiAccessAdapter_Init:215) Try identify UART0 connection failed
...
[21.823][adapter]-[Error]-[DjiAccessAdapter_Init:220) Try identify UART0 connection timeout
[21.823][core]-[Error]-[DjiCore_Init:126) Access adapter init error, stat:225
```

**If you see timeout errors**, return to [Enable PSDK Mode on Drone](#enable-psdk-mode-on-drone) and verify the configuration.

### Step 3: Monitor Application Output

Watch for these success indicators:

- ✅ **PSDK Initialization**: Completes in 2-4 seconds without timeout
- ✅ **Aircraft Information**: Successfully retrieves aircraft type and version
- ✅ **No UART Timeout Errors**: No "Try identify UART0 connection failed" messages
- ✅ **Data Transmission**: "Beacon data transmitted successfully" messages appear

---

## Verify Communication Success

### Success Indicators

1. **PSDK Initialization**: No timeout errors, successful connection
2. **Aircraft Info**: Successfully retrieves aircraft type and version
3. **UART Communication**: No "Try identify UART0 connection failed" messages
4. **Data Transmission**: Mock beacon data transmits successfully

### Check Application Logs

```bash
# View recent PSDK logs
tail -f Logs/DJI/psdk_log_*.txt

# Look for successful initialization
grep "Core init" Logs/DJI/psdk_log_*.txt

# Check for transmission success
grep "Beacon data transmitted successfully" Logs/DJI/psdk_log_*.txt

# Verify no timeout errors
grep -i "timeout\|failed" Logs/DJI/psdk_log_*.txt
```

### DJI Assistant 2 Verification

1. In DJI Assistant 2, navigate to **"Payload SDK"** section
2. Check that:
   - Payload is **recognized** and shows as **"Connected"**
   - **Low-Speed Data Channel** is **Active**
   - Incoming data is visible in the data channel monitor
3. Verify JSON data format matches expected structure:
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

---

## Troubleshooting

### Issue: UART Timeout Errors

**Symptoms**: Application shows "Try identify UART0 connection failed" or "Access adapter init error"

**Solutions**:

1. **Verify DJI Assistant 2 Configuration**
   - Re-check PSDK mode is enabled in DJI Assistant 2
   - Verify extension port is active
   - Try disabling and re-enabling PSDK mode
   - Restart the drone after configuration changes

2. **Check UART Device**
   ```bash
   # Verify device exists
   ls -la /dev/ttyUSB*
   
   # Fix permissions
   sudo chmod 666 /dev/ttyUSB0
   
   # Try alternative device if available
   # Edit hal/hal_uart.h to use /dev/ttyACM0
   ```

3. **Drone Power Cycle**
   - Power off drone completely
   - Wait 10 seconds
   - Power on drone
   - Re-run application

4. **Baud Rate Verification**
   - Confirm baud rate is 921600 in application (`dji_sdk_app_info.h`) for Mavic 3 Enterprise
   - Verify baud rate in DJI Assistant 2 matches
   - Try different baud rates if necessary (115200, 230400)

5. **Physical Connection Check**
   - Verify E-Port adapter is properly seated
   - Check for loose connections
   - Inspect cables for damage
   - Verify payload board LED is active

### Issue: "Access adapter init error" (Error Code 225)

**Cause**: PSDK mode not enabled on drone or E-Port not responding

**Solution**: 
- Follow [Enable PSDK Mode on Drone](#enable-psdk-mode-on-drone) steps carefully
- Ensure drone is restarted after enabling PSDK mode
- Verify in DJI Assistant 2 that PSDK mode is active

### Issue: "Try identify UART0 connection failed"

**Cause**: Wrong UART device, permissions issue, or device in use

**Solution**:
```bash
# Check device permissions
ls -la /dev/ttyUSB0

# Fix permissions
sudo chmod 666 /dev/ttyUSB0

# Check if device is in use
lsof /dev/ttyUSB0

# If in use, kill the process or wait for it to release
```

### Issue: No Aircraft Info Retrieved

**Cause**: Communication not fully established

**Solution**:
- Verify PSDK mode is enabled and active
- Check UART device is correct
- Ensure drone is powered on and E-Port adapter is connected
- Try power cycling the drone

### Issue: Application Crashes or Hangs

**Cause**: System resource issues or configuration problems

**Solution**:
```bash
# Check system resources
free -h
df -h

# Check for other processes using UART
ps aux | grep scout_beacon

# Verify log directory permissions
sudo mkdir -p Logs/DJI
sudo chown -R aviscout:aviscout Logs
```

### Common Error Codes

| Error Code | Meaning | Solution |
|------------|---------|----------|
| 225 | Access adapter init error | Enable PSDK mode in DJI Assistant 2 |
| Timeout | UART communication timeout | Check device, permissions, and PSDK mode |
| Permission denied | UART device access denied | Fix permissions: `sudo chmod 666 /dev/ttyUSB0` |

---

## Quick Reference Commands

### SSH to Raspberry Pi
```bash
sshpass -p 'scout123' ssh aviscout@192.168.1.84
```

### Fix UART Permissions
```bash
sudo chmod 666 /dev/ttyUSB0
```

### Create Log Directory
```bash
sudo mkdir -p Logs/DJI && sudo chown -R aviscout:aviscout Logs
```

### Run Application (Mock Mode)
```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
sudo ./bin/scout_beacon --mock
```

### Monitor Logs
```bash
tail -f Logs/DJI/psdk_log_*.txt
```

### Check UART Device
```bash
ls -la /dev/ttyUSB*
dmesg | tail -20 | grep ttyUSB
```

---

## Next Steps

Once communication is established:

1. **Test with Real Hardware**: Connect actual beacon receiver to GPIO pins
2. **Monitor Data Transmission**: Verify beacon data is transmitted correctly
3. **Test with DJI Pilot**: Check if payload appears in DJI Pilot app
4. **Performance Testing**: Monitor transmission rates and latency
5. **Field Testing**: Test in actual deployment scenario

---

## Additional Resources

- **DJI Developer Documentation**: https://developer.dji.com/doc/payload-sdk-tutorial/en/
- **PSDK API Reference**: https://developer.dji.com/doc/payload-sdk-api-reference/en/
- **DJI Assistant 2 Download**: https://www.dji.com/downloads/softwares/assistant-dji-2-for-mavic
- **Project Documentation**: See `.cursor/docs/scout_beacon_deployment_progress.md` for detailed progress

---

## Support

For issues or questions:

1. Check this troubleshooting guide
2. Review application logs in `Logs/DJI/`
3. Verify DJI Assistant 2 shows payload connected
4. Check project documentation in `.cursor/docs/`

---

**Last Updated**: Based on PSDK V3.9.2-beta.0-build.2125  
**Tested With**: Mavic 3 Enterprise, Raspberry Pi 4 (aarch64)

