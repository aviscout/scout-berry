# PSDK Mode Verification Guide

This guide provides step-by-step instructions to verify that PSDK mode is properly enabled on your Mavic 3 Enterprise drone.

## Quick Verification Checklist

- [ ] Drone is powered ON
- [ ] DJI Assistant 2 is installed and running
- [ ] Drone is connected to computer via USB
- [ ] PSDK mode is enabled in DJI Assistant 2
- [ ] Drone has been restarted after enabling PSDK mode
- [ ] UART device is accessible on Raspberry Pi

---

## Step-by-Step Verification

### Step 1: Install DJI Assistant 2

If you haven't already, download and install DJI Assistant 2:

1. **Download**: https://www.dji.com/downloads/softwares/assistant-dji-2-for-mavic
2. **Install** on your computer (Windows/Mac/Linux)
3. **Launch** DJI Assistant 2

### Step 2: Connect Drone to Computer

1. **Power ON** your Mavic 3 Enterprise drone
   - Wait for full boot sequence (LEDs stabilize)
   - Ensure drone is in a safe, stable position

2. **Connect** drone to computer using USB cable
   - Use the USB port on the drone (not the controller)
   - Wait for computer to recognize the device

3. **Verify Connection** in DJI Assistant 2:
   - Open DJI Assistant 2
   - You should see "Mavic 3 Enterprise" in the aircraft list
   - Click "Connect" if prompted

### Step 3: Navigate to PSDK Settings

The location of PSDK settings varies by DJI Assistant 2 version. Try these locations:

#### Option A: Onboard SDK Settings
1. In DJI Assistant 2, look for **"Settings"** or **"Configuration"** tab
2. Navigate to **"Onboard SDK"** section
3. Look for **"Enable API Control"** or **"API Control"** option

#### Option B: Payload SDK Settings
1. Look for **"Payload SDK"** or **"PSDK"** section
2. Check for **"Enable Payload SDK"** or **"PSDK Mode"** option

#### Option C: Extension Port Settings
1. Navigate to **"Extension Port"** or **"E-Port"** settings
2. Look for **"Enable Extension Port"** or **"Payload Port"** option

### Step 4: Enable PSDK Mode

1. **Locate** the PSDK/API Control setting
2. **Check the box** or **Toggle ON** to enable:
   - ✅ "Enable API Control"
   - ✅ "Enable Payload SDK"
   - ✅ "Enable Extension Port"
   - ✅ "PSDK Mode"

3. **Verify** the setting shows as **"Enabled"** or **"Active"**

### Step 5: Configure Payload Information (If Required)

Some versions may require payload information:

- **Application Name**: `AviScout`
- **Application ID**: `156341`
- **Baud Rate**: `460800` (or auto-detect)
- **Payload Type**: `Custom Payload` or `PSDK Payload`

### Step 6: Apply and Save Settings

1. Click **"Apply"** or **"Save"** button
2. Wait for confirmation that settings are saved
3. The drone may display a message indicating settings are applied

### Step 7: Restart the Drone

**CRITICAL**: PSDK mode changes require a drone restart to take effect.

1. **Power OFF** the drone completely
   - Wait 10 seconds
2. **Power ON** the drone again
   - Wait for full boot sequence
3. **Reconnect** to DJI Assistant 2 if needed

### Step 8: Verify PSDK Mode is Active

1. **Reconnect** to drone in DJI Assistant 2
2. **Navigate back** to PSDK/Onboard SDK settings
3. **Verify** that:
   - ✅ "API Control" shows as **"Enabled"** or **"Active"**
   - ✅ "Extension Port" status shows **"Connected"** or **"Active"**
   - ✅ No error messages are displayed

### Step 9: Test Connection from Raspberry Pi

Once PSDK mode is verified in DJI Assistant 2:

1. **SSH** to Raspberry Pi:
   ```bash
   ssh aviscout@192.168.1.84
   ```

2. **Run connection verification script**:
   ```bash
   cd /home/aviscout/MVP/scout-berry/scout_beacon/build
   ../scripts/verify_connection.sh
   ```

3. **Run heartbeat script**:
   ```bash
   sudo ./bin/get_drone_heartbeat
   ```

4. **Expected output** (if successful):
   ```
   PSDK core initialized successfully
   Aircraft Type: X
   Mount Position: Y
   [Heartbeat] Status: CONNECTED
   ```

---

## Troubleshooting

### Issue: PSDK Settings Not Visible

**Possible Causes:**
- DJI Assistant 2 version doesn't support PSDK for your drone model
- Drone firmware doesn't support PSDK
- Wrong drone model selected

**Solutions:**
1. Update DJI Assistant 2 to latest version
2. Update drone firmware to latest version
3. Verify you're using Mavic 3 Enterprise (not standard Mavic 3)
4. Check DJI documentation for your specific model

### Issue: Settings Won't Save

**Possible Causes:**
- Drone not fully connected
- Firmware version incompatible
- Settings locked by other configuration

**Solutions:**
1. Disconnect and reconnect drone
2. Restart DJI Assistant 2
3. Update drone firmware
4. Check for conflicting settings

### Issue: PSDK Mode Enabled But No Connection

**Possible Causes:**
- Drone not restarted after enabling PSDK mode
- UART device not accessible
- Wrong baud rate
- Physical connection issue

**Solutions:**
1. **Power cycle** the drone (power off, wait 10s, power on)
2. **Check UART device**:
   ```bash
   ls -la /dev/ttyUSB*
   sudo chmod 666 /dev/ttyUSB0
   ```
3. **Verify baud rate** in `dji_sdk_app_info.h` is `460800`
4. **Check physical connections**:
   - E-Port adapter securely connected
   - UART/USB cable properly connected
   - No loose connections

### Issue: "Access adapter init error" (Error Code 225)

**Meaning**: PSDK cannot establish communication with the drone

**Solutions:**
1. Verify PSDK mode is enabled (follow steps above)
2. Power cycle the drone
3. Check UART device permissions
4. Verify physical connections
5. Try different UART device (`/dev/ttyACM0` instead of `/dev/ttyUSB0`)

---

## Verification Commands

### On Computer (with DJI Assistant 2)

1. **Check connection status** in DJI Assistant 2
2. **Verify settings** are saved and active
3. **Check for error messages** in DJI Assistant 2

### On Raspberry Pi

```bash
# 1. Check UART device
ls -la /dev/ttyUSB* /dev/ttyACM*

# 2. Check permissions
stat /dev/ttyUSB0

# 3. Fix permissions if needed
sudo chmod 666 /dev/ttyUSB0

# 4. Run verification script
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
../scripts/verify_connection.sh

# 5. Test PSDK connection
sudo ./bin/get_drone_heartbeat
```

---

## Expected Behavior

### When PSDK Mode is Correctly Enabled:

1. **DJI Assistant 2**:
   - Shows "API Control" as "Enabled"
   - Shows "Extension Port" as "Active"
   - No error messages

2. **Heartbeat Script**:
   - Initializes PSDK successfully (2-4 seconds)
   - Retrieves aircraft information
   - Shows "CONNECTED" status
   - Displays heartbeat updates continuously

3. **Logs**:
   - No timeout errors
   - Successful aircraft info retrieval
   - Continuous heartbeat updates

### When PSDK Mode is NOT Enabled:

1. **Heartbeat Script**:
   - Shows "Try identify UART0 connection failed" messages
   - Times out after ~22 seconds
   - Error: "PSDK core initialization failed (0x000000E1)"
   - Error: "Access adapter init error, stat:225"

---

## Quick Reference

### Enable PSDK Mode:
1. Connect drone → Computer (USB)
2. Open DJI Assistant 2
3. Settings → Onboard SDK → Enable API Control
4. Apply/Save
5. **Power cycle drone** (OFF → wait 10s → ON)
6. Verify in DJI Assistant 2

### Test Connection:
```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
sudo ./bin/get_drone_heartbeat
```

### Fix Permissions:
```bash
sudo chmod 666 /dev/ttyUSB0
sudo usermod -a -G dialout $USER
# Then log out and back in
```

---

## Additional Resources

- **PSDK Setup Guide**: `PSDK_SETUP_GUIDE.md`
- **Connection Verification**: Run `scripts/verify_connection.sh`
- **DJI Developer Docs**: https://developer.dji.com/doc/payload-sdk-tutorial/en/
- **DJI Assistant 2 Download**: https://www.dji.com/downloads/softwares/assistant-dji-2-for-mavic

---

## Support

If you continue to have issues after following this guide:

1. Check `PSDK_SETUP_GUIDE.md` for detailed setup instructions
2. Review logs: `scout_beacon/build/Logs/DJI/psdk_log_*.txt`
3. Run verification script: `scripts/verify_connection.sh`
4. Verify hardware connections
5. Check DJI Assistant 2 for error messages

---

**Last Updated**: 2024  
**Tested With**: Mavic 3 Enterprise, DJI Assistant 2, PSDK V3.9.2
