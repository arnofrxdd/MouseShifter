# 🖱️ MouseShifter

**MouseShifter** is a Windows desktop application that transforms your mouse (or XInput/SDL controller) into a fully functional H-pattern or automatic (PRNDL) gear shifter for racing simulators. It outputs gear commands via keyboard bindings, vJoy virtual joystick buttons, or mouse buttons, and optionally injects DLLs into the target game process to block conflicting hardware input.

---

## 📋 Table of Contents

- [Features](#features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Gear Layouts](#gear-layouts)
- [Input Sources](#input-sources)
- [Output Modes](#output-modes)
- [DLL Injection (Game Integration)](#dll-injection-game-integration)
- [Configuration & Profiles](#configuration--profiles)
- [UI Overview](#ui-overview)
- [Hotkeys](#hotkeys)
- [Architecture](#architecture)
- [Contributing](#contributing)
- [License](#license)

---

## ✨ Features

### 🎮 Input Sources
- **Raw Mouse** — moves a virtual shifter knob via Win32 Raw Input API at the lowest latency possible
- **XInput Controller** — use the right or left analog stick of any Xbox-compatible controller as the shifter knob
- **SDL Gamepad** — supports PlayStation, Xbox, and generic controllers via SDL2, including DualSense / DualShock 4 rumble
- **DirectInput Pedals** — clutch, brake, or accelerator pedals can be bound as a knob toggle or reverse-lock trigger
- **Mouse Scroll Wheel** — optionally maps scroll wheel to the vJoy Rx (clutch) axis
- **Left Trigger (LT)** — optionally maps the controller's left trigger to clutch (Rx axis)

### 🏎️ Gear Layouts
- **H-Shifter** — 12-gear or 16-gear rail layouts with configurable scale and diagonal assist
- **PRNDL** — automatic transmission mode (Park, Reverse, Neutral, Drive, Low) with vertical-only snapping
- **Custom Gear Layouts** — load named layouts from `gearlayouts.ini` at runtime; switch between them in-app
- **Reverse Lock** — configurable reverse gear protection; unlock via keyboard key, mouse button, or pedal press

### 🕹️ Output Modes
- **Keyboard Bindings** — each gear position maps to a configurable virtual key code (VK)
- **vJoy Virtual Joystick** — output gear commands as vJoy button presses on any configured vJoy device ID
- **Mouse Buttons** — bind gears to mouse buttons (left, right, middle, X1, X2)
- **Neutral** — dedicated neutral zone that fires a separate key/button binding and supports glow animations

### 📡 DLL Injection (Game Integration)
- **RawMouseInput.dll** — injected into the selected game process to block the physical mouse from registering as input, while MouseShifter takes over
- **xInputBlocker.dll** — injected to block the gamepad's right stick from being seen by the game, enabling clean vJoy-only input
- **Auto‑Inject** — automatically injects on process detection and uninjects on process exit or knob disable
- **Assist Button Logic** — XInput blocking can toggle on/off via a configurable gamepad shoulder/button press

### 🖼️ UI & Visuals
- **Borderless / Transparent Mode** — full borderless overlay mode with configurable alpha, Windows 11 Acrylic blur, and layered window transparency
- **Dynamic Transparency** — alpha fades automatically based on mouse movement activity
- **GDI+ Double-Buffered Rendering** — all UI elements are drawn via GDI+ in a memory DC for flicker-free rendering
- **Knob Glow Animations** — gears and the Neutral position display configurable glow/hold animations on activation
- **Ghost Knob** — a secondary "assist" pointer that the user moves independently; when the assist button is released, the real knob snaps to the ghost position
- **Indicator Bars** — X-axis and Y-axis visual bars for mouse steering feedback
- **Keybind Panel, Settings Panel, Toggle Panel, Input Panel** — four independently toggleable UI panels

### ⚙️ System Features
- **Multi-Profile Configuration** — save and load multiple named profiles; each stores all settings independently
- **INI-Based Config** — all settings persist to a human-readable INI file via Windows `WritePrivateProfileString` / `GetPrivateProfileString`
- **File Backup** — automatic backup of configuration files before modification
- **Auto-Updater** — built-in update checker that downloads and extracts the latest release from a remote URL
- **Smart Redraws** — optional optimization that only redraws the regions of the window that changed (knob, bars, etc.)

---

## 💻 System Requirements

| Requirement | Details |
|---|---|
| **OS** | Windows 10 / 11 (64-bit) |
| **Build Toolchain** | Visual Studio 2019 or later (MSVC v142+) |
| **Runtime** | Visual C++ Redistributable 2019 |
| **vJoy** | vJoy 2.x driver (optional, for virtual joystick output) |
| **SDL2** | Bundled in `SDL2 LIBS/` (no separate install needed) |
| **DirectX** | Direct2D, GDI+, DirectInput8 (all standard Windows SDK) |
| **XInput** | Windows SDK XInput (standard) |

---

## 📦 Installation

### Pre-built Release
1. Download the latest release `.zip` from the [Releases](../../releases) page.
2. Extract to any folder.
3. (Optional) Install the **vJoy** driver if you want virtual joystick button output.
4. Run `MouseShifter.exe`.

### Build from Source

**Prerequisites:**
- Visual Studio 2019 or later with the **Desktop development with C++** workload
- Windows 10 SDK (10.0.x)

```
1. Clone or download this repository.
2. Open MouseShifter/MouseShifter.vcxproj in Visual Studio.
3. Set the build target to Release | x64.
4. Build → Build Solution  (Ctrl+Shift+B).
5. The output binary will be at MouseShifter/x64/Release/MouseShifter.exe (or similar).
```

> **Note:** The SDL2 static/dynamic library is already bundled under `SDL2 LIBS/`. No additional SDL2 installation is needed.

---

## 🚀 Quick Start

1. Launch `MouseShifter.exe`.
2. Open the **Settings Panel** (visible by default on the left side).
3. Choose your **Input Source**: Mouse, XInput, or Controller.
4. Set each gear's **keybind** in the **Keybind Panel** (right side).
5. If using a simulator:
   - Select the game process in the **Injection Panel**.
   - Enable **Mouse Block** and/or **XInput Block** as needed.
6. Move your mouse (or analog stick) to shift gears — the knob will snap into gear positions and fire the bound input.

---

## 🗺️ Gear Layouts

MouseShifter supports the following built-in layout types (configured via `layoutType`):

| ID | Layout Name | Description |
|---|---|---|
| 1 | Normal | Standard H-pattern, reverse on top |
| 2 | No Reverse | H-pattern without reverse rail |
| 3 | Reverse Bottom First | Reverse gear on bottom-left rail |
| 4 | Reverse Bottom Last | Reverse gear on bottom-right rail |
| 5 | 5-Gear Only | 5-speed H-pattern |
| 6 | 5-Gear Reverse First | 5-speed with reverse on first rail |
| 7 | 4-Gear Reverse Top | 4-speed with reverse on top |
| 8 | 4-Gear Reverse Bottom | 4-speed with reverse on bottom |
| 9 | 4-Gear Reverse Mixed | 4-speed mixed reverse |
| 10 | Reverse Top Last | Standard layout, reverse top-last |
| 11 | PRNDL | Automatic transmission (Park/Reverse/Neutral/Drive/Low) |

Custom layouts can be defined in **`gearlayouts.ini`** and selected from the in-app layout picker.

---

## 🎮 Input Sources

### Mouse (Raw Input)
- The application registers a Raw Input device listener via `RegisterRawInputDevices`.
- Mouse delta (`lLastX`, `lLastY`) is translated directly to knob movement.
- Sensitivity, diagonal assist, and snap thresholds are all configurable.

### XInput Controller
- Left or right analog stick (configurable).
- Full knob physics including rail tracking, snap-in/snap-out with hysteresis, and haptic rumble on gear snap.
- An "Assist Pointer" (Ghost Knob) mode lets you preview a target gear before committing.

### SDL / PlayStation Controller
- Detected and managed via `SDL_InitGameController`.
- Supports hot-plug via `SDL_CONTROLLERDEVICEADDED / REMOVED` events.
- Haptic rumble supported on DualSense and DualShock 4 via `SDL_HapticOpenFromJoystick`.
- PlayStation button layout mode toggle included.

### DirectInput Pedals
> ⚠️ **Currently not working.** DirectInput pedal support is present in the codebase but is non-functional at this time.
- Detected via DirectInput8 (`IDirectInput8`, `IDirectInputDevice8`).
- Clutch (Z), Brake (Y), and Accelerator (Rz) axes readable.
- Any axis can be bound as the knob toggle or reverse-unlock trigger.

---

## 📤 Output Modes

### Keyboard
Send a `VK_*` virtual key down/up via `SendInput` when a gear is engaged/released.

### vJoy Button
Call `SetBtn(true/false, deviceId, buttonId)` via the vJoy SDK to press/release a virtual joystick button.

### Mouse Button
Send `MOUSEEVENTF_*DOWN / *UP` flags via `SendInput` for left, right, or middle mouse buttons.

### Neutral
A dedicated Neutral zone fires a separate binding and shows a glow animation overlay.

---

## 💉 DLL Injection (Game Integration)

MouseShifter uses standard Win32 `CreateRemoteThread` + `LoadLibraryW` DLL injection to integrate with games on the fly.

| DLL | Purpose |
|---|---|
| `RawMouseInput.dll` | Blocks the physical mouse from being read by the game (stops double-input) |
| `xInputBlocker.dll` | Blocks the gamepad's right stick from being read by the game (stops double-input from vJoy) |

**How it works:**
1. The user selects a target process from the in-app process picker (only visible game processes are listed).
2. When mouse/XInput blocking is enabled, MouseShifter calls `InjectDLL()`, which writes the DLL path into the target process memory and creates a remote thread to call `LoadLibraryW`.
3. When blocking is disabled or the knob is toggled off, `UninjectDLL()` finds the module via `CreateToolhelp32Snapshot` and calls `FreeLibrary` via a remote thread.
4. An optional **Assist Button** on the gamepad can dynamically enable/disable XInput blocking mid-session.

> ⚠️ **Antivirus Note:** DLL injection is a technique also used by some malware. Some antivirus software may flag the injection DLLs. This is a false positive — the source code is fully open for inspection.

---

## ⚙️ Configuration & Profiles

All settings are stored in an INI file using standard Windows `WritePrivateProfileString` / `GetPrivateProfileString`.

### Profiles
- Multiple named profiles are supported.
- Create, rename, duplicate, and delete profiles from the **Settings Panel**.
- The active profile is loaded on startup and saved on exit.

### Gear Layouts (`gearlayouts.ini`)
Custom gear configurations can be defined in `gearlayouts.ini`:
```ini
count=2

[Layout0]
name=My Custom Layout
; ... gear key/value pairs

[Layout1]
name=Another Layout
; ...
```

### Key Settings (Persisted Per Profile)

| Setting | Description |
|---|---|
| `knobSensitivity` | Mouse-to-knob movement multiplier (0.05 – 2.0) |
| `gearRadius` | Visual and snap detection radius for each gear position |
| `snapSpeed` | How fast the knob lerps to a snap position (0.0 – 1.0) |
| `layoutType` | Which gear layout (0–11) is active |
| `layoutScale` | Overall size scale of the H-shifter display |
| `diagonalAssist` | How strongly the knob aligns to rails on diagonals |
| `reverseLockEnabled` | Whether reverse gear requires an unlock input |
| `useXInput` | Whether the XInput controller drives the knob |
| `is16GearSet` | Whether to use a 5-rail 16-gear layout |
| `vjoyDeviceId` | Which vJoy device ID to use for button output |
| Gear Keybinds | Per-gear: key type (keyboard/vJoy/mouse), key code |

---

## 🖥️ UI Overview

The window (`1250 × 830` by default) is divided into several visual panels:

| Panel | Location | Toggle |
|---|---|---|
| **H-Shifter Canvas** | Center | Always visible |
| **Keybind Panel** | Right side | Toggle in Settings |
| **Settings Panel** | Left side | Right-click or hotkey |
| **Toggle Panel** | Floating | Configurable |
| **Input Panel** | Floating | Configurable |

All panels are drawn via GDI+ with double-buffered rendering.

---

## ⌨️ Hotkeys

| Key | Action |
|---|---|
| `F9` | Disable / enable the knob (override via keyboard) |
| `Right Mouse Button` (held) | Unlock knob movement / temporary movement disable |
| **Configurable Toggle Key** | Activate knob movement (keyboard, pedal, or mouse button) |
| **Configurable Reverse Unlock** | Allow shifting into Reverse (keyboard, pedal, or mouse button) |
| **SDL Assist Button** | Toggle Ghost Knob / XInput block state (RB by default) |

---

## 🏗️ Architecture

MouseShifter uses a **Unity Build / Router-Fragment** architecture. All `.cpp` files are compiled as a single translation unit via `#include` chains. There are no independently compiled `.cpp` files.

```
MouseShifter.cpp  ← WinMain entry point
│
├── Core/
│   ├── Config.cpp               ← Router: loads Config fragments
│   │   └── Config/
│   │       ├── Config_Save.cpp
│   │       ├── Config_Load.cpp
│   │       ├── Config_Profiles_Mgr.cpp
│   │       ├── Config_INI.cpp
│   │       ├── Config_Processes.cpp
│   │       ├── Config_Injection.cpp
│   │       └── Config_UI.cpp
│   ├── ConfigManager.cpp        ← Router: settings persistence
│   ├── Globals/                 ← Global state, device state, UI state
│   └── System/
│       ├── Updater.cpp          ← Auto-update checker and downloader
│       └── FileBackup.cpp       ← Config file backup
│
├── Input/
│   ├── VJoySetup.cpp            ← Router: vJoy hardware init
│   │   └── VJoy/
│   │       ├── VJoy_Init.cpp
│   │       ├── VJoy_State.cpp
│   │       ├── VJoy_GearControl.cpp
│   │       ├── VJoy_RawInput.cpp
│   │       └── VJoy_ThreadState.cpp
│   ├── MouseInput.cpp           ← Raw Input device registration
│   └── Submodules/
│       ├── Input_Pedals.cpp     ← DirectInput pedal polling
│       ├── Input_RawMouse.cpp   ← Raw mouse processing
│       ├── Input_ReverseLock.cpp
│       ├── Input_XInputEngine.cpp
│       ├── XInput_Knob_Physics.cpp ← Rail/snap/ghost knob logic
│       ├── XInput_Knob_Transparency.cpp
│       ├── XInput_Rumble.cpp    ← SDL haptic rumble
│       └── XInput_vJoy.cpp
│
├── Windows/
│   ├── WindowProc.cpp           ← Master WndProc router
│   └── Handlers/
│       ├── WindowProc_Input.cpp
│       ├── WindowProc_MouseEvents.cpp
│       └── WindowProc_Timer.cpp
│   ├── Mechanics/               ← Mouse physics, keyboard handler
│   ├── MouseEvents/             ← LButton, MouseMove event handlers
│   └── Timer/                   ← Smart redraw, glow, transparency, polling
│
└── UI/
    ├── ShifterUI.cpp            ← Master paint router
    ├── Handlers/                ← Top-level UI handlers (Graphics, Transparency)
    ├── Borderless/              ← Borderless mode overlay drawing
    ├── Drawing/                 ← Gear, knob, rail drawing routines
    ├── Math/                    ← Layout computations, intersections
    └── Settings/                ← Settings panel UI fragments
```

**Key design decisions:**
- Single translation unit ensures maximum inlining and LTO without a complex build system.
- All global state is declared in `Core/Globals/` — no scattered `extern` declarations.
- Fragment files are registered as `<ClInclude>` in the `.vcxproj` so the IDE indexes them correctly.

---

## 🤝 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for full details on how to contribute.

---

## 📄 License

This project is available as open source. See the `LICENSE` file for details.
