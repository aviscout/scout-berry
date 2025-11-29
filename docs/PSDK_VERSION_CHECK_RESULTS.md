# PSDK Version Check Results

## Current Status

**Date Checked**: January 2025

### Installed Version
- **Current PSDK SDK**: V3.9.2-beta.0-build.2125
- **Release Date**: January 2024
- **Status**: ⚠️ **OUTDATED** - Multiple newer versions available

### Latest Available Versions

From DJI Payload SDK GitHub repository, the following versions are available:

| Version | Status | Notes |
|---------|--------|-------|
| **V3.14.0** | ✅ Latest | Most recent stable release |
| V3.13.1 | ✅ Available | Patch release |
| V3.13.0 | ✅ Available | Feature release |
| V3.12.1 | ✅ Available | Patch release |
| V3.12.0 | ✅ Available | Feature release |
| V3.11.1 | ✅ Available | Patch release |
| V3.11.0 | ✅ Available | Feature release |
| V3.10.0 | ✅ Available | Feature release |
| **V3.9.2** | ⚠️ **CURRENT** | Your installed version (outdated) |

## Version Gap Analysis

**You are 5 major releases behind:**
- Current: V3.9.2
- Latest: V3.14.0
- Gap: 5 versions (V3.10.0 → V3.11.0 → V3.11.1 → V3.12.0 → V3.12.1 → V3.13.0 → V3.13.1 → V3.14.0)

This significant version gap explains why you're experiencing communication issues with the updated drone firmware.

## Recommended Action

### Option 1: Update to Latest (Recommended)
**Target Version**: V3.14.0

**Pros:**
- Best compatibility with latest drone firmware
- Latest bug fixes and features
- Longest support window

**Cons:**
- May require code changes if APIs changed
- Need to review release notes for breaking changes

### Option 2: Update to Stable Intermediate Version
**Target Version**: V3.12.1 or V3.13.1

**Pros:**
- More stable, well-tested
- Fewer breaking changes than jumping to latest
- Good balance of features and stability

**Cons:**
- Still may not match very latest firmware features

## Update Steps

### Step 1: Checkout Latest Version

```bash
cd /home/aviscout/MVP/Payload-SDK

# Backup current version
git stash
git tag current-backup-$(date +%Y%m%d)

# Checkout latest version
git checkout 3.14.0

# Or checkout a specific version:
# git checkout 3.13.1
# git checkout 3.12.1
# git checkout 3.10.0
```

### Step 2: Verify Library Files

```bash
# Check that library files exist for your architecture
ls -la /home/aviscout/MVP/Payload-SDK/psdk_lib/lib/aarch64-linux-gnu-gcc/
# Should show: libpayloadsdk.a
```

### Step 3: Check Release Notes

```bash
# View release notes for the version you're updating to
cd /home/aviscout/MVP/Payload-SDK
git show 3.14.0:README.md | head -50
```

### Step 4: Rebuild Application

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build

# Clean build
rm -rf CMakeFiles CMakeCache.txt *.o bin/*

# Reconfigure
cmake ..

# Rebuild
make -j$(nproc)
```

### Step 5: Verify New Version

```bash
cd /home/aviscout/MVP/scout-berry/scout_beacon/build
./bin/get_drone_heartbeat 2>&1 | grep "Payload SDK Version"
```

Expected output should show a version newer than `V3.9.2-beta.0-build.2125`.

## Important Notes

### Breaking Changes
When updating across multiple versions, review release notes for:
- API changes
- Deprecated functions
- New requirements
- Protocol changes

### Compatibility Testing
After updating:
1. Test basic connection first
2. Verify aircraft info retrieval
3. Test all your application features
4. Check for any new error messages

### Rollback Plan
If the new version causes issues:

```bash
cd /home/aviscout/MVP/Payload-SDK
git checkout current-backup-YYYYMMDD  # Use your backup tag
# Or
git checkout 3.9.2  # Return to current version
```

## Version History Summary

Based on git repository:

- **V3.14.0** - Latest release
- **V3.13.1** - Patch release
- **V3.13.0** - Feature release
- **V3.12.1** - Patch release
- **V3.12.0** - Feature release
- **V3.11.1** - Patch release
- **V3.11.0** - Feature release
- **V3.10.0** - Feature release (first version after your current)
- **V3.9.2** - Your current version

## Next Steps

1. ✅ **Backup current installation**
2. ✅ **Review release notes** for V3.10.0 through V3.14.0
3. ✅ **Update to V3.14.0** (or V3.13.1 for stability)
4. ✅ **Rebuild application**
5. ✅ **Test connection** with drone
6. ✅ **Verify communication** works properly

## References

- DJI Payload SDK GitHub: https://github.com/dji-sdk/Payload-SDK
- DJI Developer Portal: https://developer.dji.com/
- PSDK Documentation: https://developer.dji.com/doc/payload-sdk-tutorial/en/
- API Reference: https://developer.dji.com/doc/payload-sdk-api-reference/en/
