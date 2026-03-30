# 🤝 Contributing to MouseShifter

Thank you for your interest in contributing to **MouseShifter**! This guide explains how to contribute effectively while respecting the project's architecture and code standards.

---

## 📋 Table of Contents

- [Code of Conduct]( #code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Architecture You Must Understand First](#architecture-you-must-understand-first)
- [Setting Up Your Dev Environment](#setting-up-your-dev-environment)
- [Code Style](#code-style)
- [Adding a New Feature](#adding-a-new-feature)
- [Adding a New Fragment File](#adding-a-new-fragment-file)
- [Pull Request Process](#pull-request-process)
- [Reporting Bugs](#reporting-bugs)

---

## 📜 Code of Conduct

Be respectful, constructive, and patient. This is a solo-maintainer project — responses may not be instant.

---

## 💡 How Can I Contribute?

- **🐛 Bug reports** — open an Issue with a minimal reproduction case
- **✨ Feature requests** — open an Issue with a clear description and use case
- **🔧 Code contributions** — fork → branch → implement → PR
- **📖 Documentation** — improve this README or the `Docs/MODULARIZATION_GUIDE.md`
- **🎮 Layout packs** — contribute new `gearlayouts.ini` presets for popular sim games

---

## 🏗️ Architecture You Must Understand First

MouseShifter uses a non-standard **Unity Build / Router-Fragment** architecture. **Read this section before writing a single line of code.**

### How it Works

There is **only one real compilation unit**: `MouseShifter.cpp`. Every other `.cpp` file in the project is included into it (directly or transitively via router files) using `#include`. None of them are compiled independently.

```
MouseShifter.cpp  →  #includes router files
                        ↓
              Router files  →  #includes fragment files
                                  ↓
                        Fragment files  →  actual logic
```

### Three Types of Files

| Type | Purpose | Contains |
|---|---|---|
| **Router** | Glues fragments together | Only `#include` directives |
| **Fragment** | Contains actual logic | Functions, global variable definitions |
| **Header** | Declares types & globals | `struct`, `enum`, `extern`, constants |

### Rules

1. **Never add a `<ClCompile>` entry for a fragment file in the `.vcxproj`.** Always use `<ClInclude>` so the IDE can index the file without compiling it independently.
2. **Never add `#pragma once` to a fragment file.** Only headers use `#pragma once`.
3. **Fragments are not allowed to `#include` headers from other domains** unless those headers are already included higher in the include chain.
4. **All global state must be defined exactly once.** Because everything is one translation unit, duplicate definitions will cause linker errors. Use `Core/Globals/` for all shared state.
5. **The include order within a router must match dependency order.** If fragment B calls a function defined in fragment A, A must be included first.

### Domain Structure

```
Core/           ← Configuration, global state, system utilities
Input/          ← Hardware input: vJoy, Raw Input, DirectInput, XInput
Windows/        ← WndProc message handling, timer callbacks
UI/             ← All rendering: GDI+, drawing, settings panels
```

---

## 🛠️ Setting Up Your Dev Environment

**Requirements:**
- Windows 10 or 11 (64-bit)
- Visual Studio 2019 or later with **Desktop development with C++** workload
- Windows 10 SDK

**Steps:**
1. Fork this repository on GitHub.
2. Clone your fork locally.
3. Open `MouseShifter/MouseShifter.vcxproj` in Visual Studio.
4. Set the build target to **Release | x64** (or Debug | x64 for debugging).
5. Press `Ctrl+Shift+B` to verify a clean build before making changes.

**Verify the build is green before you start.** A clean build must produce **0 Errors**.

---

## 🎨 Code Style

This project uses standard C++ with Win32 API conventions. Follow these rules:

### Naming
| Entity | Convention | Example |
|---|---|---|
| Global variables | `camelCase` | `knobSensitivity`, `activeGear` |
| Functions | `PascalCase` | `ComputeLayout()`, `LoadConfig()` |
| Boolean flags | `is` / `use` / `show` prefix | `isBorderless`, `useXInput`, `showSettingsPanel` |
| RECT variables | End in `Rect` | `knobSliderRect`, `reverseLockToggleRect` |
| Enums | `ALL_CAPS` values | `TOGGLE_KEYBOARD`, `TOGGLE_PEDAL_CLUTCH` |
| Constants | `UPPER_CASE` or `constexpr` | `MAX_GLOW_ALPHA`, `GLOW_FADE_IN_TIME` |

### Formatting
- **Indentation:** 4 spaces (no tabs)
- **Braces:** K&R style — opening brace on same line for function bodies
- **Line length:** Aim for under 120 characters
- **Comments:** Use `// ---` separator lines to delineate major sections within a file

### Includes
- Use `#include <...>` for system/SDK headers
- Use `#include "..."` for project headers and fragment files
- Use absolute-from-root relative paths in include strings, e.g., `"Core/Globals/AppGlobals.h"` not `"../../AppGlobals.h"`

---

## 🛠️ Adding a New Feature

### Step 1: Identify the Domain

| Feature type | Where it goes |
|---|---|
| New input method (new device type) | `Input/Submodules/` |
| New vJoy functionality | `Input/VJoy/` |
| New window message handler | `Windows/Handlers/` or `Windows/Mechanics/` |
| New timer callback | `Windows/Timer/` |
| New mouse event handler | `Windows/MouseEvents/` |
| New drawing/rendering | `UI/Drawing/` or `UI/Handlers/` |
| New settings panel section | `UI/Settings/` |
| New config option (save/load) | `Core/Config/` |
| New global state | `Core/Globals/` |
| New system utility (update, backup) | `Core/System/` |

### Step 2: Add Global State (if needed)

Add any required global variables to the appropriate file in `Core/Globals/`:

- **UI state** (booleans, RECTs for new UI controls) → `Globals_UI.h`
- **Device/hardware state** → `Globals_Devices.h`
- **Config/input settings** → `Globals_ConfigInput.h`

> Do **not** define globals inside fragment files that other fragments also use — define them once in the appropriate header.

### Step 3: Implement the Logic

Create a new fragment file in the appropriate subdirectory:

```cpp
// Example: UI/Settings/Settings_MyNewFeature.cpp
// NO #pragma once
// NO #include for things already included above in the chain
// Just write the functions

void DrawMyNewFeaturePanel(HDC hdc, RECT& panelRect)
{
    // logic here
}
```

### Step 4: Register in the Router

Open the router for your domain and add an `#include` for your new fragment:

```cpp
// Example: adding to UI/Handlers/ShifterUI_Settings.cpp (or a relevant router)
#include "Settings/Settings_MyNewFeature.cpp"
```

Ensure the include position is correct based on dependencies.

### Step 5: Register in the `.vcxproj`

Add your new file as a `<ClInclude>` in `MouseShifter.vcxproj`:

```xml
<ItemGroup>
  <ClInclude Include="UI\Settings\Settings_MyNewFeature.cpp" />
</ItemGroup>
```

> ⚠️ **Never** use `<ClCompile>` for a fragment file. Always use `<ClInclude>`.

### Step 6: Update Config (if the feature needs persistence)

- **Save:** Add a `WritePrivateProfileString` call in `Core/Config/Config_Save.cpp`
- **Load:** Add a `GetPrivateProfileString` / `GetPrivateProfileInt` call in `Core/Config/Config_Load.cpp`

### Step 7: Register in the Settings Registry (if it is a setting)

If your feature adds a new setting (slider or toggle), you should register it in `UI/Settings/Settings_VariablesAndTitles.cpp`:

1.  **Variable:** Ensure your variable is declared in a global header (e.g., `Globals_ConfigInput.h`).
2.  **Registry Push:** Add a `g_settingsRegistry.push_back(...)` call in `Settings_VariablesAndTitles.cpp`.
3.  **Special Logic:** If the setting requires per-change updates (like recomputing layouts), add the logic to:
    *   `LButtonDown_Settings.cpp` (inside the registry loop's slider/toggle handlers).
    *   `MouseEvents_MouseMove.cpp` (inside the `g_draggingElement` handler for real-time slider updates).

### Step 8: Build and Verify

```
Build → Rebuild Solution (Ctrl+Shift+B)
```

The build **must produce 0 Errors** before submitting a PR.

---

## 📁 Adding a New Fragment File

Quick checklist for any new `.cpp` fragment:

- [ ] No `#pragma once`
- [ ] No independent `#include <windows.h>` etc. (already included higher in the chain)
- [ ] Placed in the correct subdirectory for the domain
- [ ] Included in the parent router at the correct position
- [ ] Registered as `<ClInclude>` in `MouseShifter.vcxproj`
- [ ] Build passes with **0 Errors**

---

## 🔀 Pull Request Process

1. **Fork** the repository and create a descriptively named branch:
   ```
   git checkout -b feature/my-new-feature
   ```

2. **Make your changes** following the architecture and style rules above.

3. **Build and verify** — the solution must compile with **0 Errors**.

4. **Write a clear commit message:**
   ```
   feat(Input): add DirectInput axis smoothing toggle
   
   - Added axisSmoothingFactor slider to Settings panel
   - New global useAxisSmoothing in Globals_Devices.h
   - Persisted in Config_Save / Config_Load
   - Fragment: Input/Submodules/Input_AxisSmoothing.cpp
   ```

5. **Open a Pull Request** against the `main` branch with:
   - A description of what changed and why
   - Which files were added/modified
   - A note confirming the build is clean

6. **Respond to review feedback** promptly.

---

## 🐛 Reporting Bugs

Open a GitHub Issue and include:

- **OS version** (e.g., Windows 11 22H2)
- **MouseShifter version** (from title bar or About dialog)
- **Steps to reproduce** — be specific
- **Expected behaviour**
- **Actual behaviour**
- **Debug output** (if available — copy from the Visual Studio Output window or attach a debugger)

---

## 📖 Further Reading

- [`Docs/MODULARIZATION_GUIDE.md`](Docs/MODULARIZATION_GUIDE.md) — Deep dive into the Unity Build architecture and the rules for file slicing.
- [`MouseShifter/Core/Globals/`](MouseShifter/Core/Globals/) — The central location for all global state.
- [`MouseShifter/gearlayouts.ini`](MouseShifter/gearlayouts.ini) — Example of the gear layout file format.

---

*Thank you for helping make MouseShifter better! 🏎️*
