import codecs

def split_file():
    filepath = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/MouseShifter.cpp'
    with codecs.open(filepath, 'r', 'utf-8-sig') as f:
        lines = f.readlines()
        
    # lines 0 to 4160 are the globals and helpers (up to line 4161)
    globals_lines = lines[:4161]
    main_lines = ['#include "AppGlobals.h"\n'] + lines[4161:]
    
    with codecs.open('e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/AppGlobals.h', 'w', 'utf-8-sig') as f:
        f.writelines(globals_lines)
        
    with codecs.open(filepath, 'w', 'utf-8-sig') as f:
        f.writelines(main_lines)

if __name__ == "__main__":
    split_file()
