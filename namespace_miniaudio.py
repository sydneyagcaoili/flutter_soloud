#!/usr/bin/env python3
"""
Script to namespace miniaudio symbols in flutter_soloud to avoid conflicts with rive_common.
This script will replace all ma_* symbols with soloud_ma_* throughout the codebase.
"""

import os
import re
from pathlib import Path

# Define the root directory
ROOT_DIR = Path(__file__).parent

# Files to process
FILES_TO_PROCESS = [
    # Main miniaudio header
    "src/soloud/src/backend/miniaudio/miniaudio.h",
    # Backend implementation
    "src/soloud/src/backend/miniaudio/soloud_miniaudio.cpp",
    # Waveform files that use miniaudio
    "src/waveform/waveform.cpp",
    "src/waveform/miniaudio_libvorbis.h",
    "src/waveform/miniaudio_libvorbis.cpp",
    # Player files that reference miniaudio types
    "src/player.h",
    "src/player.cpp",
]

def namespace_file(filepath):
    """Namespace all ma_* symbols in a file to soloud_ma_*"""
    print(f"Processing: {filepath}")
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    original_content = content
    
    # Pattern to match ma_ symbols
    # We need to be careful to match:
    # - Function definitions: ma_device_init
    # - Function calls: ma_device_init()
    # - Type definitions: ma_device
    # - Struct members: device.ma_something
    # - Macros: MA_SOMETHING
    # But NOT match things already prefixed or in comments
    
    # First, handle MA_ macros (uppercase)
    content = re.sub(r'\bMA_', 'SOLOUD_MA_', content)
    
    # Then handle ma_ identifiers (lowercase/mixed case)
    # This regex matches ma_ followed by word characters, but not if already prefixed
    content = re.sub(r'(?<!soloud_)\bma_', 'soloud_ma_', content)
    
    if content != original_content:
        # Create backup
        backup_path = str(filepath) + '.backup'
        with open(backup_path, 'w', encoding='utf-8') as f:
            f.write(original_content)
        print(f"  ✓ Created backup: {backup_path}")
        
        # Write modified content
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"  ✓ Namespaced symbols in: {filepath}")
        
        # Count changes
        ma_count = len(re.findall(r'\bsoloud_ma_', content))
        MA_count = len(re.findall(r'\bSOLOUD_MA_', content))
        print(f"  ✓ Found {ma_count} soloud_ma_ symbols and {MA_count} SOLOUD_MA_ macros")
    else:
        print(f"  - No changes needed")
    
    return content != original_content

def main():
    print("=" * 80)
    print("Namespacing miniaudio symbols in flutter_soloud")
    print("=" * 80)
    print()
    
    changes_made = False
    
    for file_path in FILES_TO_PROCESS:
        full_path = ROOT_DIR / file_path
        if full_path.exists():
            if namespace_file(full_path):
                changes_made = True
        else:
            print(f"WARNING: File not found: {full_path}")
        print()
    
    if changes_made:
        print("=" * 80)
        print("✓ Namespacing complete!")
        print("=" * 80)
        print()
        print("Next steps:")
        print("1. Review the changes in the modified files")
        print("2. Test the package locally")
        print("3. Update your pubspec.yaml to use this forked version")
        print()
        print("To restore original files, use the .backup files created")
    else:
        print("=" * 80)
        print("No changes were needed - files may already be namespaced")
        print("=" * 80)

if __name__ == "__main__":
    main()