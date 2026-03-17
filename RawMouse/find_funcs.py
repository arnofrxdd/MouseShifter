import re
import sys

def find_functions():
    with open('e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/MouseShifter.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()
        
    brace_level = 0
    in_function = False
    func_start = -1
    funcs = []
    
    for i, line in enumerate(lines[:4160]):
        stripped = line.strip()
        
        # Check if line looks like a function signature and we are at top level
        # We ignore lines starting with 'struct', 'enum', 'class', 'typedef', '#', or ending with ';'
        if brace_level == 0 and not in_function:
            if re.match(r'^[a-zA-Z_][a-zA-Z0-9_: <>*&]*\s+[a-zA-Z0-9_]+\s*\([^;]*$', stripped) or \
               re.match(r'^LRESULT\s+CALLBACK\s+[a-zA-Z0-9_]+\s*\(', stripped) or \
               re.match(r'^DWORD\s+WINAPI\s+[a-zA-Z0-9_]+\s*\(', stripped):
                if not stripped.endswith(';') and not stripped.startswith('return') and not stripped.startswith('if') and not stripped.startswith('for') and not stripped.startswith('while') and not stripped.startswith('switch'):
                    in_function = True
                    func_start = i
        
        # Track braces
        # A line can have multiple { and }
        for char in line:
            # Not handling comments or strings perfectly, but should be OK for this well-formatted file
            if char == '{':
                brace_level += 1
            elif char == '}':
                brace_level -= 1
                if brace_level == 0 and in_function:
                    funcs.append((func_start, i))
                    in_function = False
                    
    print("Found functions at lines:")
    for start, end in funcs:
        print(f"{start+1} - {end+1}: {lines[start].strip()}")

find_functions()
