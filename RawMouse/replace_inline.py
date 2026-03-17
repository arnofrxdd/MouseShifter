import codecs

def replace_inline():
    filepath = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/AppGlobals.h'
    with codecs.open(filepath, 'r', 'utf-8-sig') as f:
        lines = f.readlines()

    out_lines = []
    
    for line in lines:
        if line.lstrip().startswith('inline '):
            out_lines.append(line.replace('inline ', '__declspec(selectany) ', 1))
        else:
            out_lines.append(line)
            
    with codecs.open(filepath, 'w', 'utf-8-sig') as f:
        f.writelines(out_lines)

if __name__ == "__main__":
    replace_inline()
