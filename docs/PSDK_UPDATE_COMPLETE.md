# PSDK Update Complete

## Update Summary

**Date**: January 28, 2025  
**Previous Version**: V3.9.2-beta.0-build.2125 (January 2024)  
**New Version**: **V3.14.0-beta.0-build.2296** (Latest Stable Release)

## Update Status: ✅ COMPLETE

### What Was Updated

1. **PSDK SDK Library**
   - Updated from V3.9.2 to V3.14.0
   - Build number: 2125 → 2296
   - Location: `/home/aviscout/MVP/Payload-SDK`

2. **Application Rebuilt**
   - Scout Beacon application rebuilt with new SDK
   - Executables updated:
     - `scout_beacon/build/bin/scout_beacon`
     - `scout_beacon/build/bin/get_drone_heartbeat`

3. **Backup Created**
   - Backup location: `/home/aviscout/MVP/Payload-SDK-backups/Payload-SDK-backup-20251128-232602`
   - Git tag created: `backup-3.9.2-*`

## Version Comparison

| Component | Before | After | Change |
|-----------|--------|-------|--------|
| SDK Version | V3.9.2 | **V3.14.0** | +5 major versions |
| Build Number | 2125 | **2296** | +171 builds |
| Release Date | Jan 2024 | Latest | ~1 year newer |

## What Changed Between Versions

### Major Versions Skipped
- V3.10.0 - Feature release
- V3.11.0 - Feature release  
- V3.11.1 - Patch release
- V3.12.0 - Feature release
- V3.12.1 - Patch release
- V3.13.0 - Feature release
- V3.13.1 - Patch release
- **V3.14.0** - Latest feature release

### Expected Improvements

1. **Better Firmware Compatibility**
   - Updated protocol support for latest drone firmware
   - Improved handshake and initialization
   - Better error handling and diagnostics

2. **Bug Fixes**
   - Fixed communication issues
   - Improved stability
   - Enhanced error reporting

3. **New Features**
   - Support for newer drone models
   - Enhanced API capabilities
   - Improved performance

## Next Steps

### 1. Test Connection

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
./bin/get_drone_heartbeat
```

**Expected Results:**
- ✅ SDK initializes successfully
- ✅ Receives responses from drone (not just sending)
- ✅ Aircraft information retrieved
- ✅ No timeout errors

### 2. Verify SDK Version in Logs

After running the application, check the logs:

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
tail -20 Logs/DJI/psdk_log_*.txt | grep "Payload SDK Version"
```

Should show: `Payload SDK Version : V3.14.0-beta.0-build.2296`

**Note**: The "-beta.0-" in the version string is part of DJI's version string format and does NOT indicate a beta release. The `DJI_VERSION_BETA` value in the header file is 0, confirming this is the stable V3.14.0 release.

### 3. Test Full Application

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
./bin/scout_beacon
```

### 4. Monitor for Issues

Watch for:
- ✅ Successful connection establishment
- ✅ Two-way communication (sending AND receiving)
- ✅ No timeout errors
- ✅ Aircraft information retrieval

## Rollback Instructions

If you need to rollback to the previous version:

```bash
cd /home/aviscout/MVP/Payload-SDK
git checkout 3.9.2

# Then rebuild
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
rm -rf CMakeFiles CMakeCache.txt *.o bin/*
cmake ..
make -j$(nproc)
```

Or use the backup:

```bash
cd /home/aviscout/MVP/Payload-SDK
git checkout backup-3.9.2-*
```

## Troubleshooting

### Issue: Still No Communication

**Possible Causes:**
1. Drone firmware still newer than V3.14.0
2. PSDK mode needs to be re-enabled after firmware update
3. Drone needs power cycle

**Solutions:**
1. Check drone firmware version in DJI Assistant 2
2. Re-enable PSDK mode in DJI Assistant 2
3. Power cycle drone (power off, wait 10s, power on)

### Issue: Build Errors

If you see build errors:
1. Clean build directory: `rm -rf CMakeFiles CMakeCache.txt *.o bin/*`
2. Reconfigure: `cmake ..`
3. Rebuild: `make -j$(nproc)`

### Issue: API Changes

If the application fails due to API changes:
1. Review release notes for V3.10.0 through V3.14.0
2. Check for deprecated functions
3. Update code to use new APIs

## Files Modified

- `/home/aviscout/MVP/Payload-SDK` - Updated to V3.14.0
- `/home/aviscout/MVP/scout-berry/scout_beacon/build/bin/scout_beacon` - Rebuilt
- `/home/aviscout/MVP/scout-berry/scout_beacon/build/bin/get_drone_heartbeat` - Rebuilt

## References

- PSDK Version Check: `docs/PSDK_VERSION_CHECK_RESULTS.md`
- Update Guide: `docs/PSDK_VERSION_UPDATE_GUIDE.md`
- DJI Developer Portal: https://developer.dji.com/
