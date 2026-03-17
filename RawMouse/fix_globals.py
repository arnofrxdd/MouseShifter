import codecs
import re

def fix_globals():
    filepath = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/AppGlobals.h'
    with codecs.open(filepath, 'r', 'utf-8-sig') as f:
        lines = f.readlines()

    out_lines = []
    
    # Simple regex to catch clear variable declarations and avoid structs/enums/functions
    # Examples:
    # int vjoyDeviceId = 1;
    # bool useXInput = false;
    # RECT knobAccelToggleRect = { 0, 0, 0, 0 };
    # std::map<std::string, POINT> lowerGearPositions;
    
    # We look for lines that:
    # 1. Do not start with #, //, struct, enum, void, std::string
    # 2. End with ;
    # 3. Contain = or are simple declarations
    # 4. Don't have { or } (except for assignment like RECT r = {0};)
    
    for line in lines:
        stripped = line.strip()
        
        # Skip empty lines, comments, preprocessor, and things that are already inline/extern
        if not stripped or stripped.startswith(('//', '#', 'struct', 'enum', 'void', 'inline', 'extern', 'class')):
            out_lines.append(line)
            continue
            
        # Catch typical variable definitions like `int x = 1;` or `bool y = false;` or `const float z = 1.0f;`
        # Check if it ends with a semicolon
        if stripped.endswith(';'):
            # If it looks like a function declaration (has parenthesis but no '=' before the parenthesis)
            if '(' in stripped and not ('=' in stripped and stripped.find('=') < stripped.find('(')):
                pass # Probably a function
            # Extra safety, make sure it's not a using statement
            elif stripped.startswith('using'):
                pass
            # Extra safety, skip if it has 'return' or '}' or '{' (unless initialized struct)
            elif 'return ' in stripped or (('{' in stripped or '}' in stripped) and '=' not in stripped):
                pass
            else:
                # Add inline
                line = 'inline ' + line
        
        out_lines.append(line)

    with codecs.open(filepath, 'w', 'utf-8-sig') as f:
        f.writelines(out_lines)

if __name__ == "__main__":
    fix_globals()
