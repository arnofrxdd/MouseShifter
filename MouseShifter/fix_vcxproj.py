import codecs
import re

def fix_vcxproj():
    filepath = 'e:/MouseShifter Source/MouseShifter Refactor/MouseShifter/RawMouse/RawMouse.vcxproj'
    with codecs.open(filepath, 'r', 'utf-8') as f:
        content = f.read()

    # Find <ItemDefinitionGroup> sections containing <ClCompile> and inject <LanguageStandard>stdcpp17</LanguageStandard>
    # We can just replace <ClCompile> with <ClCompile>\n      <LanguageStandard>stdcpp17</LanguageStandard>
    
    content = content.replace('<ClCompile>', '<ClCompile>\n      <LanguageStandard>stdcpp17</LanguageStandard>')
    
    with codecs.open(filepath, 'w', 'utf-8') as f:
        f.write(content)

if __name__ == "__main__":
    fix_vcxproj()
