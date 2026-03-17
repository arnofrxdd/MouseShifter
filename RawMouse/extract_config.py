import codecs

def extract_config():
    ag_path = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/AppGlobals.h'
    cfg_cpp_path = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/Config.cpp'
    cfg_h_path = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/Config.h'
    
    with codecs.open(ag_path, 'r', 'utf-8-sig') as f:
        lines = f.readlines()
        
    start_idx = 2782  # Line 2783
    end_idx = 3293    # Line 3293
    
    config_lines = lines[start_idx:end_idx]
    
    # Remove from AppGlobals
    lines[start_idx:end_idx] = ['#include "Config.h"\n']
    
    with codecs.open(ag_path, 'w', 'utf-8-sig') as f:
        f.writelines(lines)
        
    # Write Config.cpp
    cpp_lines = [
        '#include "Config.h"\n',
        '#include "AppGlobals.h"\n',  # Needed for globals
        '\n'
    ] + config_lines
    
    with codecs.open(cfg_cpp_path, 'w', 'utf-8-sig') as f:
        f.writelines(cpp_lines)
        
    # Create Config.h
    sigs = [
        '#pragma once',
        '#include <string>',
        '#include <windows.h>',
        '',
        'void SaveConfig();',
        'void LoadConfig();',
        'void InitializeProfiles();',
        'void RefreshProfilesList();',
        'void CreateNewProfile(HWND hwnd);',
        'void SwitchProfile(int index, HWND hwnd);',
        ''
    ]
    with codecs.open(cfg_h_path, 'w', 'utf-8-sig') as f:
        f.write('\n'.join(sigs))

if __name__ == "__main__":
    extract_config()
