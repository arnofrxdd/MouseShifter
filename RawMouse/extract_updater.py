import codecs
import re

def extract_updater():
    ag_path = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/AppGlobals.h'
    up_cpp_path = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/Updater.cpp'
    up_h_path = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/Updater.h'
    
    with codecs.open(ag_path, 'r', 'utf-8-sig') as f:
        lines = f.readlines()
        
    # Lines 844-1132 are 0-indexed as 843-1131
    start_idx = 843
    end_idx = 1132
    
    updater_lines = lines[start_idx:end_idx]
    
    # Remove from AppGlobals
    lines[start_idx:end_idx] = ['#include "Updater.h"\n']
    
    with codecs.open(ag_path, 'w', 'utf-8-sig') as f:
        f.writelines(lines)
        
    # Write Updater.cpp
    cpp_lines = [
        '#include "AppGlobals.h"\n',
        '#include "Updater.h"\n',
        '\n'
    ] + updater_lines
    
    with codecs.open(up_cpp_path, 'w', 'utf-8-sig') as f:
        f.writelines(cpp_lines)
        
    # Create Updater.h
    # Extract function signatures to put in header
    sigs = [
        '#pragma once',
        '#include <string>',
        '#include <windows.h>',
        '',
        'int CompareVersions(const std::string& a, const std::string& b);',
        'bool IsInternetAvailable();',
        'std::string DownloadString(const std::string& url);',
        'bool DownloadFile(const std::string& url, const std::wstring& localPath);',
        'bool ExtractZipNew(const std::wstring& zipPath, const std::wstring& extractPath);',
        'void CheckForUpdates();',
        'void PerformUpdate();',
        ''
    ]
    with codecs.open(up_h_path, 'w', 'utf-8-sig') as f:
        f.write('\n'.join(sigs))

if __name__ == "__main__":
    extract_updater()
