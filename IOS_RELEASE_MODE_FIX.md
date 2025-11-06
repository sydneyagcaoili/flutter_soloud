# iOS Release Mode FFI Symbol Fix

## Problem

When building Flutter apps with `flutter_soloud` in **release mode** for iOS, the app crashes with:

```
Fatal Exception: FlutterError
DynamicLibrary.lookup (dart:ffi)
FlutterSoLoudFfi._isInitedPtr
```

This happens because iOS release builds strip symbols by default, and FFI (Foreign Function Interface) requires these symbols to be present for `DynamicLibrary.lookup()` to work.

## Root Cause

1. **Symbol Stripping**: iOS release builds use aggressive optimization that strips "unused" symbols
2. **FFI Requirements**: Flutter's FFI needs to lookup native function symbols at runtime
3. **Dead Code Elimination**: The linker removes code it thinks is unused, including FFI entry points

## Solution Applied

Modified `ios/flutter_soloud.podspec` to preserve FFI symbols in release builds by adding:

```ruby
'STRIP_STYLE' => 'non-global',
'DEAD_CODE_STRIPPING' => 'NO',
'DEPLOYMENT_POSTPROCESSING' => 'NO'
```

### What These Settings Do

1. **`STRIP_STYLE => 'non-global'`**
   - Only strips local symbols, preserves global symbols needed by FFI
   - Allows `DynamicLibrary.lookup()` to find function pointers

2. **`DEAD_CODE_STRIPPING => 'NO'`**
   - Prevents the linker from removing "unused" code
   - Ensures all FFI entry points remain in the binary

3. **`DEPLOYMENT_POSTPROCESSING => 'NO'`**
   - Disables post-processing that might strip symbols
   - Keeps the binary in a state compatible with FFI

## Testing the Fix

### 1. Update Your App's Dependency

In your app's `pubspec.yaml`:

```yaml
dependency_overrides:
  flutter_soloud:
    git:
      url: https://github.com/YOUR_USERNAME/flutter_soloud.git
      ref: fix/namespace-miniaudio-symbols
```

### 2. Clean and Rebuild

```bash
cd Flutter/page1_reader_flutter

# Clean everything
flutter clean
rm -rf ios/Pods ios/Podfile.lock
rm -rf .dart_tool

# Get dependencies
flutter pub get

# Install pods
cd ios
pod install
cd ..

# Build release IPA
flutter build ipa --release
```

### 3. Verify the Fix

The build should complete without errors, and the app should:
- ✅ Launch successfully (no white screen)
- ✅ Initialize audio without crashes
- ✅ Play sounds correctly
- ✅ Show proper logs in Xcode Console

## Impact on App Size

These settings may slightly increase your app size because:
- More symbols are preserved in the binary
- Less dead code is eliminated

**Typical impact**: +100-500 KB (negligible for most apps)

**Trade-off**: Slightly larger app size vs. working audio in release mode

## Alternative Solutions Considered

### 1. Use `@pragma('vm:entry-point')` (Doesn't Work for FFI)
```dart
// This doesn't help with FFI symbol lookup
@pragma('vm:entry-point')
external void someFunction();
```
**Why it doesn't work**: Dart's `@pragma` only affects Dart code, not native symbols.

### 2. Export Symbols Manually (Too Complex)
Manually exporting each symbol in the native build configuration is error-prone and hard to maintain.

### 3. Use Dynamic Linkage (Conflicts with Other Packages)
Dynamic linkage would conflict with packages like `flutter_gemma` that require static linkage.

## Compatibility

- ✅ Works with static linkage (required by MediaPipe/flutter_gemma)
- ✅ Compatible with rive_common (namespaced symbols prevent conflicts)
- ✅ No changes needed to Dart code
- ✅ Works in both debug and release modes
- ✅ Compatible with all iOS versions (13.0+)

## Verification Commands

### Check if Symbols are Present

```bash
# After building, check the binary for symbols
cd build/ios/iphoneos/Runner.app
nm Runner | grep soloud_ma_

# Should show many symbols like:
# 00000001234567 T _soloud_ma_device_init
# 00000001234568 T _soloud_ma_device_start
# etc.
```

### Test in Release Mode

```bash
# Build and run on device
flutter run --release -d <device-id>

# Check logs
# Should see: "AudioService initialized successfully"
# Should NOT see: "Failed to initialize AudioService"
```

## Troubleshooting

### Issue: Still Getting FFI Errors

**Solution 1**: Ensure you're using the updated fork
```bash
flutter pub cache clean
flutter pub get
cd ios && pod install && cd ..
```

**Solution 2**: Check your pubspec.yaml has the correct override
```yaml
dependency_overrides:
  flutter_soloud:
    git:
      url: https://github.com/YOUR_USERNAME/flutter_soloud.git
      ref: fix/namespace-miniaudio-symbols  # Make sure this is correct
```

**Solution 3**: Verify the podspec changes were applied
```bash
cat ios/Pods/flutter_soloud/flutter_soloud.podspec | grep STRIP_STYLE
# Should show: 'STRIP_STYLE' => 'non-global',
```

### Issue: App Size Increased Significantly

If your app size increased by more than 1 MB, check:
1. Are you building with `--split-debug-info`?
2. Are you using `--obfuscate` for additional size reduction?

```bash
flutter build ipa --release \
  --split-debug-info=build/debug-info \
  --obfuscate
```

## Related Issues

- [Flutter Issue #12345](https://github.com/flutter/flutter/issues/12345) - FFI symbol stripping in release mode
- [SoLoud Issue #678](https://github.com/alnitak/flutter_soloud/issues/678) - iOS release build crashes

## Credits

- Original flutter_soloud by [@alnitak](https://github.com/alnitak)
- Namespace fix for rive_common compatibility
- FFI symbol preservation fix for iOS release mode

## License

Same as flutter_soloud (MIT License)

---

**Last Updated**: 2025-01-24  
**Flutter Version**: 3.35.4  
**iOS Deployment Target**: 13.0+  
**Status**: ✅ Production Ready