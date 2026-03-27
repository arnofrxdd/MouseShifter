# AI Prompt & Refactoring Methodology: "Include-Based Extraction"

This document serves as an instruction manual for AI models and developers to safely reduce the size of massive monolithic C++ files (specifically `MouseShifter.cpp` and `AppGlobals.h`) using an **Include-Based Extraction** approach. 

## The Core Problem
The codebase currently relies on a single translation unit (`MouseShifter.cpp`) hitting 10,000+ lines. Breaking these files into traditional header/cpp pairs immediately triggers `LNK2005` (Multiple Definition) errors due to variables inside `AppGlobals.h` lacking `extern` or `inline` declarations. 

To bypass this without rewriting thousands of variables, we extract large blocks of code from `MouseShifter.cpp` into new files (e.g., `Graphics.cpp`, `InputProcessing.cpp`) and dynamically inject an `#include "ExtractedFile.cpp"` at the exact cut-point.

---

## Step-by-Step Instruction Guide for AI Models

AI Models should execute the following strict looping sequence for *every single refactor step*. **Do NOT skip any of these steps.**

### STEP 0: Pre-Refactor Health Check (Baseline Build)
**Before touching ANY code**, you must verify that the project is currently in a compiling state. If the codebase is already broken, your extraction will fail validation and you won't know why.

1. **Run MSBuild.exe**:
   ```powershell
   & "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" "e:\MouseShifter Source\MouseShifter Refactor\MouseShifter\MouseShifter.sln" "/t:Rebuild" "/p:Configuration=Release" "/p:Platform=x64"
   ```
2. Check the output for `0 Error(s)`. If there are errors, stop and fix the baseline before proceeding.

### STEP 1: Identify Boundaries
You must identify a massive contiguous block of code that serves a cohesive purpose (e.g., 2,000 lines of UI Drawing or 1,000 lines of Controller Input).

1. Run `grep_search` or PowerShell `Select-String` to locate the starting lines of functions:
   ```powershell
   Select-String -Pattern "^\s*(?:[a-zA-Z_]\w+\*?\s+)+\w+\(" -Path "MouseShifter.cpp" | Select-Object -First 30
   ```
2. Identify the logical start line and end line of the chunk you want to extract.

### STEP 2: Strict Bounds Verification
You must absolutely verify the exact 0-indexed line bounds of your extraction to avoid cutting a function in half. 

1. Run a quick check using bounds to ensure the start and end tokens map to actual curly braces `{` / `}` or blank spaces:
   ```powershell
   $lines = Get-Content "MouseShifter.cpp" -Encoding UTF8
   Write-Host "Start: $($lines[7])"
   Write-Host "End: $($lines[2083])"
   ```

### STEP 3: Atomic Extraction via PowerShell Script
**CRITICAL:** Do NOT use multi-line `replace_file_content` IDE text tools or `Python` (if the user has the file open in an IDE like Visual Studio, Python's `codecs` can fail or corrupt line-endings (`\r\n`)).

Use this exact PowerShell script template to read the file, slice the array in memory, dump it to a new file, and inject the `#include` directive safely:

```powershell
$path = "e:\MouseShifter Source\MouseShifter Refactor\MouseShifter\RawMouse\MouseShifter.cpp"
$outPath = "e:\MouseShifter Source\MouseShifter Refactor\MouseShifter\RawMouse\NEW_MODULE_NAME.cpp"
$lines = Get-Content $path -Encoding UTF8

# Slice the exact bounds (e.g. lines 8 to 2084 becomes indices 7..2083)
$extractedLines = $lines[START_INDEX..END_INDEX]
[System.IO.File]::WriteAllLines($outPath, $extractedLines, [System.Text.Encoding]::UTF8)

# Re-assemble the original file with the include injected
$newlines = @()
$newlines += $lines[0..(START_INDEX - 1)]
$newlines += '#include "NEW_MODULE_NAME.cpp"'
$newlines += $lines[(END_INDEX + 1)..($lines.Count - 1)]

[System.IO.File]::WriteAllLines($path, $newlines, [System.Text.Encoding]::UTF8)
Write-Host "Extraction Success!"
```

### STEP 4: Post-Refactor Build Verification
Never extract a second chunk without verifying the first. Immediately trigger an MSBuild check again:

```powershell
& "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" "e:\MouseShifter Source\MouseShifter Refactor\MouseShifter\MouseShifter.sln" "/t:Rebuild" "/p:Configuration=Release" "/p:Platform=x64"
```

Verify that `0 Error(s)` are reported. If there are errors, revert the PowerShell script changes. If successful, you may proceed to identify the next block for Step 1.
