# AI Prompt & Refactoring Methodology: "Include-Based Extraction"

This document serves as an instruction manual for AI models and developers to safely reduce the size of massive monolithic C++ files (specifically `MouseShifter.cpp` and `AppGlobals.h`) using an **Include-Based Extraction** approach. 

## The Core Problem
The codebase currently relies on a single translation unit (`MouseShifter.cpp`) hitting 10,000+ lines. Breaking these files into traditional header/cpp pairs immediately triggers `LNK2005` (Multiple Definition) errors due to variables inside `AppGlobals.h` lacking `extern` or `inline` declarations. 

To bypass this without rewriting thousands of variables, we extract large blocks of code from `MouseShifter.cpp` into new files (e.g., `Graphics.cpp`, `InputProcessing.cpp`) and dynamically inject an `#include "ExtractedFile.cpp"` at the exact cut-point.

---

# 🚨 MOUSE SHIFTER AI PRIME DIRECTIVE 🚨

**ATTENTION ALL FUTURE AI AGENTS:** 
By reading this file, you are bound by the active architecture rules of the `MouseShifter` repository. Failure to comply will result in build corruption.

## 1. THE ARCHITECTURE: "UNITY BUILD INJECTION"
This project does **NOT** use traditional C++ header/source separation for its modules. 
It uses **Include-Based Extraction (Unity Build)**.

- **DO NOT** attempt to create `.h` files for newly extracted modules.
- **DO NOT** attempt to write `extern` for global variables to share state.
- **DO NOT** add standard `#pragma once` or independent `#include <windows.h>` headers to sub-modules.

All files in `UI/`, `Input/`, and `Windows/` are built to be physically injected directly into the master `MouseShifter.cpp` hub (or its immediate children) via `#include "Module.cpp"`.
Because they compile as a single translation unit, they automatically share all `static` variables and `AppGlobals.h` references.

***Example of the Router Architecture:***
```cpp
// Inside MouseShifter.cpp
#include "UI/Graphics_StaticElements.cpp"
#include "Input/Input_Pedals.cpp"
```

## 2. THE BUSINESS LOGIC FREEZE
- **DO NOT** attempt to "optimize" or "rewrite" mathematical business logic while doing structural refactoring.
- The physics engine (`Input_*.cpp`), Reverse Lock logic, and slider math (`MouseEvents_*.cpp`) are highly tuned natively.
- You are authorized to *move* code. You are **strictly forbidden** from *changing* what the code does without explicit user permission.

## 3. IDENTIFYING BRACKET LEAKS (`C2601` Errors)
When extracting code via array slices, always match your `{ }` brackets. 
If `MSBuild.exe` throws `error C2601: 'FunctionName': local function definitions are illegal`, it means a preceding module **failed to close a bracket**, causing the compiler to think the current file is still inside a parent function scope. Trace the brackets backward through the `#include` router.

## 4. IDE COMPILER TRACKING (`.vcxproj`)
When generating new modules (e.g., `Settings_Sliders.cpp`), you must inform Visual Studio of their existence so they appear in the Solution Explorer, but **they must not be compiled independently!**

- **DO NOT** add new extracted fragments to the `<ClCompile>` tags in `.vcxproj`.
- **DO** add them to the `<ClInclude>` (Headers) section instead. 
- MSBuild will skip compiling them natively, but will perfectly inline them when the root `MouseShifter.cpp` is built.

---

## Step-by-Step Instruction Guide for AI Models

### 🚨 AI PRIME DIRECTIVE: NO LOGIC DELETION OR HALLUCINATION 🚨
**Under NO circumstances should an AI model analyze this codebase and decide to "delete", "optimize", or "rewrite" existing business logic, loops, or variables.** 
Your ONLY job is pure structural refactoring. You are physically moving 1:1 identical chunks of raw code out of a massive text file into a secondary `.cpp` file and replacing the chunk with an `#include`. 
**Do NOT hallucinate changes. Do NOT rewrite function signatures. Do NOT delete what you assume is dead code.** You must treat the exact text as a fragile monolith. Failure to adhere to this strictly will brutally break the interconnected `MouseShifter` state machine.

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
