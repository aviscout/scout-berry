# Build Persistence Guide - What Needs Rebuilding?

## TL;DR: You DON'T Need to Rebuild Every Time!

**The PSDK application build persists indefinitely** - there's no time expiry. Once built, the executable (`scout_beacon`) will work until:
1. You modify the source code
2. You manually delete the build directory
3. You update dependencies/libraries

---

## What Persists (No Rebuild Needed)

### ✅ Built Executable
- **Location**: `/home/aviscout/MVP/scout-berry/scout_beacon/build/bin/scout_beacon`
- **Status**: Once built, it stays built
- **Last Built**: Check with `ls -lh bin/scout_beacon`
- **No expiry**: The executable doesn't expire or become invalid over time

### ✅ PSDK Mode on Drone
- **Status**: Once enabled in DJI Assistant 2, it stays enabled
- **Persists**: Until you manually disable it
- **No expiry**: Settings are stored in drone firmware

### ✅ Build Configuration
- **CMake cache**: Persists in `build/CMakeCache.txt`
- **Object files**: Persist in `build/CMakeFiles/`
- **No expiry**: Build artifacts remain until cleaned

---

## When You DO Need to Rebuild

### 1. **Source Code Changes**
If you modify any `.c` or `.h` files:
```bash
cd scout_beacon/build
make  # Incremental rebuild - only rebuilds changed files
```

### 2. **CMake Configuration Changes**
If you modify `CMakeLists.txt`:
```bash
cd scout_beacon/build
cmake ..  # Regenerate build files
make      # Rebuild
```

### 3. **Clean Build (Optional)**
If you want a completely fresh build:
```bash
cd scout_beacon/build
rm -rf *  # WARNING: Deletes everything in build directory
cmake ..
make -j$(nproc)
```

**Note**: The `rm -rf *` command in the deployment doc is **optional** - it's for a clean rebuild, not required!

### 4. **Library/Dependency Updates**
If you update PSDK libraries or system libraries:
```bash
cd scout_beacon/build
make clean  # Clean previous build
cmake ..
make
```

---

## Common Misconceptions

### ❌ "I need to rebuild every time I use it"
**False!** The executable runs directly - no rebuild needed:
```bash
sudo ./bin/scout_beacon --mock
```

### ❌ "Build expires after X days"
**False!** There's no time-based expiry mechanism. The executable persists indefinitely.

### ❌ "I need to rebuild when connecting to drone"
**False!** The executable is independent of drone connection. You just run it.

### ❌ "PSDK mode needs to be re-enabled"
**False!** Once enabled in DJI Assistant 2, PSDK mode persists until you disable it.

---

## What Actually Happens When You "Use" the Application

### Running the Application (No Rebuild)
```bash
cd scout_beacon/build
sudo ./bin/scout_beacon --mock
```
- Uses the **existing executable**
- No build process involved
- Runs immediately

### If Executable Doesn't Exist
```bash
# Only then do you need to build:
cd scout_beacon/build
cmake ..
make
```

---

## Build Verification

### Check if Executable Exists
```bash
cd scout_beacon/build
ls -lh bin/scout_beacon
```

### Check Build Date
```bash
stat bin/scout_beacon | grep Modify
```

### Verify Build is Still Valid
```bash
file bin/scout_beacon
# Should show: ELF 64-bit LSB pie executable, ARM aarch64
```

---

## Incremental vs Clean Builds

### Incremental Build (Recommended)
Only rebuilds changed files - **fast**:
```bash
cd scout_beacon/build
make
```

### Clean Build (Rarely Needed)
Rebuilds everything from scratch - **slow**:
```bash
cd scout_beacon/build
make clean
make
```

### Full Clean Build (Almost Never Needed)
Deletes everything and starts fresh:
```bash
cd scout_beacon/build
rm -rf *
cmake ..
make -j$(nproc)
```

---

## Why You Might Think You Need to Rebuild

### 1. **Confusing "Build" with "Run"**
- **Build**: Compile source code → executable (one-time, or when code changes)
- **Run**: Execute the application (every time you use it)

### 2. **Seeing Build Scripts**
Scripts like `build_and_test.sh` are for **initial setup** or **after code changes**, not for every use.

### 3. **Deployment Documentation**
Docs showing `rm -rf * && cmake .. && make` are for **clean deployment**, not routine use.

### 4. **PSDK Mode Confusion**
You might be confusing:
- **Enabling PSDK mode** (one-time setup in DJI Assistant 2)
- **Running the application** (every time you use it)
- **Rebuilding** (only when code changes)

---

## Best Practices

### ✅ Do This:
1. **Build once** after code changes
2. **Run directly** every time: `sudo ./bin/scout_beacon --mock`
3. **Keep the build directory** - don't delete it
4. **Use incremental builds** (`make`) when code changes

### ❌ Don't Do This:
1. **Don't rebuild** every time you run the app
2. **Don't delete** the build directory unnecessarily
3. **Don't run** `rm -rf *` unless you need a clean build
4. **Don't confuse** building with running

---

## Quick Reference

| Action | Rebuild Needed? | Command |
|--------|----------------|---------|
| Run application | ❌ No | `sudo ./bin/scout_beacon --mock` |
| Code changed | ✅ Yes | `make` |
| CMakeLists.txt changed | ✅ Yes | `cmake .. && make` |
| First time setup | ✅ Yes | `cmake .. && make` |
| Libraries updated | ✅ Yes | `make clean && cmake .. && make` |
| Want clean build | ✅ Yes | `rm -rf * && cmake .. && make` |

---

## Summary

**You DON'T need to rebuild every time!**

- ✅ **Executable persists**: Once built, it stays built
- ✅ **No time expiry**: Builds don't expire
- ✅ **Just run it**: `sudo ./bin/scout_beacon --mock`
- ✅ **Rebuild only when**: Code changes, config changes, or first setup

The build system is designed to be **efficient** - it only rebuilds what's necessary when things change. For routine use, just run the executable directly!
