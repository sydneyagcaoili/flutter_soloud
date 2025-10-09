# Miniaudio Symbol Namespace Changes

## Overview

This fork of `flutter_soloud` includes namespaced miniaudio symbols to prevent conflicts with other packages that also include miniaudio (such as `rive_common`).

## Problem

When using `flutter_soloud` alongside `rive` (which depends on `rive_common`) in a Flutter iOS project with static linkage, the build fails with 988 duplicate symbol errors. Both packages statically link the miniaudio library, causing symbol conflicts:

```
duplicate symbol '_ma_device_init' in:
    flutter_soloud (miniaudio symbols)
    rive_common (miniaudio symbols)
```

## Solution

All miniaudio symbols have been namespaced to avoid conflicts:
- `ma_*` → `soloud_ma_*` (39,746 replacements)
- `MA_*` → `SOLOUD_MA_*` (15,970 replacements)

## Modified Files

1. `src/soloud/src/backend/miniaudio/miniaudio.h` - Main miniaudio header
2. `src/soloud/src/backend/miniaudio/soloud_miniaudio.cpp` - Backend implementation
3. `src/waveform/waveform.cpp` - Waveform processing
4. `src/waveform/miniaudio_libvorbis.h` - Vorbis decoder header
5. `src/waveform/miniaudio_libvorbis.cpp` - Vorbis decoder implementation

## Usage

### Option 1: Use from GitHub (Recommended)

After pushing this fork to your GitHub account, update your `pubspec.yaml`:

```yaml
dependencies:
  medee_audio:
    git:
      url: https://github.com/YOUR_USERNAME/flutter_soloud.git
      ref: fix/namespace-miniaudio-symbols
```

### Option 2: Use Local Path (For Testing)

```yaml
dependencies:
  flutter_soloud:
    path: /path/to/flutter_soloud_fork
```

## Reproducing the Changes

If you need to reapply the namespace changes or apply them to a newer version:

```bash
python3 namespace_miniaudio.py
```

The script will:
1. Create `.backup` files of all modified files
2. Replace all `ma_*` symbols with `soloud_ma_*`
3. Replace all `MA_*` macros with `SOLOUD_MA_*`
4. Report the number of changes made

## Compatibility

- ✅ Works with `rive_common` in static linkage mode
- ✅ Works with `flutter_gemma` (requires static linkage)
- ✅ Maintains all original flutter_soloud functionality
- ✅ No API changes - all public APIs remain the same

## Testing

After integrating this fork:

1. Clean your Flutter project:
   ```bash
   flutter clean
   cd ios && rm -rf Pods Podfile.lock && cd ..
   ```

2. Get dependencies:
   ```bash
   flutter pub get
   cd ios && pod install && cd ..
   ```

3. Build for iOS:
   ```bash
   flutter build ios
   ```

The build should complete without duplicate symbol errors.

## Original Repository

This is a fork of [flutter_soloud](https://github.com/alnitak/flutter_soloud) by alnitak.

## License

Same as the original flutter_soloud package (MIT License).

## Contributing

If you encounter any issues with the namespaced version, please open an issue in this repository.

## Branch Information

- **Branch**: `fix/namespace-miniaudio-symbols`
- **Base Version**: flutter_soloud 3.3.7
- **Commit**: Namespace miniaudio symbols to avoid conflicts with rive_common