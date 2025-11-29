# How to Verify PSDK Mode is Enabled

PSDK mode must be enabled **before** connecting your Raspberry Pi to the drone. Here are multiple ways to verify it's enabled:

## Method 1: Check in DJI Assistant 2 (Most Reliable)

This is the **primary method** to verify PSDK mode is enabled.

### Steps:

1. **Connect Drone to Computer**
   - Power on your Mavic 3 Enterprise drone
   - Connect drone to computer via USB cable
   - Wait for computer to recognize the device

2. **Open DJI Assistant 2**
   - Launch DJI Assistant 2 application
   - Select "Mavic 3 Enterprise" from aircraft list
   - Click "Connect"

3. **Navigate to Settings**
   - Look for **"Onboard SDK"** tab in the left menu
   - OR go to **"Settings"** → **"Onboard SDK"**
   - Alternative locations:
     - **"Payload SDK"** section
     - **"Extension Port"** settings
     - **"API Control"** settings

4. **Verify PSDK Mode Status**
   Look for these indicators that PSDK mode is **ENABLED**:
   
   ✅ **"API Control"** checkbox is **checked** / shows **"Enabled"** or **"Active"**
   
   ✅ **"Extension Port"** status shows **"Connected"** or **"Active"**
   
   ✅ **"Payload SDK"** mode shows **"Active"** or **"Enabled"**
   
   ✅ No error messages displayed
   
   ✅ Payload type is set to **"Custom Payload"** or **"PSDK Payload"**

### What You'll See:

**PSDK Mode ENABLED:**
```
Onboard SDK Settings:
  ☑ Enable API Control          [Enabled]
  Extension Port:                [Active]
  Payload SDK Mode:              [Active]
  Payload Type:                  Custom Payload
```

**PSDK Mode DISABLED:**
```
Onboard SDK Settings:
  ☐ Enable API Control          [Disabled]
  Extension Port:                [Inactive]
  Payload SDK Mode:              [Inactive]
```

---

## Method 2: Test with Application (Runtime Verification)

Run your Scout Beacon application and check the output/logs.

### Steps:

1. **Connect Raspberry Pi to Drone**
   - Ensure E-Port adapter is connected
   - Power on drone
   - Connect Raspberry Pi to drone's E-Port

2. **Run Application**
   ```bash
   cd /home/aviscout/MVP/scout-berry/scout_beacon/build
   sudo ./bin/scout_beacon --mock
   ```

3. **Check Output**

   **PSDK Mode ENABLED (Success):**
   ```
   === Scout Beacon Application Starting ===
   [0.001][core]-[Info]-[DjiCore_Init:106) Payload SDK Version : V3.9.2-beta.0-build.2125
   [2.500][core]-[Info]-[DjiCore_Init:126) Access adapter init success
   [2.501][core]-[Info]-[DjiAircraftInfo_GetBaseInfo] Aircraft type: X, Mount position: Y
   [2.502][core]-[Info]-[DjiAircraftInfo_GetAircraftVersion] Aircraft version is VXX.XX.XX.XX
   Mock mode enabled - generating simulated beacon data
   Scout Beacon Application started successfully
   Beacon detection active. Monitoring for avalanche beacon signals...
   ```

   **PSDK Mode DISABLED (Failure):**
   ```
   === Scout Beacon Application Starting ===
   [0.001][core]-[Info]-[DjiCore_Init:106) Payload SDK Version : V3.9.2-beta.0-build.2125
   [2.390][adapter]-[Info]-[DjiAccessAdapter_Init:215) Try identify UART0 connection failed
   [4.821][adapter]-[Info]-[DjiAccessAdapter_Init:215) Try identify UART0 connection failed
   ...
   [21.823][adapter]-[Error]-[DjiAccessAdapter_Init:220) Try identify UART0 connection timeout
   [21.823][core]-[Error]-[DjiCore_Init:126) Access adapter init error, stat:225
   ```

### Key Indicators:

**✅ PSDK Mode ENABLED:**
- ✅ "Access adapter init success" message appears
- ✅ Aircraft information is retrieved (type, version)
- ✅ No timeout errors
- ✅ Application continues running without errors

**❌ PSDK Mode DISABLED:**
- ❌ "Try identify UART0 connection failed" messages (repeated)
- ❌ "Access adapter init error" with error code 225
- ❌ "Try identify UART0 connection timeout" error
- ❌ Application fails to initialize PSDK connection

---

## Method 3: Check Application Logs

After running the application, check the PSDK logs:

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
tail -50 Logs/DJI/psdk_log_*.txt
```

### Look for:

**✅ PSDK Mode ENABLED:**
- "Access adapter init success"
- "Aircraft type" and "Aircraft version" information
- No "timeout" or "failed" errors after initialization

**❌ PSDK Mode DISABLED:**
- Multiple "Try identify UART0 connection failed" messages
- "Access adapter init error"
- "Try identify UART0 connection timeout"

---

## Method 4: Check in DJI Assistant 2 - Payload SDK Section

Once your application is running and connected:

1. In DJI Assistant 2, navigate to **"Payload SDK"** section
2. Check that:
   - Payload is **recognized** and shows as **"Connected"**
   - **Low-Speed Data Channel** is **Active**
   - Incoming data is visible in the data channel monitor
   - You can see JSON data being transmitted

---

## Quick Verification Checklist

Use this checklist to verify PSDK mode:

- [ ] **DJI Assistant 2**: "API Control" is checked/enabled
- [ ] **DJI Assistant 2**: "Extension Port" shows Active/Connected
- [ ] **Application Output**: "Access adapter init success" appears
- [ ] **Application Output**: Aircraft information is retrieved
- [ ] **Application Logs**: No timeout errors after initialization
- [ ] **DJI Assistant 2**: Payload shows as "Connected" in Payload SDK section

---

## Common Issues

### Issue: "API Control" checkbox is grayed out or not visible

**Possible causes:**
- Drone firmware doesn't support PSDK
- Wrong version of DJI Assistant 2
- Drone model doesn't support E-Port payloads

**Solution:**
- Update drone firmware
- Use correct DJI Assistant 2 version for Mavic 3 Enterprise
- Verify drone model supports PSDK

### Issue: Settings show enabled but application still fails

**Possible causes:**
- Drone wasn't restarted after enabling PSDK mode
- Settings weren't saved/applied

**Solution:**
- Power cycle drone (power off, wait 10 seconds, power on)
- Re-check settings in DJI Assistant 2
- Ensure "Apply" or "Save" was clicked

### Issue: Application connects but then disconnects

**Possible causes:**
- Physical connection issue
- E-Port adapter not properly seated
- Power supply issue

**Solution:**
- Check physical connections
- Verify E-Port adapter is properly connected
- Check payload board LED is active

---

## Important Notes

1. **PSDK mode must be enabled BEFORE connecting Raspberry Pi**
   - Enable it first in DJI Assistant 2
   - Then connect your Raspberry Pi to the drone

2. **Drone restart is required**
   - After enabling PSDK mode, always restart the drone
   - Power off completely, wait 10 seconds, power on

3. **Settings persist**
   - Once enabled, PSDK mode stays enabled until you disable it
   - You don't need to re-enable it every time you connect

4. **Verification is ongoing**
   - You can verify PSDK mode is enabled anytime by:
     - Checking DJI Assistant 2 settings
     - Running the application and checking for successful connection

---

## Summary

**The easiest way to verify PSDK mode is enabled:**

1. Connect drone to computer via USB
2. Open DJI Assistant 2
3. Navigate to "Onboard SDK" settings
4. Verify "API Control" checkbox is checked
5. Verify "Extension Port" shows "Active"

**If you see timeout errors when running the application:**
- PSDK mode is likely NOT enabled
- Go back to DJI Assistant 2 and enable it
- Restart the drone
- Try again

**If you see "Access adapter init success":**
- PSDK mode IS enabled and working correctly! ✅
