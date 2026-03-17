import re
import sys

def process():
    with open('e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/MouseShifter.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # We will split lines into:
    # 1. Includes/Pragmas -> both or Main
    # 2. Structs/Enums -> AppGlobals.h
    # 3. Global variables -> extern in .h, definition in .cpp
    # 4. Functions -> Main

    includes_h = [
        '#pragma once',
        '#include <windows.h>',
        '#include <d2d1.h>',
        '#include <math.h>',
        '#include <fstream>',
        '#include <string>',
        '#include <tchar.h>',
        '#include <tlhelp32.h>',
        '#include <map>',
        '#include <cmath>',
        '#include <windowsx.h>',
        '#include "public.h"',
        '#include "vjoyinterface.h"',
        '#include <Xinput.h>',
        '#include <gdiplus.h>',
        '#include <sstream>',
        '#include <vector>',
        '#include <algorithm>',
        '#include <mutex>',
        '#include <chrono>',
        '#include <SDL.h>',
        '#include <SDL_syswm.h>',
        '#include <dinput.h>',
        '#include <wininet.h>',
        'using namespace Gdiplus;',
        ''
    ]

    globals_h = []
    globals_cpp = ['#include "AppGlobals.h"', '']
    main_cpp = ['#include "AppGlobals.h"', '']

    brace_level = 0
    in_function = False
    in_struct_enum = False
    
    current_block = []
    
    # regexes
    # matches var: Type name = val; or Type name;
    # not matching functions or struct/enum
    
    for i in range(4161):
        line = lines[i]
        stripped = line.strip()
        
        # brace counting
        open_b = line.count('{')
        close_b = line.count('}')
        
        # If we are at root level
        if brace_level == 0:
            # Check if this line starts a struct or enum
            if stripped.startswith('struct ') or stripped.startswith('enum ') or stripped.startswith('typedef '):
                in_struct_enum = True
            
            # Check if this line starts a function
            # Rough heuristic: has word, space, word, parens, no trailing semicolon
            elif not in_struct_enum and not stripped.startswith('#') and not stripped.startswith('//') and not stripped == '':
                if re.search(r'\w+\s+[*&]*\w+\s*\(.*?\)\s*(?:const)?\s*(?:\{.*)?$', stripped) and not stripped.endswith(';'):
                    if not (stripped.startswith('return ') or stripped.startswith('if ') or stripped.startswith('else ')):
                        in_function = True
                        
        brace_level += open_b
        current_block.append(line)
        
        brace_level -= close_b
        
        if brace_level == 0 and stripped != '':
            # Block ended
            if in_function:
                # Keep in main.cpp
                main_cpp.extend(current_block)
            elif in_struct_enum:
                # Move to AppGlobals.h
                globals_h.extend(current_block)
            elif stripped.startswith('#include') or stripped.startswith('#pragma'):
                # includes handled manually above, but we can keep pragmas in main too just in case
                main_cpp.extend(current_block)
            elif stripped.startswith('//'):
                # comment at root
                globals_h.extend(current_block)
                globals_cpp.extend(current_block)
            else:
                # It's a global variable declaration!
                # e.g. int gearRadius = 30;
                # We need to split into extern and def.
                text = "".join(current_block).strip()
                if '=' in text:
                    # has initializer
                    # "Type name = val;"
                    # wait, might be multiple statements? usually one per block if brace_level=0
                    # let's just do:
                    # extern Type name; (extracting before =)
                    # definition: Type name = val;
                    parts = text.split('=', 1)
                    left_side = parts[0].strip()
                    
                    # Watch out for array inits: int arr[] = {1,2};
                    # We can't just do extern int arr[]; we need the exact type.
                    # Or structs.
                    # Let's write the whole text to .cpp
                    globals_cpp.append("".join(current_block))
                    
                    # For .h, just write extern + left_side + ;
                    # Remove static if present
                    left_side = re.sub(r'^static\s+', '', left_side)
                    
                    if '[' in left_side and ']' in left_side:
                        globals_h.append(f"extern {left_side};\n")
                    else:
                        globals_h.append(f"extern {left_side};\n")
                else:
                    # No init: bool showSettingsPanel;
                    left_side = text.rstrip(';')
                    left_side = re.sub(r'^static\s+', '', left_side)
                    globals_h.append(f"extern {left_side};\n")
                    globals_cpp.append("".join(current_block))
            
            # reset block
            current_block = []
            in_function = False
            in_struct_enum = False
            
    # append the rest of the file
    main_cpp.extend(lines[4161:])
    
    # write out files
    with open('e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/AppGlobals.h', 'w', encoding='utf-8') as f:
        f.write("\n".join(includes_h))
        f.writelines(globals_h)
        
    with open('e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/AppGlobals.cpp', 'w', encoding='utf-8') as f:
        f.writelines(globals_cpp)
        
    with open('e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/MouseShifter_new.cpp', 'w', encoding='utf-8') as f:
        f.writelines(main_cpp)

if __name__ == "__main__":
    process()
