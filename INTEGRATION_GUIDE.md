# Flutter Soloud Fork - Integration Guide

## 🎯 What We've Done

Successfully forked and modified `flutter_soloud` to namespace all miniaudio symbols, preventing conflicts with `rive_common`. This allows you to use both audio features and Rive animations in your iOS app with static linkage.

### Changes Summary
- ✅ Cloned flutter_soloud repository
- ✅ Created Python script to automate symbol namespacing
- ✅ Replaced 39,746 `ma_*` symbols with `soloud_ma_*`
- ✅ Replaced 15,970 `MA_*` macros with `SOLOUD_MA_*`
- ✅ Modified 5 critical files
- ✅ Committed changes to branch `fix/namespace-miniaudio-symbols`
- ✅ Created documentation

## 📋 Next Steps for You

### Step 1: Push to Your GitHub Account

1. **Create a new repository on GitHub:**
   - Go to https://github.com/new
   - Name it: `flutter_soloud` (or `flutter_soloud_fork`)
   - Make it public or private (your choice)
   - **Do NOT** initialize with README, .gitignore, or license

2. **Push your local fork:**
   ```bash
   cd /Users/sydneyagcaoili/MilkdromedaProjects/flutter_soloud_fork
   
   # Add your GitHub repository as remote
   git remote remove origin  # Remove the original repo
   git remote add origin https://github.com/YOUR_USERNAME/flutter_soloud.git
   
   # Push the branch
   git push -u origin fix/namespace-miniaudio-symbols
   ```

   Replace `YOUR_USERNAME` with your actual GitHub username.

### Step 2: Update Your Medee Clinic Project

Navigate to your medee_clinic project and update the dependency:

```bash
cd /Users/sydneyagcaoili/MilkdromedaProjects/Flutter/medee_clinic
```

#### Option A: Update medee-common Repository (Recommended)

Since `medee_audio` comes from the medee-common repository, you should update it there:

1. Clone or navigate to your medee-common repository
2. Find the `packages/audio/pubspec.yaml` file
3. Update the `flutter_soloud` dependency:

```yaml
dependencies:
  flutter_soloud:
    git:
      url: https://github.com/YOUR_USERNAME/flutter_soloud.git
      ref: fix/namespace-miniaudio-symbols
```

4. Commit and push the changes
5. Update the medee-common version reference in your medee_clinic `pubspec.yaml`

#### Option B: Override Directly in medee_clinic (Quick Test)

Add a dependency override in `Flutter/medee_clinic/pubspec.yaml`:

```yaml
dependency_overrides:
  flutter_soloud:
    git:
      url: https://github.com/YOUR_USERNAME/flutter_soloud.git
      ref: fix/namespace-miniaudio-symbols
  pointycastle: ^4.0.0  # Keep existing override
```

### Step 3: Clean and Rebuild

```bash
cd /Users/sydneyagcaoili/MilkdromedaProjects/Flutter/medee_clinic

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

# Build for iOS
flutter build ios --debug
```

### Step 4: Verify the Fix

The build should now complete without the 988 duplicate symbol errors. You should see:

✅ **Before (Error):**
```
duplicate symbol '_ma_device_init' in:
    flutter_soloud
    rive_common
... (988 duplicate symbols)
```

✅ **After (Success):**
```
Build succeeded
```

## 🔍 Troubleshooting

### Issue: Still Getting Duplicate Symbols

**Solution:** Make sure you've cleaned everything:
```bash
flutter clean
cd ios
rm -rf Pods Podfile.lock .symlinks
pod deintegrate
pod install
cd ..
```

### Issue: Git Push Fails

**Solution:** Check your GitHub credentials:
```bash
# Use SSH instead of HTTPS
git remote set-url origin git@github.com:YOUR_USERNAME/flutter_soloud.git
git push -u origin fix/namespace-miniaudio-symbols
```

### Issue: Dependency Not Updating

**Solution:** Force Flutter to re-fetch:
```bash
flutter pub cache clean
flutter pub get
```

## 📊 What Changed in the Code

### Before (Original)
```c
ma_device_config deviceConfig;
ma_device_init(&device, &deviceConfig);
MA_SAMPLE_FORMAT_F32
```

### After (Namespaced)
```c
soloud_ma_device_config deviceConfig;
soloud_ma_device_init(&device, &deviceConfig);
SOLOUD_MA_SAMPLE_FORMAT_F32
```

All changes are internal to flutter_soloud - **no changes needed in your app code**.

## 🎉 Benefits

1. ✅ Keep both audio features (flutter_soloud) and animations (rive)
2. ✅ No more duplicate symbol errors
3. ✅ Works with static linkage (required by MediaPipe/flutter_gemma)
4. ✅ No API changes - your existing code works as-is
5. ✅ Reproducible with the included Python script

## 📝 Files Modified

Located in `/Users/sydneyagcaoili/MilkdromedaProjects/flutter_soloud_fork/`:

1. `src/soloud/src/backend/miniaudio/miniaudio.h` - 39,372 symbols + 15,836 macros
2. `src/soloud/src/backend/miniaudio/soloud_miniaudio.cpp` - 45 symbols + 20 macros
3. `src/waveform/waveform.cpp` - 35 symbols + 7 macros
4. `src/waveform/miniaudio_libvorbis.h` - 55 symbols + 9 macros
5. `src/waveform/miniaudio_libvorbis.cpp` - 239 symbols + 98 macros

Backup files (`.backup`) are included for reference.

## 🔄 Updating to Newer flutter_soloud Versions

If flutter_soloud releases a new version and you want to update:

```bash
cd /Users/sydneyagcaoili/MilkdromedaProjects/flutter_soloud_fork

# Fetch latest from original repo
git remote add upstream https://github.com/alnitak/flutter_soloud.git
git fetch upstream

# Merge or rebase
git merge upstream/main
# or
git rebase upstream/main

# Re-run the namespace script
python3 namespace_miniaudio.py

# Commit and push
git add -A
git commit -m "Update to flutter_soloud vX.X.X with namespaced symbols"
git push origin fix/namespace-miniaudio-symbols
```

## 📞 Support

If you encounter any issues:

1. Check the `NAMESPACE_CHANGES.md` file for detailed information
2. Review the backup files to see what changed
3. Run the `namespace_miniaudio.py` script again if needed
4. Check that your GitHub repository URL is correct in pubspec.yaml

## ✨ Success Criteria

You'll know it worked when:
- ✅ `flutter build ios` completes without errors
- ✅ No "duplicate symbol" errors in the build log
- ✅ Your app runs on iOS simulator/device
- ✅ Audio features work correctly
- ✅ Rive animations work correctly

---

**Created:** 2025-01-09  
**Flutter Soloud Version:** 3.3.7  
**Branch:** fix/namespace-miniaudio-symbols