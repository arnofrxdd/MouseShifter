# 🐧 MouseShifter — Linux Port Feasibility Analysis

> **Short answer:** The core gear logic is portable. The UI, windowing, and injection systems are not. This is a **heavy rewrite**, not a recompile.

---

## Why Raw Input Was Used (And What Linux Needs Instead)

On Windows, `RegisterRawInputDevices` + `WM_INPUT` receives mouse deltas at the HID driver level — **below** the game's own input capture. This is why MouseShifter can still receive mouse movement even when a game holds exclusive mouse focus via DirectInput or `SetCapture`.

On Linux, the equivalent is reading `/dev/input/eventX` directly via **`libevdev`**. This bypasses whatever the game does with `XGrabPointer` or SDL2 device acquisition. The good news: on Linux you **don't need DLL injection** for the mouse-blocking part — the kernel event node is a file you can read independently of what the game sees.

The remaining problem: the game still receives its own copy of the events. Solutions:
1. **`uinput` filter device** — intercept and re-emit filtered deltas (clean, complex)
2. **`LD_PRELOAD` hook** — hook `read()` inside the game process (fragile)
3. **In-game setting** — many Linux racing sims let you disable mouse input in their own config (often the simplest path)

---

## Dependency Map

### 🔴 Hard Blockers — Must Be Rewritten From Scratch

| System | Windows API | Linux Situation |
|---|---|---|
| **Window + message loop** | `WinMain`, `WndProc`, `WM_*` messages | No Win32 on Linux. Replace with SDL2 window + event loop. Entire `Windows/` domain must be rebuilt. |
| **GDI+ rendering** | `Gdiplus::Graphics`, `CreateDIBSection`, `BitBlt` | GDI+ doesn't exist. Replace with **Cairo**, **Skia**, or SDL2 renderer. Entire `UI/` domain (~40 files) needs a new backend. |
| **DLL injection** (`Config_Injection.cpp`) | `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread` + `LoadLibraryW` | Linux uses `ptrace` + `dlopen`. Restricted by `ptrace_scope` on most distros. May be partially unnecessary (see above). |
| **`Mouse_DLL.cpp` — MinHook on `GetRawInputData`** | MinHook API hooking `GetRawInputData` inside the game | `GetRawInputData` doesn't exist on Linux. Games use `evdev`/SDL. Needs `LD_PRELOAD` hook on `read()` instead — entirely different approach. |
| **`Controller_DLL.cpp` — MinHook on `XInputGetState`** | MinHook hooks `XInputGetState` / ordinal 100 | XInput is Windows-only. Linux games use `evdev`/SDL2. This DLL has no Linux equivalent. |
| **Win11 Acrylic / layered transparency** | `DwmSetWindowAttribute`, `SetLayeredWindowAttributes`, `UpdateLayeredWindow` | No standard equivalent. Compositor-specific (KDE/GNOME). Skip or implement per-compositor. |
| **Screen capture for brightness** (`Graphics_ScreenCapture.cpp`) | `GetDC(NULL)` + `BitBlt` from desktop DC | X11: `XGetImage`. Wayland: no stable screen capture API (requires portal). |

---

### 🟡 Replaceable — Medium Effort

| System | Windows API | Linux Replacement |
|---|---|---|
| **Raw mouse input** | `RegisterRawInputDevices`, `WM_INPUT`, `RAWINPUT` | `libevdev` on `/dev/input/eventX`, or SDL2 relative mouse mode |
| **XInput controller** | `XInput.h`, `XInputGetState` | **SDL2 gamepad API** — already bundled and cross-platform |
| **DirectInput pedals** | `IDirectInput8`, `IDirectInputDevice8`, `DIJOYSTATE2` | `libevdev` or SDL2 joystick API. **⚠️ Currently broken even on Windows** |
| **Virtual joystick output** | `vJoy` SDK (`SetBtn`, `SetAxis`) | Linux `uinput` kernel module + `libevdev` uinput API |
| **Keyboard/mouse output** | `SendInput`, `KEYBDINPUT`, `MOUSEEVENTF_*` | `uinput` device with `EV_KEY` events |
| **Process enumeration** | `CreateToolhelp32Snapshot`, `Process32First/Next` | Read `/proc/[pid]/status` + `/proc/[pid]/exe` |
| **Auto-updater** | `URLDownloadToFile` (urlmon) | `libcurl` or `wget` subprocess |
| **ATS/ETS2 config path** | `SHGetKnownFolderPath(FOLDERID_Documents)` | `$HOME/.local/share/` game paths |

---

### ✅ Already Portable — No Changes Needed

| System | Notes |
|---|---|
| All gear physics / snap / rail logic | Pure C++ math, zero Windows APIs |
| Layout computations (`UI/Math/`) | Pure geometry, fully portable |
| INI config read/write | Replace `GetPrivateProfileString` with `std::fstream` (trivial) |
| SDL2 gamepad + haptics | SDL2 is cross-platform and already bundled |
| Gear layout INI parser | Standard file I/O |

---

## Effort Estimate by Layer

| Layer | Effort |
|---|---|
| Core physics / gear math | ✅ Zero — already portable |
| SDL2 input (controller) | ✅ Zero — already cross-platform |
| INI config + ATS/ETS2 path | 🟡 Easy (1–2 days) |
| Process enumeration | 🟡 Easy (1 day) |
| Raw mouse input via `libevdev` | 🟡 Medium (3–5 days) |
| Virtual joystick output via `uinput` | 🟡 Medium (3–5 days) |
| Win32 message loop → SDL2 event loop | 🔴 Hard (2–4 weeks) |
| GDI+ rendering → Cairo/Skia (entire UI) | 🔴 Hard (4–8 weeks) |
| DLL injection → `LD_PRELOAD` / `uinput` | 🔴 Very hard (2–4 weeks, may be partially skippable) |
| Win11 transparency effects | 🔴 Hard / skip (1–2 weeks or drop) |

**Total realistic estimate: 3–4 months** for a single experienced developer. The result would share only the physics, math, and config core with the Windows version — the windowing and UI layers would be new code.

---

## Recommended Port Strategy

If a Linux port were to be attempted, the cleanest path would be:

1. **Extract the portable core** — gear physics, math, layout, config — into a platform-agnostic static library.
2. **Replace the window + event loop** with SDL2 (already a dependency).
3. **Replace GDI+ rendering** with SDL2_gfx + SDL2_ttf, or Cairo on an SDL2 surface.
4. **Replace vJoy** with a `uinput` backend behind the same `SetGearKey()` / `ReleaseGearKey()` interface.
5. **Replace Raw Input** with `libevdev` behind the same `ProcessRawInput()` interface.
6. **Drop or stub the DLL injection system** — rely on in-game settings to disable conflicting mouse input instead.
