# 📁 MouseShifter — Codebase Documentation

> **Every file in the project documented.** This is a living document — update it when you add a new file.
>
> MouseShifter uses a **Unity Build** architecture. All `.cpp` fragment files are `#include`d (not compiled independently). See `CONTRIBUTING.md` for the rules.

---

## Table of Contents

- [Root](#root)
- [Core Domain](#core-domain)
  - [Core/](#core)
  - [Core/Config/](#coreconfig)
  - [Core/Globals/](#coreglobals)
  - [Core/System/](#coresystem)
- [Input Domain](#input-domain)
  - [Input/](#input)
  - [Input/VJoy/](#inputvjoy)
  - [Input/Submodules/](#inputsubmodules)
- [Windows Domain](#windows-domain)
  - [Windows/](#windows)
  - [Windows/Handlers/](#windowshandlers)
  - [Windows/Mechanics/](#windowsmechanics)
  - [Windows/MouseEvents/](#windowsmouseevents)
  - [Windows/Timer/](#windowstimer)
- [UI Domain](#ui-domain)
  - [UI/](#ui)
  - [UI/Handlers/](#uihandlers)
  - [UI/Borderless/](#uiborderless)
  - [UI/Drawing/](#uidrawing)
  - [UI/Math/](#uimath)
  - [UI/Settings/](#uisettings)
- [vJoy SDK](#vjoy-sdk)
- [Resources](#resources)
- [Root Data Files](#root-data-files)

---

## Root

| File | Description |
|---|---|
| `MouseShifter.cpp` | **WinMain entry point.** Registers window class `HShifterWinClass`, creates the main `1250×830` popup window, initializes GDI+, calls `processAllFiles(true)` to edit ATS/ETS2 controls.ini, loads config and gear layouts, registers raw input, then runs the message loop. On exit: saves config, reverts game edits, releases GDI+. This is the **only real compilation unit** — all `.cpp` fragments are `#include`d from here via routers. |
| `MouseShifter.h` | Top-level project header. Includes Windows SDK, GDI+, SDL2, DirectInput, and forward-declares a few WinMain-level functions. |
| `MouseShifter.vcxproj` | Visual Studio 2019 project file. All fragment `.cpp` files are listed as `<ClInclude>` (never `<ClCompile>`), allowing IDE indexing without independent compilation. |
| `MouseShifter.vcxproj.filters` | IDE filter definitions mapping physical paths to virtual folder names shown in Solution Explorer. |
| `MouseShifter.vcxproj.user` | Per-developer debugging settings (launch dir, debugger type). Not committed in a real open-source project. |
| `packages.config` | NuGet package references (none active; retained for VS compatibility). |
| `framework.h` | Minimal Windows prelude — defines `WIN32_LEAN_AND_MEAN`, `NOMINMAX`, and `_WIN32_WINNT`. Included by `MouseShifter.h`. |
| `public.h` | Thin wrapper that pulls in `vjoyinterface.h` from the `vJoy/` folder. Required for vJoy API calls. |
| `targetver.h` | Windows SDK minimum version pin (`_WIN32_WINNT_WIN10`). |
| `gearlayouts.ini` | **User-editable gear layout file.** Defines named H-Shifter layouts (gear counts, positions, names) loaded at startup by `Config_INI.cpp`. |

---

## Core Domain

### `Core/`

| File | Description |
|---|---|
| `Core/Config.cpp` | **Router** for the entire config subsystem. `#include`s all seven `Core/Config/` fragment files in dependency order. Top-level entry points: `LoadConfig()` / `SaveConfig()`. |
| `Core/Config.h` | Header declaring the config subsystem: `LoadConfig()`, `SaveConfig()`, `InitializeProfiles()`, and input map types (`GearInput`, `InputType` enum, `InputToVJoy` map). |
| `Core/ConfigManager.cpp` | **Router** that `#include`s `Globals_Logic_Misc.cpp`, bridging the global state layer to the config system. Also pulls in the `LayoutComputations.cpp` chain via Misc. |

---

### `Core/Config/`

| File | Description |
|---|---|
| `Config_INI.cpp` | Parses `gearlayouts.ini` via `LoadGearLayoutsFromIni()`. Reads named `[LayoutN]` sections, populates `gearLayouts` and `gearLayoutNames` vectors. Handles BOM detection, whitespace trimming, and multi-pass section seeking. |
| `Config_Load.cpp` | Implements `LoadConfig()`. Reads every slider value, toggle state, gear keybind, layout type, sensitivity, and transparency setting from the active profile INI using `GetPrivateProfileString` / `GetPrivateProfileInt`. |
| `Config_Save.cpp` | Implements `SaveConfig()`. Mirror image of `Config_Load.cpp` — writes current state to the active profile INI using `WritePrivateProfileString`. |
| `Config_Profiles_Mgr.cpp` | Manages multi-profile lifecycle: `InitializeProfiles()` (default profile creation), `CreateProfile()`, `RenameProfile()`, `DeleteProfile()`, `SwitchProfile()`. Profiles stored as named subdirectory INI files. |
| `Config_Processes.cpp` | Implements `RefreshProcessList()` and `IsGameProcess()`. Enumerates running processes via `CreateToolhelp32Snapshot`. Filters out browsers (`chrome.exe`, `firefox.exe`), IDEs (`devenv.exe`), launchers (`Steam.exe`), and MouseShifter itself. Only visible, reasonably-sized windows are added to `g_processList`. |
| `Config_Injection.cpp` | Implements `InjectDLL()` and `UninjectDLL()` via `CreateRemoteThread` + `LoadLibraryW` / `FreeLibrary`. Implements `UpdateAutoInjection()`: per-tick logic that monitors `g_selectedProcessId`, auto-injects `RawMouseInput.dll` when mouse blocking is active, and manages `xInputBlocker.dll` lifecycle based on `g_xinputBlockEnabled` and assist button state. |
| `Config_UI.cpp` | Draws the process picker combo box, inject/uninject button state indicators, and tooltip logic in the Settings panel. Also manages `Shift+Click` vJoy button picker UI state. |

---

### `Core/Globals/`

All files here define **global variables** shared across the entire codebase. Because this is a unity build, each variable is defined exactly once.

| File | Description |
|---|---|
| `AppGlobals.h` | **Master global header.** Chains `Config.h` → `Globals_ConfigInput.h` → `Globals_UI.h` → `Globals_Devices.h` → `AppGlobals.cpp`. Single include from `MouseShifter.cpp` to pull in all shared state. |
| `AppGlobals.cpp` | Defines the final shared items (scroll panel variables, profile list, update tracking). Also `#include`s `Globals_Logic_DirectInput.cpp`, `Globals_Logic_ReverseLock.cpp`, `Globals_Logic_Toggles.cpp`, `Globals_Logic_Misc.cpp`. |
| `Globals_ConfigInput.h` | Defines all **input configuration globals**: `vjoyDeviceId`, `useXInput`, `useRightStick`, knob sensitivity min/max, gear position maps (`lowerGearPositions`, `highGearPositions`), `activeGear`, `lockedInGear`, `isBorderless`, reverse lock state, drag booleans for all sliders, and the `ToggleType` enum (KEYBOARD / PEDAL_CLUTCH / PEDAL_BRAKE / PEDAL_ACCEL / MOUSE_LEFT / RIGHT / MIDDLE / BUTTON4 / BUTTON5). Also `#include`s `Transparency.h`, `public.h`, `vjoyinterface.h`, `XInput.h`. |
| `Globals_UI.h` | Defines all **UI state globals**: `KeybindAnimation` struct and `keybindAnimations` map, glow fade constants (`GLOW_FADE_IN_TIME`, `GLOW_FADE_OUT_TIME`, `MAX_GLOW_ALPHA`), panel visibility booleans, knob physics constants (`gearRadius`, `diagonalAssist`, `snapSpeed`, `layoutScale`), rail geometry (`railX[]`, `railOffsets12[]`, `railOffsets16[]`, `centerX/Y`, `topY/bottomY`), D2D1 factory pointer, `hwndMain`, `knobPos`, `ghostKnobPos`, and the `RailType` enum. |
| `Globals_Devices.h` | Defines all **device and hardware globals**: vJoy axis values (`joyX/Y/RX/RY/Z`), SDL gamepad list (`g_gamepads`), `Gamepad` struct, `InitGamepads()` / `RefreshGamepads()` (SDL controller hot-plug), XInput sensitivity and smoothing values, scroll clutch state variables, DirectInput globals (`g_pDI`, `g_pJoystick`, `g_diState`), DLL injection state (`g_selectedProcessId`, `g_autoInjectEnabled`, inject/uninject button RECTs), and controller sensitivity slider state. |
| `Globals_Logic_DirectInput.cpp` | Defines `InitDirectInput()` and `PollDirectInput()`. Opens DirectInput8, enumerates joystick devices to find pedal axes, reads full `DIJOYSTATE2` on each poll so clutch/brake/accelerator values are available for toggle logic. |
| `Globals_Logic_ReverseLock.cpp` | Implements the reverse-lock boundary system: `IsReverseUnlockActive_Fix()`, `ClampMovementForReverseLock_Fix()`, `ClampHorizontalMovementForReverseLock_Fix()`, `EnforceReverseLockBoundary()`. Prevents the knob from entering the reverse gear slot unless the configured unlock input is actively held. |
| `Globals_Logic_Toggles.cpp` | Implements `IsKnobToggleActive()` and `IsReverseUnlockActive()`. Both switch on `ToggleType` enum and return whether the user holds the configured input (keyboard key, pedal axis spike, or mouse button). Also defines toggle panel UI state RECTs (`togglePanelRect`, `inputPanelRectUnified`) and booleans (`vJoyMouseEnabled`, `showYBar`, `showXBar`, `assistButton`). |
| `Globals_Logic_Misc.cpp` | The bootstrap glue fragment. Implements `processAllFiles()` (reads ATS/ETS2 Documents INI files and injects/reverts mouse steering config lines). `#include`s `FileBackup.cpp`, `VJoySetup.cpp`, `MouseInput.cpp`, and `LayoutComputations.cpp` to pull them into the unity build chain. |

---

### `Core/System/`

| File | Description |
|---|---|
| `Updater.h` | Header declaring the auto-updater API: `currentVersion`, `updateAvailable`, `latestVersion`, `CheckForUpdates()`, `PerformUpdate()`, `DownloadFile()`, `ExtractZipNew()`, `IsInternetAvailable()`. |
| `Updater.cpp` | Implements the auto-update system. `CheckForUpdates()` uses `URLDownloadToFile` (urlmon) to fetch a version manifest, parses it, and sets `updateAvailable`. `PerformUpdate()` downloads the release ZIP, extracts it to the app directory, and triggers a restart. Runs on a background thread to avoid blocking the UI. |
| `FileBackup.cpp` | Implements `BackupFile()` and `RestoreBackup()`. Before MouseShifter edits any game config file (ATS/ETS2 controls.ini), it copies the original to a `.bak` file. On shutdown, `RestoreBackup()` reverts the edits. |

---

## Input Domain

### `Input/`

| File | Description |
|---|---|
| `Input/VJoySetup.cpp` | **Router** for the vJoy subsystem. `#include`s all five `Input/VJoy/` fragments in order. Top-level entry points: `SetGearKey()`, `ReleaseGearKey()`, `GetVJoyButtonCount()`, `InitVJoy()`. |
| `Input/MouseInput.cpp` | Implements `InitRawInput()`. Calls `RegisterRawInputDevices()` to subscribe to `HID_USAGE_GENERIC_MOUSE` events for all detected devices, enabling per-device delta tracking that bypasses Windows mouse acceleration. |

---

### `Input/VJoy/`

| File | Description |
|---|---|
| `VJoy_Init.cpp` | Initializes the vJoy virtual device: calls `vJoyEnabled()`, `AcquireVJD()`, retrieves axis ranges (`GetVJDAxisMax/Min`), resets state to center, and stores `axisMin/axisMax` for later normalization. |
| `VJoy_State.cpp` | Defines `gearInputMap` (maps gear name strings like `"1"`, `"R"`, `"N"` → `GearInput` structs containing `InputType` and code), `inputMap` (key → vJoy button secondary mappings), and the `GearInput` / `InputType` enum (KEYBOARD / MOUSE / VJOY_BUTTON). |
| `VJoy_GearControl.cpp` | Implements `SetGearKey(gear)` and `ReleaseGearKey()`. Looks up the gear in `gearInputMap` and fires the appropriate `SendInput()` (keyboard), `SetBtn()` (vJoy button), or mouse button event. Also triggers glow animation on the activated keybind row in the side panel. |
| `VJoy_RawInput.cpp` | Implements `UpdateVJoyFromMouse()`. Maps raw mouse `dx` delta values to vJoy X-axis (steering) output for mouse-steering mode, applying `steeringSensitivity` and `maxSteeringDegrees` clamping. |
| `VJoy_ThreadState.cpp` | Manages thread-safe toggle state for the vJoy mouse output thread. Defines `StartVJoyThread()`, `StopVJoyThread()`, and the mutex-guarded flag `vJoyMouseEnabled`. Ensures the vJoy axis update loop can be started and stopped cleanly. |

---

### `Input/Submodules/`

| File | Description |
|---|---|
| `Input_Pedals.cpp` | Implements `PollPedalBinding()`. On each timer tick, if the user is in `togglePedalBeingSet` mode, reads `g_diState` to detect which pedal axis spiked and binds it. Also checks `UpdateAutoInjection()` state for pedal-driven toggle conditions. |
| `Input_RawMouse.cpp` | Implements `ProcessRawInput(hwnd, lParam)`. Handles `WM_INPUT` messages: reads the `RAWINPUT` struct, identifies the source device, applies sensitivity scaling and diagonal assist, calls `ClampMovementForReverseLock_Fix()`, then updates `knobPos`. Calls `UpdateVJoyFromMouse()` for steering axis output. |
| `Input_ReverseLock.cpp` | "Fix" wrapper for reverse lock. Wraps `IsReverseUnlockActive()` with additional logic that tracks `g_reverseAuthorizedOnRail` — prevents entry into the reverse gear slot unless the unlock input is actively held at entry. |
| `Input_XInputEngine.cpp` | **Router** for the XInput physics sub-system. `#include`s all five `XInput_*.cpp` fragments in sequence: Rumble → Knob_Prefix → Knob_Physics → Knob_Transparency → vJoy. Exports `UpdateKnobFromXInput()` and `UpdateVJoyFromXInput()`. |
| `XInput_Rumble.cpp` | Implements `SafeRumble(leftMotor, rightMotor, durationMs)`. Fires SDL haptic/rumble on the currently selected gamepad (`g_gamepads[g_selectedGamepadIndex]`). Used by the physics fragment on gear snap events. |
| `XInput_Knob_Prefix.cpp` | Entry wrapper fragment. Sets up `dx`/`dy` from XInput stick deltas, applies `xInputSensitivity`, and selects `targetKnob` pointer (either `&knobPos` or `&ghostKnobPos` based on `useAssistPointer`). Variables consumed by the subsequent physics fragment. |
| `XInput_Knob_Physics.cpp` | **Core knob physics fragment.** Handles snap detection (Euclidean distance for H-Shifter, Y-axis-only for PRNDL), rail transitions (`HORIZONTAL` ↔ `VERTICAL`), lerp-based ghost→real knob "commit" animation, and hysteresis snap-in/snap-out with `realKnobSnapped` / `ghostKnobSnapped` guards. Calls `SafeRumble()` on snap. |
| `XInput_Knob_Transparency.cpp` | In XInput mode: monitors knob movement and calls `SetLayeredWindowAttributes` to manage window alpha — the XInput equivalent of the mouse-driven dynamic transparency system in `Timer_DynamicTransparency.cpp`. |
| `XInput_vJoy.cpp` | Maps XInput stick axis values (right or left X axis based on `useRightStick`) to the vJoy X axis in real time for mouse-steering axis output, independently of gear snap/lock logic. |

---

## Windows Domain

### `Windows/`

| File | Description |
|---|---|
| `Windows/WindowProc.cpp` | **Master `WndProc` function.** Handles `WM_CREATE` (D2D factory init, SDL gamepad init, close/max title buttons), `WM_CLOSE` (save + shutdown), `WM_CHAR`/`WM_KEYDOWN`/`WM_SYSKEYDOWN` (profile-name text entry and keybind capture), `WM_CAPTURECHANGED` (drag reset), `WM_SIZE` (layout recompute), `WM_PAINT` (calls `DrawShifterGDIPlus()`), `WM_NCHITTEST` (custom drag area for borderless), and `WM_MOUSEWHEEL` (settings scroll). Sub-routers for `WM_INPUT`, `WM_LBUTTONDOWN/UP/MOVE`, and `WM_TIMER` are `#include`d from `Windows/Handlers/`. |

---

### `Windows/Handlers/`

| File | Description |
|---|---|
| `WindowProc_Input.cpp` | Sub-router for `WM_INPUT`. `#include`s the five `Windows/Mechanics/` fragments in order: Prefix → Keyboard → MouseCore → MouseBinds → MousePhysics. |
| `WindowProc_MouseEvents.cpp` | Sub-router for `WM_LBUTTONDOWN`, `WM_MOUSEMOVE`, and `WM_LBUTTONUP`. `#include`s the three `Windows/MouseEvents/` routers. |
| `WindowProc_Timer.cpp` | Sub-router for `WM_TIMER`. `#include`s the five `Windows/Timer/` fragments in execution order: InputPolling → GlowAnimations → StateTracking → SmartRedraw → DynamicTransparency. |

---

### `Windows/Mechanics/`

| File | Description |
|---|---|
| `Input_Prefix.cpp` | Prefix invoked at the top of every `WM_INPUT` message. Allocates the `RAWINPUT` buffer via `GetRawInputData`, extracts `raw->data.mouse`, and sets up shared variables (`dx`, `dy`, device handle) consumed by subsequent fragments. |
| `Input_Keyboard.cpp` | Handles keyboard sub-events within `WM_INPUT`: specifically `WM_KEYDOWN` cases for cursor navigation in the new-profile text field (arrow keys, Home, End, Backspace, Escape). |
| `Input_MouseCore.cpp` | Core raw mouse dispatcher. Checks if the incoming device matches the selected steering device. For vJoy mouse-steering mode, calls `UpdateVJoyFromMouse()`. For shifter mode, calls `ProcessRawInput()` to update `knobPos`. |
| `Input_MouseBinds.cpp` | Handles raw mouse button events (`RI_MOUSE_LEFT_BUTTON_DOWN`, etc.) to detect when the user clicks a pedal-binding or assist-button capture row in the Toggle panel. |
| `Input_MousePhysics.cpp` | Main H-Shifter/PRNDL physics router for mouse input. Applies `knobSensitivity`, diagonal assist, and rail tracking. `#include`s `Input_Mouse_PRNDL.cpp` and `Input_Mouse_HShifter.cpp` conditionally based on `layoutType`. |
| `Input_Mouse_PRNDL.cpp` | PRNDL-specific mouse movement: vertical-only snapping with strong horizontal centering pull. Snaps purely by Y-axis proximity distance. No horizontal rail switching. |
| `Input_Mouse_HShifter.cpp` | H-Shifter intersection logic: checks if knob is within a diamond intersection zone and enables switching between horizontal (neutral rail) and vertical (gear) rail modes using `IsInsideIntersection()`. |

---

### `Windows/MouseEvents/`

| File | Description |
|---|---|
| `MouseEvents_LButtonDown.cpp` | **Router** for `WM_LBUTTONDOWN`. `#include`s five sub-fragments: Dropdowns → Profiles → Panels → Settings → Devices. |
| `LButtonDown_Devices.cpp` | Handles left-click on device-selection UI: gamepad combo dropdown, vJoy device picker, mouse/steering device binder button, and injection process picker. |
| `LButtonDown_Dropdowns.cpp` | Handles left-click on the gear-layout type dropdown and the H-Shifter layout dropdown. Opens/closes dropdowns, applies the selected `layoutType`, calls `ComputeLayout()`. |
| `LButtonDown_Panels.cpp` | Handles left-click on panel toggle buttons (Settings, Keybind, Input, Toggle), close/minimize title bar buttons, and the drag-area grab handle in borderless mode. |
| `LButtonDown_Profiles.cpp` | Handles left-click on profile management UI rows: create, rename, delete, and switch active profile buttons. Triggers `CreateProfile()` / `SwitchProfile()` / etc. |
| `LButtonDown_Settings.cpp` | Handles left-click on the registry-based settings panel. Manages section collapsing, toggle flipping, and initiates slider dragging by setting `g_draggingElement`. Also includes specialized logic for specific settings like "Gear Radius" or "Refresh Process List". |
| `MouseEvents_MouseMove.cpp` | Handles `WM_MOUSEMOVE` for the entire application. **Crucial:** Implements the registry-based slider dragging logic by updating values based on mouse delta while `g_draggingElement` is active. Also updates hover states for sidebar panels and manages tooltip timing logic. |
| `MouseEvents_LButtonUp.cpp` | Handles `WM_LBUTTONUP` to terminate all drag operations (resets `g_draggingElement` and all legacy `dragging*` flags). Calls `SaveConfig()` after any interactive drag ends. |

---

### `Windows/Timer/`

| File | Description |
|---|---|
| `Timer_InputPolling.cpp` | **Primary per-tick update fragment** called on every `WM_TIMER` event. Calls `UpdateKnobMovement()`, `UpdateAutoInjection()`, `UpdateSmoothScroll()`, `UpdateKnobFromXInput()`, `UpdateVJoyFromXInput()`, `PollDirectInput()`, `PollPedalBinding()`. Handles F9 knob-disable state and tooltip timer (`KillTimer` for tooltip display/hide). |
| `Timer_GlowAnimations.cpp` | Advances all active `KeybindAnimation` instances each tick. Held key = immediate `MAX_GLOW_ALPHA`. Released = fade-in over `GLOW_FADE_IN_TIME`, then fade-out over `GLOW_FADE_OUT_TIME`. Cleans up completed animations (glowAlpha ≤ 0). Triggers `InvalidateRect` only if any alpha value changed. |
| `Timer_StateTracking.cpp` | Tracks state changes requiring a redraw: knob enable/disable flicker, `knobPos` change, `activeGear` change, clutch indicator change. Triggers targeted `InvalidateRect` only for the specific changed region (knob RECT, gear indicator RECT) to minimize CPU usage. |
| `Timer_SmartRedraw.cpp` | **Router** for smart redraw logic. `#include`s `Timer_Redraw_Knob.cpp` and `Timer_Redraw_Bars.cpp`. Decides whether a full, knob-only, or bar-only redraw is needed. If `disableSmartRedraws` is true, always performs a full `InvalidateRect(NULL)`. |
| `Timer_Redraw_Knob.cpp` | Per-tick knob redraw geometry. Uses `CalculateKnobRedrawArea()` and `CalculateGhostKnobRedrawArea()` (from `Math_Knobs.cpp`) to compute minimal RECTs covering current + previous knob positions. Calls `InvalidateRect` only for those regions. |
| `Timer_Redraw_Bars.cpp` | Per-tick bar redraw geometry. Uses `CalculateXBarCompleteRedrawArea()` and `CalculateYBarCompleteRedrawArea()` (from `Math_XBar/YBar.cpp`) to invalidate only the bar regions when telemetry values changed beyond a threshold. |
| `Timer_DynamicTransparency.cpp` | Dynamic transparency system (borderless mode only). Every ~1500ms, calls `GetHShifterBackgroundBrightnessDebug()` to sample background luminance under the H-Shifter canvas, computes a `brightnessFactor`, and updates `dynamicMinAlpha` using a 3-sample history with hysteresis. When knob moves or toggle is held, snaps to `maxAlpha`. When idle, fades to `dynamicMinAlpha` via exponential decay with `SetLayeredWindowAttributes`. Also drives smooth scrolling for settings and right panels, and triggers the one-shot update-button redraw. |

---

## UI Domain

### `UI/`

| File | Description |
|---|---|
| `UI/ShifterUI.cpp` | **Master paint function `DrawShifterGDIPlus(hwnd, hdc)`**. Creates a 32-bit off-screen DIBSection, initializes GDI+ `Graphics` on it, clears to dark background with optional Win11 Acrylic blur. `#include`s `ShifterUI_Drawing`, `ShifterUI_SidePanels`, and `ShifterUI_Settings`. Finally `BitBlt`s the completed memory DC to the real `HDC` for flicker-free double-buffered rendering. |

---

### `UI/Handlers/`

| File | Description |
|---|---|
| `ShifterUI_Drawing.cpp` | **Router** for all canvas drawing. `#include`s the seven `UI/Drawing/` fragments back-to-front: HotkeyOverlay → HShifterRails → Gears → ReverseGear → GhostKnob → MainKnob → IndicatorBars. Painter's algorithm ensures correct layering. |
| `ShifterUI_SidePanels.cpp` | Draws the three modernized right-side panels using a card-based layout: **Keybind Panel** (gear → key rows with pulse-glow animations and vJoy button picker), **Input Panel** (secondary mappings), and **Toggle Panel** (Knob activation, Assist button, Reverse unlock). Features smooth scrolling and premium teal accents. |
| `ShifterUI_Settings.cpp` | **Router** for the Settings Panel. High-level structure: `#include`s the registry-based loop parts (`Settings_VariablesAndTitles`, `Settings_LayoutsAndProfiles`, `Settings_Registry_Continue`, `Settings_Game_Selector`, `Settings_Registry_Final`, `Settings_VJoyAndKeybinds`). This panel uses the unified `g_settingsRegistry` for most controls. |
| `ShifterUI_Overlays.cpp` | Draws floating overlays: the borderless-mode grab handle indicator, the update-available button (appears after `CheckForUpdates()` signals `updateAvailable`), and modal dialogs such as the vJoy button picker. |
| `Graphics_StaticElements.cpp` | Draws non-interactive decorative elements: window border shadow, panel dividers, and title bar area (close button ✕ glyph, maximize button glyph). |
| `Graphics_DrawBorderless.cpp` | **Router** for borderless overlay rendering. `#include`s the five `UI/Borderless/` fragments in sequence: Prefix → GhostKnob → MainKnob → IndicatorBars → (Math used internally). Calls `UpdateLayeredWindow` with the composed DIBSection for per-pixel alpha transparency. |
| `Graphics_ScreenCapture.cpp` | Implements `GetHShifterBackgroundBrightnessDebug()`. Captures the screen region under the H-Shifter canvas via `BitBlt` from the desktop DC, then computes the average luminance of the captured pixels. Used by `Timer_DynamicTransparency.cpp`. |
| `Graphics_RedrawMath.cpp` | **Router** for all redraw area calculation. `#include`s the four `UI/Math/` redraw fragments: Math_Knobs → Math_Gears → Math_XBar → Math_YBar. Also pulls in `LayoutComputations.cpp` chain via `Globals_Logic_Misc`. |
| `Transparency.cpp` | Implements `EnableWin11Blur(hwnd)` (sets `DWMWA_SYSTEMBACKDROP_TYPE` = Acrylic for Win11), `IsTransparencyEnabled()` (checks system transparency setting via registry), and the `SetLayeredWindowAttributes` wrapper for windowed transparency mode. |
| `Transparency.h` | Header declaring the transparency API: `EnableWin11Blur()`, `IsTransparencyEnabled()`, `currentAlpha`, `minAlpha`, `maxAlpha`, `isTransparent`, `dynamicTransparencyEnabled`, `alphaDecay`, `transparencyFadeDelay`. |

---

### `UI/Borderless/`

These five fragments are `#include`d by `Graphics_DrawBorderless.cpp` and execute within the borderless `DrawBorderless(HDC, width, height)` function scope.

| File | Description |
|---|---|
| `Borderless_Prefix.cpp` | Opens the `DrawBorderless()` function, creates a `Gdiplus::Graphics` object on the provided HDC with `SmoothingModeHighQuality`, then calls `DrawStaticShifterElements()` and `DrawStaticGearElements()` to render rails and gear circles that also appear in windowed mode. |
| `Borderless_GhostKnob.cpp` | Draws the ghost/assist pointer knob in borderless mode: a 70%-radius semi-transparent blue circle at `ghostKnobPos`. When `ghostSnappedGear` is non-empty, draws the snapped gear label centered in the ghost knob. Only rendered when `useAssistPointer` is true. |
| `Borderless_MainKnob.cpp` | Draws the main shifter knob in borderless mode. Supports two modes: **Realistic Knob** (inverted teardrop GDI+ bezier path with a top rounded cap, dark grey fill, light grey circular outline, and a scaled-down mini H-Shifter grid drawn on the top face with gear numbers) and **Basic Circle Knob** (simple `FillEllipse` with the active gear number centered, supporting low/high gear label stacking). Color reflects: gear active (light grey), disabled (red), disabled+clutch (purple), clutch engaged (red gradient), button flash (yellow). |
| `Borderless_IndicatorBars.cpp` | Draws the X-axis steering bar (horizontal, below the canvas), Y-axis telemetry bar (vertical, right of last rail, color shifts green→red based on axis value), and Clutch engagement bar (vertical, further right, blue gradient). Each bar shows a dynamic fill line and a center marker. Only drawn when their respective `showXBar`, `showYBar`, `showClutchIndicator` flags are enabled. |
| `Borderless_Math.cpp` | Implements `CalculateHShifterBoundaries()`. Returns a `RECT` representing the bounding box of the entire H-Shifter canvas (from the leftmost rail minus padding to the rightmost rail plus padding, topY−25 to bottomY+25). Used by `Graphics_ScreenCapture.cpp` for the brightness sampling region. |

---

### `UI/Drawing/`

These seven fragments are `#include`d by `ShifterUI_Drawing.cpp` and execute within the windowed GDI+ `Graphics` context.

| File | Description |
|---|---|
| `Drawing_HotkeyOverlay.cpp` | Draws a small text overlay at the top of the canvas showing active hotkey states: F9 knob-disable state indicator, the bound toggle key name, and the reverse unlock key name. |
| `Drawing_HShifterRails.cpp` | Draws the H-Shifter rail grid: the horizontal neutral rail and the vertical T-shaped rails connecting gear positions. Supports 3–5 rails depending on `layoutType` and `is16GearSet`. Applies `railOffsets12[]` or `railOffsets16[]` scaled by `layoutScale`. |
| `Drawing_Gears.cpp` | Draws gear position circles for all entries in `lowerGearPositions`. Colors: **active** = `#00FF88` bright green, **snapped** = lighter grey, **default** = dark grey. Triggers `KeybindAnimation` glow pulses on snap events. Draws gear number labels using Segoe UI. |
| `Drawing_ReverseGear.cpp` | Draws the Reverse (`R`) gear circle with distinct visual treatment (darker fill + red accent). Applies the reverse lock overlay (grey-out tint or lock icon) when `reverseLockEnabled` is true and the unlock input is not currently held. |
| `Drawing_GhostKnob.cpp` | Draws the ghost/assist pointer knob in windowed mode as a semi-transparent smaller circle at `ghostKnobPos`. Color: blue `(150, 150, 255)` when unsnapped, brighter blue `(200, 200, 255)` when snapped. Draws the snapped gear label when `ghostSnappedGear` is non-empty. |
| `Drawing_MainKnob.cpp` | Draws the main shifter knob circle at `knobPos` with `knobRadius`. Fills with flash-faded color (`knobFlash` → warm yellow blend on toggle press). Red fill + border when knob movement is disabled. Draws the active gear number centered inside. Also handles high-gear label stacking (low gear above, high gear below center) when `hideHighGears` is off. |
| `Drawing_IndicatorBars.cpp` | Draws the X-axis (horizontal steering) bar below the canvas when `showXBar` is true, and the Y-axis (clutch/brake) vertical bar to the right of the last rail when `showYBar` is true. Normalizes `joyX`/`joyY` axis values to pixel positions and draws fill bar + label. |

---

### `UI/Math/`

| File | Description |
|---|---|
| `LayoutComputations.cpp` | **Router** for all layout math. `#include`s `Layout_ComputeMain.cpp`, `Layout_Intersections.cpp`, and `Layout_Labels.cpp`. Top-level entry: `ComputeLayout(hwnd)`. |
| `Layout_ComputeMain.cpp` | Implements `ComputeLayout(HWND hwnd)`. Given the current window size, computes: `centerX/Y`, `railX[]` positions (applying `layoutScale` and centering offsets), `topY`/`bottomY` (±60px for H-Shifter, ±140px for PRNDL), `knobMinX/MaxX/Y`. Then populates `lowerGearPositions` and `highGearPositions` maps for all **11 layout types**: Layout 1 (Normal, R top-left), Layout 2 (No Reverse), Layout 3 (R bottom-first), Layout 4 (R bottom-last), Layout 5 (5-gear R-bottom-last-rail), Layout 6 (5-gear R-top-first), Layout 7 (4-gear R-top-first), Layout 8 (4-gear R-bottom-first), Layout 9 (4-gear R mixed R/1 on first rail), Layout 10 (R top-last), Layout 11 (PRNDL single-rail with 5 evenly spaced positions). Sets default `knobPos` to the center rail at `centerY`. |
| `Layout_Intersections.cpp` | Defines the `Intersection` struct and `intersections` vector. Implements `ComputeIntersections()` (places one intersection diamond at each `railX[i].y = centerY`, scaled by `diagonalAssist`) and `IsInsideIntersection(x, y)` (asymmetric ellipse math: larger radius toward gear sides, small buffer toward empty directions). Used by `Input_Mouse_HShifter.cpp` for rail-switching detection. |
| `Layout_Labels.cpp` | Defines the `gearLabelOverride` map (default gear label strings), the built-in `gearLayouts` vector (3 presets: **Default** `1-16/R`, **Alphabet** `A-P/R`, **Roman Numerals** `I-XVI/R`), `gearLayoutNames`, and `gearLayoutButtonRect` / `gearLayoutDropdownOpen` UI state. |
| `Math_Knobs.cpp` | Implements `CalculateKnobRedrawArea()` (union of current + previous `knobPos` RECT padded by `knobRadius + 10`), `CalculateGhostKnobRedrawArea()` (same for `ghostKnobPos`), `CalculateClutchKnobRedrawArea()`, `ShouldRedrawClutchKnob()` (checks if `joyRx` clutch position changed > 1%), and `CalculateKnobMovementIntensity()` (Euclidean displacement magnitude). Tracks `lastRedrawKnobPos`/`lastRedrawGhostKnobPos` for delta computation. |
| `Math_Gears.cpp` | Implements `CalculateSingleGearRedrawArea(gearName)` (RECT padded by `gearRadius + 8` around a gear's position) and `CalculateAllGearsRedrawArea()` (union RECT over all lower + high gear positions). Tracks `lastActiveGearState` and `lastGearStates` map to detect which specific gear circles changed. |
| `Math_XBar.cpp` | Implements `CalculateXBarRedrawArea()` (static X-bar RECT padded for indicator circle), `CalculateXBarIndicatorRedrawArea()` (current indicator position RECT), and `CalculateXBarCompleteRedrawArea()` (union of current + previous bar + indicator RECTs, updated at 30 FPS via `BAR_REDRAW_INTERVAL`). Also defines all X-bar and clutch-bar per-frame position tracking statics. |
| `Math_YBar.cpp` | Implements `CalculateYBarRedrawArea()` (static Y-bar RECT), `CalculateYBarFillRedrawArea()` (current fill height RECT, normalized from `joyY`), and `CalculateYBarCompleteRedrawArea()` (union of current + previous Y-bar + fill RECTs, updated at 30 FPS). Tracks `lastRedrawYBarX` and `lastRedrawYBarFillHeight`. |
| `Math_ClutchBar.cpp` | Implements `CalculateClutchBarRedrawArea()` (position RECT for the scroll-clutch Rx axis bar, placed right of Y-bar), `CalculateClutchBarFillRedrawArea()`, and `CalculateClutchBarCompleteRedrawArea()` (union of current + previous clutch bar RECTs, updated at 30 FPS). |

---

### `UI/Settings/`

These fragments are `#include`d by `ShifterUI_Settings.cpp` and draw into the Settings Panel GDI+ context.

| File | Description |
|---|---|
| `Settings_VariablesAndTitles.cpp` | Initializes the `g_settingsRegistry` if empty. Defines labels, tooltips, value pointers, and ranges for all core settings (Physics, Toggles, Transparency, Steering, Indicators). Also draws the "Settings" title and manages smooth scrolling offsets. |
| `Settings_Registry.cpp` / `.h` | Defines the `SettingElement` structure and the global `g_settingsRegistry` vector. This is the source of truth for all automated settings UI generation. |
| `Settings_Registry_LoopBody.cpp` | **Core Rendering Engine** for settings. Iterates over the registry and draws modern card-based UI for headings, high-quality GDI+ sliders (with glow and reset icons), and pill-style toggle switches. Automatically calculates hitboxes (`element.rect`) for interaction. |
| `Settings_LayoutsAndProfiles.cpp` | Custom UI fragment for profile management and layout selection. Not part of the generic registry as it requires complex custom interactions (text entry, renaming, multiple lists). |
| `Settings_Game_Selector.cpp` | Specialized widget for the DLL injector's target process selection. Includes the process picker modal logic and high-performance list rendering. |
| `Settings_VJoyAndKeybinds.cpp` | UI for vJoy device selection and advanced keybinding modes. |
| `Legacy_Backup/` | Contains the old `Settings_Sliders.cpp`, `Settings_Toggles.cpp`, and `Settings_Transparencies.cpp` that were used before the registry system was implemented. |

---

## vJoy SDK

| File | Description |
|---|---|
| `vJoy/vjoyinterface.h` | vJoy SDK public header. Declares `vJoyEnabled()`, `AcquireVJD()`, `GetVJDAxisMax/Min()`, `SetAxis()`, `SetBtn()`, `RelinquishVJD()`, and all related types. Wrapped by `public.h`. |
| `vJoy/vJoyInterface.dll` | vJoy runtime DLL. Must be present in the app's working directory for vJoy axis/button calls to work. |
| `vJoy/vJoyInterface.lib` | Import library for `vJoyInterface.dll`. Linked by the MSVC linker via the project's `AdditionalDependencies`. |
| `vJoy/vJoy_Setup.exe` | Bundled vJoy driver installer. Not used at runtime; included for convenience when setting up the development environment. |

---

## Resources

| File | Description |
|---|---|
| `Resources/MouseShifter.rc` | Windows resource script. Embeds the application icon into the executable and defines the `VERSIONINFO` resource. |
| `Resources/Resource.h` | Auto-generated resource ID constants (e.g., `IDI_MOUSESHIFTER = 107`). |
| `Resources/MouseShifter.ico` | Main application icon (256×256 ICO with multiple embedded sizes). |
| `Resources/small.ico` | Small taskbar icon (16×16). |

---

## Root Data Files

| File | Description |
|---|---|
| `gearlayouts.ini` | User-editable custom gear layout definitions. Loaded at startup by `Config_INI.cpp`. Format: `count=N` followed by `[LayoutN]` sections with `gearName=keyCode` pairs. |
| `profiles/profile_1.ini` | Default user profile INI. Stores all user settings (keybinds, sensitivities, toggles, layout type, transparency etc.) in standard INI format. Additional profiles are created alongside this file by `Config_Profiles_Mgr.cpp`. |
| `.gitignore` | Git ignore rules. Excludes `*.user`, `Debug/`, `Release/`, `x64/`, `*.ncb`, `*.suo`, `packages/`, build logs, and IDE metadata. |
| `Docs/MODULARIZATION_GUIDE.md` | Developer reference guide explaining the Router-Fragment Unity Build architecture, the rules for adding new files, the `<ClInclude>` vs `<ClCompile>` distinction, and the PowerShell slicing methodology used during the architectural refactor. |
| `REFACTORING_METHOD.md` | Historical document describing the include-based extraction trick used to modularize the original monolithic `MouseShifter.cpp` file. Useful context for understanding why the architecture looks the way it does. |
