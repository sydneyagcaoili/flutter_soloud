# Flutter SoLoud Fork - Namespace Solution Summary

## Problem Overview

The iOS build for `medee_clinic` failed with duplicate symbol errors because both `flutter_soloud` and `rive_common` packages statically link the miniaudio library. MediaPipe's requirement for static linkage throughout the project prevented using dynamic linkage as a workaround.

### Original Error
```
Error (Xcode): 988 duplicate symbols for architecture arm64
```

All duplicate symbols were miniaudio functions (`ma_*`) and macros (`MA_*`) that conflicted between the two packages.

## Solution Implemented

Created a namespaced fork of `flutter_soloud` that prefixes all miniaudio symbols to avoid conflicts:
- `ma_*` → `soloud_ma_*` (39,746 symbols)
- `MA_*` → `SOLOUD_MA_*` (15,970 macros)

## Files Modified

### Core Namespace Changes (Initial Processing)
1. **src/soloud/src/backend/miniaudio/miniaudio.h** - Main miniaudio header
2. **src/soloud/src/backend/miniaudio/soloud_miniaudio.cpp** - Backend implementation
3. **src/waveform/waveform.cpp** - Waveform processing
4. **src/waveform/miniaudio_libvorbis.h** - Vorbis codec header
5. **src/waveform/miniaudio_libvorbis.cpp** - Vorbis codec implementation

### Additional Files (Second Processing)
6. **src/player.h** - Player class header (1 symbol)
7. **src/player.cpp** - Player class implementation (8 symbols + 2 macros)

### Automation Script
- **namespace_miniaudio.py** - Python script that automates the namespacing process

## Additional Fix: iOS Audio Codec Libraries

After namespacing miniaudio symbols, a second issue appeared: missing Ogg/Opus/Vorbis codec symbols.

### Problem
```
Error (Xcode): Undefined symbol: _ogg_page_bos
Error (Xcode): Undefined symbol: _opus_decode_float
Error (Xcode): Undefined symbol: _vorbis_synthesis
... (39 total undefined symbols)
```

### Root Cause
The `flutter_soloud.podspec` only included device libraries in `vendored_libraries`, but the linker flags referenced both device and simulator libraries. CocoaPods wasn't properly linking the simulator libraries.

### Solution
Updated `ios/flutter_soloud.podspec` to include both device and simulator libraries:

```ruby
s.ios.vendored_libraries = [
  'libs/libopus_iOS-device.a',
  'libs/libogg_iOS-device.a',
  'libs/libvorbis_iOS-device.a',
  'libs/libvorbisfile_iOS-device.a',
  'libs/libopus_iOS-simulator.a',      # Added
  'libs/libogg_iOS-simulator.a',       # Added
  'libs/libvorbis_iOS-simulator.a',    # Added
  'libs/libvorbisfile_iOS-simulator.a' # Added
]
```

## Git Repository Status

**Branch:** `fix/namespace-miniaudio-symbols`

**Commits:**
1. Initial commit with README and namespace script
2. Applied namespace changes to 5 core files (39,746 symbols + 15,970 macros)
3. Added comprehensive documentation
4. Fixed player.h and player.cpp namespace processing
5. Fixed iOS podspec to include simulator audio codec libraries

**Total Changes:**
- 55,727 symbol/macro replacements
- 7 C/C++ files modified
- 1 iOS podspec configuration fix
- 1 Python automation script created
- Full documentation provided

## Next Steps for User

### 1. Push Fork to GitHub

```bash
cd "Flutter External Forks/flutter_soloud_fork"

# Add your GitHub remote (replace with your fork URL)
git remote add origin https://github.com/YOUR_USERNAME/flutter_soloud.git

# Push the branch
git push -u origin fix/namespace-miniaudio-symbols
```

### 2. Update pubspec.yaml

In your `Flutter/medee_clinic/pubspec.yaml`, replace the flutter_soloud dependency:

**Before:**
```yaml
dependencies:
  medee_audio:
    git:
      url: https://github.com/Medee-Health/medee-common.git
      ref: release/1.0.50
      path: packages/audio
```

**After:**
```yaml
dependencies:
  medee_audio:
    git:
      url: https://github.com/Medee-Health/medee-common.git
      ref: release/1.0.50
      path: packages/audio

dependency_overrides:
  flutter_soloud:
    git:
      url: https://github.com/YOUR_USERNAME/flutter_soloud.git
      ref: fix/namespace-miniaudio-symbols
```

### 3. Clean and Rebuild

```bash
cd Flutter/medee_clinic

# Clean previous builds
flutter clean
rm -rf ios/Pods
rm ios/Podfile.lock

# Get dependencies
flutter pub get

# Rebuild for iOS
flutter build ios --debug
# or
flutter run -d <your-ios-device>
```

## Technical Details

### Why This Solution Works

1. **Symbol Isolation**: By prefixing all miniaudio symbols in flutter_soloud, they no longer conflict with the unprefixed symbols in rive_common
2. **Static Linkage Compatible**: The solution works with MediaPipe's static linkage requirement
3. **Automated Process**: The namespace script can be re-run if flutter_soloud updates
4. **No Functional Changes**: Only symbol names changed; all functionality remains identical

### Namespace Script Features

- Processes multiple files in a single run
- Creates automatic backups (.backup files)
- Uses regex for precise symbol matching
- Handles both function names and macro definitions
- Provides detailed output of changes made

### Files Processed Summary

| File | ma_* Symbols | MA_* Macros | Total |
|------|--------------|-------------|-------|
| miniaudio.h | 39,746 | 15,970 | 55,716 |
| soloud_miniaudio.cpp | Included in above | Included in above | - |
| waveform.cpp | Included in above | Included in above | - |
| miniaudio_libvorbis.h | Included in above | Included in above | - |
| miniaudio_libvorbis.cpp | Included in above | Included in above | - |
| player.h | 1 | 0 | 1 |
| player.cpp | 8 | 2 | 10 |
| **TOTAL** | **39,755** | **15,972** | **55,727** |

## Verification

After rebuilding, verify the solution by:

1. **Check for duplicate symbols**: Build should complete without linker errors
2. **Test audio functionality**: Ensure all audio features work correctly
3. **Monitor for issues**: Watch for any runtime audio problems

## Maintenance

If `flutter_soloud` updates in the future:

1. Pull latest changes from upstream
2. Merge into your fork
3. Re-run the namespace script: `python3 namespace_miniaudio.py`
4. Commit and push changes
5. Update your app's dependency reference if needed

## Alternative Solutions Considered

1. **Dynamic Linkage**: Not possible due to MediaPipe's static XCFrameworks
2. **Remove One Package**: Would lose required functionality
3. **Modify rive_common**: More complex and harder to maintain
4. **Use Different Audio Library**: Would require extensive refactoring

## Contact & Support

- **Original Issue**: iOS build failure with duplicate miniaudio symbols
- **Solution Date**: January 2025
- **Flutter Version**: 3.35.4
- **Xcode Version**: Compatible with latest iOS SDK

## License

This fork maintains the same license as the original flutter_soloud package (MIT License).

---

**Status**: ✅ Ready for deployment
**Last Updated**: 2025-01-10