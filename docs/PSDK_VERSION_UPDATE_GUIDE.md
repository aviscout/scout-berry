# PSDK SDK Version Update Guide

## Problem: Version Mismatch Between SDK and Drone Firmware

**Current Situation:**
- **PSDK SDK Version**: V3.9.2-beta.0-build.2125 (January 2024)
- **Drone Firmware**: Recently updated (newer than SDK)
- **Symptom**: SDK sends packets but receives no responses from drone

## Why This Causes Communication Failures

When the drone firmware is newer than the PSDK SDK, several issues can occur:

1. **Protocol Mismatch**: Newer firmware may use updated communication protocols
2. **API Changes**: New commands or data structures may not be recognized
3. **Version Checks**: Firmware may reject connections from older SDK versions
4. **Handshake Failures**: Initialization sequence may have changed

## Solution: Update PSDK SDK

### Step 1: Download Latest PSDK SDK

1. **Visit DJI Developer Portal**
   - Go to: https://developer.dji.com/
   - Log in with your developer account
   - Navigate to **Payload SDK** section

2. **Download Latest Version**
   - Look for the latest stable release (not beta if possible)
   - Download the Linux ARM64 (aarch64) version
   - Ensure it matches your drone's firmware version

3. **Check Compatibility**
   - Verify the SDK version supports your drone model (Mavic 3 Enterprise)
   - Check release notes for firmware compatibility requirements

### Step 2: Backup Current PSDK Library

```bash
# Create backup of current PSDK library
cd /home/aviscout/MVP
cp -r Payload-SDK Payload-SDK-backup-$(date +%Y%m%d)
```

### Step 3: Extract New PSDK SDK

```bash
# Extract new SDK (adjust filename as needed)
cd /home/aviscout/MVP
tar -xzf Payload-SDK-V*.tar.gz
# Or unzip if it's a zip file
unzip Payload-SDK-V*.zip
```

### Step 4: Verify Library Structure

Ensure the new SDK has the correct structure:

```bash
ls -la /home/aviscout/MVP/Payload-SDK/psdk_lib/lib/aarch64-linux-gnu-gcc/
# Should show: libpayloadsdk.a
```

### Step 5: Update CMakeLists.txt Paths

The current `CMakeLists.txt` has hardcoded macOS paths that need to be fixed:

**Current (WRONG):**
```cmake
include_directories(/Users/mikeneqa/Dev/Payload-SDK/psdk_lib/include)
link_directories(/Users/mikeneqa/Dev/Payload-SDK/psdk_lib/lib/aarch64-linux-gnu-gcc)
```

**Should be:**
```cmake
include_directories(/home/aviscout/MVP/Payload-SDK/psdk_lib/include)
link_directories(/home/aviscout/MVP/Payload-SDK/psdk_lib/lib/aarch64-linux-gnu-gcc)
```

### Step 6: Clean and Rebuild

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build

# Clean previous build
rm -rf CMakeFiles CMakeCache.txt *.o bin/*

# Reconfigure with new paths
cmake ..

# Rebuild
make -j$(nproc)
```

### Step 7: Verify New SDK Version

After rebuilding, check the logs to confirm the new version:

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
./bin/get_drone_heartbeat 2>&1 | grep "Payload SDK Version"
```

You should see a newer version number than `V3.9.2-beta.0-build.2125`.

### Step 8: Test Connection

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
./bin/get_drone_heartbeat
```

**Expected Results:**
- ✅ SDK initializes successfully
- ✅ Receives responses from drone (not just sending)
- ✅ Aircraft information is retrieved
- ✅ No timeout errors

## Troubleshooting

### Issue: Can't Find Latest SDK Version

**Solution:**
- Check DJI Developer Portal for release announcements
- Contact DJI support if SDK seems outdated
- Check DJI forums for compatibility information

### Issue: New SDK Breaks Existing Code

**Solution:**
- Review SDK release notes for breaking changes
- Check API migration guides
- Update deprecated function calls

### Issue: Still No Communication After Update

**Possible Causes:**
1. SDK version still doesn't match firmware
2. PSDK mode needs to be re-enabled after firmware update
3. Drone needs power cycle after firmware update

**Solutions:**
1. Verify SDK version matches firmware requirements
2. Re-enable PSDK mode in DJI Assistant 2
3. Power cycle drone (power off, wait 10s, power on)

## Version Compatibility Matrix

| Drone Firmware | Recommended PSDK SDK | Notes |
|---------------|---------------------|-------|
| Latest (2025) | V3.10.x or newer | Check DJI docs for exact version |
| Mid-2024 | V3.9.x | May work but verify compatibility |
| Early 2024 | V3.9.2-beta.0-build.2125 | Current version (outdated) |

## Additional Notes

- **Always backup** before updating SDK
- **Test thoroughly** after updating
- **Check release notes** for breaking changes
- **Keep SDK and firmware versions aligned** for best compatibility

## References

- DJI Developer Portal: https://developer.dji.com/
- PSDK Documentation: Check SDK download package
- DJI Support: For version compatibility questions
