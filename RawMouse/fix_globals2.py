import codecs

def fix_all_globals():
    filepath = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/AppGlobals.h'
    with codecs.open(filepath, 'r', 'utf-8-sig') as f:
        lines = f.readlines()

    out_lines = []
    ignore_prefixes = ('#', '//', '/*', 'struct', 'enum', 'class', 'inline', 'extern', 'void', 'using', 'namespace', 'typedef')
    
    for line in lines:
        s = line.strip()
        if not s or s.startswith(ignore_prefixes):
            out_lines.append(line)
            continue
        
        s_no_comment = s.split('//')[0].strip()
        
        if s_no_comment.endswith(';'):
            if '(' in s_no_comment and not ('=' in s_no_comment and s_no_comment.find('=') < s_no_comment.find('(')):
                out_lines.append(line)
                continue
            
            if s_no_comment.startswith('return '):
                out_lines.append(line)
                continue
                
            # Valid variable, prefix with inline
            out_lines.append('inline ' + line.lstrip())
        else:
            out_lines.append(line)
            
    with codecs.open(filepath, 'w', 'utf-8-sig') as f:
        f.writelines(out_lines)

if __name__ == "__main__":
    fix_all_globals()
