# 📁 MouseShifter — Codebase Documentation

> **Every file in the project documented.** This is a living document. Update it when you add a new file.
> 
> MouseShifter uses a **Unity Build** architecture — all `.cpp` files are `#include`d (not compiled independently). See `CONTRIBUTING.md` for the rules.

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
- [Resources](#resources)
- [Root Data Files](#root-data-files)

---

## Root

| File | Description |
|---|---|
| `MouseShifter.cpp` | **WinMain entry point.** Registers the window class (`HShifterWinClass`), creates the main `1250×830` popup window, initializes GDI+, calls `processAllFiles(true)` to edit ATS/ETS2 controls, loads config and gear layouts, initializes raw input, then runs the hybrid Win32 + SDL2 message loop. On exit, saves config, reverts ATS/ETS2 edits, and releases GDI+. This is the **only real compilation unit** — everything else is `#include`d from here. |
| `MouseShifter.h` | Top-level project header. Pulls in Windows SDK headers, GDI+, SDL2, and forward-declares the few functions needed at the WinMain level. |
| `MouseShifter.vcxproj` | Visual Studio 2019 project file. All fragment `.cpp` files are listed as `<ClInclude>` (not `<ClCompile>`). This lets the IDE index them without compiling them independently. |
| `MouseShifter.vcxproj.filters` | IDE filter definitions that organize files into virtual folders in Solution Explorer. |
| `MouseShifter.vcxproj.user` | Per-developer debugging settings (launch args, working directory). Not committed in a real open-source project. |
| `packages.config` | NuGet package references (if any). |
| `framework.h` | Minimal Windows header prelude (`WIN32_LEAN_AND_MEAN`, etc.). Included by `MouseShifter.h`. |
| `public.h` | vJoy public SDK header (`vJoyInterface.h` wrapping). Required for vJoy button/axis calls. |
| `targetver.h` | Windows SDK minimum version pin (`_WIN32_WINNT`). |
| `gearlayouts.ini` | **User-editable gear layout file.** Defines named H-Shifter layouts (gear positions, counts, names) loaded at startup by `Config_INI.cpp`. |

---

## Core Domain

### `Core/`

| File | Description |
|---|---|
| `Core/Config.cpp` | **Router** for the entire config subsystem. `#include`s all seven `Core/Config/` fragment files in dependency order. Exposes `LoadConfig()` and `SaveConfig()` to the rest of the app. |
| `Core/Config.h` | Header declaring the config subsystem entry points: `LoadConfig()`, `SaveConfig()`, `InitializeProfiles()`, gear input map types (`GearInput`, `InputToVJoy`), and the config data structures used across all fragments. |
| `Core/ConfigManager.cpp` | **Router** that `#include`s `Globals_Logic_Misc.cpp` and bridges between the global state layer and the config system. Also includes `LayoutComputations.cpp` via the Misc fragment chain. |

---

### `Core/Config/`

| File | Description |
|---|---|
| `Config_INI.cpp` | Parses `gearlayouts.ini` via `LoadGearLayoutsFromIni()`. Reads named `[LayoutN]` sections, populates `gearLayouts` and `gearLayoutNames` vectors. Handles BOM, whitespace trimming, and multi-pass section seeking. |
| `Config_Load.cpp` | Implements `LoadConfig()`. Reads all settings from the INI profile file using `GetPrivateProfileString` / `GetPrivateProfileInt`. Restores every slider value, toggle state, gear keybind, layout type, and sensitivity from the last saved session. |
| `Config_Save.cpp` | Implements `SaveConfig()`. Writes all current settings to the active profile INI using `WritePrivateProfileString`. Mirror image of `Config_Load.cpp`. |
| `Config_Profiles_Mgr.cpp` | Manages multi-profile lifecycle: `InitializeProfiles()` (creates default profile), `CreateProfile()`, `RenameProfile()`, `DeleteProfile()`, `SwitchProfile()`. Profile state is stored as named INI sections. |
| `Config_Processes.cpp` | Implements `RefreshProcessList()` and `IsGameProcess()`. Enumerates running processes via `CreateToolhelp32Snapshot`. Filters out browsers, IDEs, system apps, launchers, and MouseShifter itself using hardcoded exclusion lists. Only game-like processes with visible, reasonably-sized windows are added to `g_processList`. |
| `Config_Injection.cpp` | Implements `InjectDLL()` and `UninjectDLL()` using `CreateRemoteThread` + `LoadLibraryW` / `FreeLibrary`. Also implements `UpdateAutoInjection()`: the per-tick logic that monitors `g_selectedProcessId`, auto-injects `RawMouseInput.dll` when mouse blocking is enabled, and injects/uninjects `xInputBlocker.dll` based on `g_xinputBlockEnabled` + assist button state. |
| `Config_UI.cpp` | Draws the process picker, inject/uninject buttons and their state indicators in the Settings panel. Also contains the `Shift+Click` vJoy picker UI state management. |

---

### `Core/Globals/`

All files here define **global variables** shared across the entire codebase. Because this is a unity build, each variable is defined exactly once.

| File | Description |
|---|---|
| `AppGlobals.h` | **Master global header.** `#pragma once`. Chains `Config.h` → `Globals_ConfigInput.h` → `Globals_UI.h` → `Globals_Devices.h` → `AppGlobals.cpp`. This is the single include needed from `MouseShifter.cpp` to pull in all globals. |
| `AppGlobals.cpp` | Defines the final few shared items not in the headers (e.g., scroll panel variables, profile list, gear update helpers). Also pulls in `Globals_Logic_DirectInput.cpp`, `Globals_Logic_ReverseLock.cpp`, `Globals_Logic_Toggles.cpp`, `Globals_Logic_Misc.cpp`. |
| `Globals_ConfigInput.h` | Defines all **input configuration globals**: `vjoyDeviceId`, `useXInput`, `useRightStick`, knob sensitivity min/max, gear position maps (`lowerGearPositions`, `highGearPositions`), `activeGear`, `lockedInGear`, `isBorderless`, reverse lock state, drag state booleans for all sliders (`draggingKnobSlider`, `draggingSensSlider`, etc.), and the `ToggleType` enum. Also `#include`s `Transparency.h`, `public.h`, `vjoyinterface.h`, `XInput.h`. |
| `Globals_UI.h` | Defines all **UI state globals**: `KeybindAnimation` struct and `keybindAnimations` map, glow fade constants (`GLOW_FADE_IN_TIME`, `GLOW_FADE_OUT_TIME`, `MAX_GLOW_ALPHA`), panel visibility booleans (`showKeybindPanel`, `showSettingsPanel`, etc.), knob physics constants (`gearRadius`, `diagonalAssist`, `snapSpeed`, `layoutScale`), rail geometry (`railX[]`, `railOffsets12[]`, `railOffsets16[]`, `centerX/Y`, `topY/bottomY`), D2D1 factory pointer, main `hwndMain`, `knobPos`, `ghostKnobPos`, and the `RailType` enum. |
| `Globals_Devices.h` | Defines all **device and hardware globals**: vJoy axis values (`joyX/Y/RX/RY`), SDL gamepad list (`g_gamepads`), `Gamepad` struct, `InitGamepads()` / `RefreshGamepads()` (SDL controller hot-plug), XInput sensitivity and smoothing values, scroll clutch state, DirectInput globals (`g_pDI`, `g_pJoystick`, `g_diState`), DLL injection state variables (`g_selectedProcessId`, `g_autoInjectEnabled`, RECT handles for injection UI buttons), and controller sensitivity slider state. |
| `Globals_Logic_DirectInput.cpp` | Defines `InitDirectInput()` and `PollDirectInput()`. Opens the DirectInput8 interface, enumerates joystick devices to find pedals, and on each poll reads the full `DIJOYSTATE2` so clutch/brake/accelerator axes are available for toggle logic. |
| `Globals_Logic_ReverseLock.cpp` | Implements the reverse lock boundary system: `IsReverseUnlockActive_Fix()`, `ClampMovementForReverseLock_Fix()`, `ClampHorizontalMovementForReverseLock_Fix()`, `EnforceReverseLockBoundary()`. Prevents the knob from entering the reverse slot unless the configured unlock input is held. |
| `Globals_Logic_Toggles.cpp` | Implements `IsKnobToggleActive()` and `IsReverseUnlockActive()`. Both functions switch on a `ToggleType` enum (KEYBOARD, PEDAL_CLUTCH, PEDAL_BRAKE, PEDAL_ACCEL, MOUSE_LEFT/RIGHT/MIDDLE/BUTTON4/BUTTON5) and return whether the user is actively holding the bound input. Also defines toggle panel UI state variables (`togglePanelRect`, `inputPanelRectUnified`, `vJoyMouseEnabled`, `showYBar`, `showXBar`, `assistButton`). |
| `Globals_Logic_Misc.cpp` | The "bootstrap glue" fragment. Defines game-path-based ETS2/ATS config line editing (`processAllFiles()` → reads game INI files from Documents and injects/reverts mouse steering lines). Defines the full `hShifterLayouts` list and `layoutType`. `#include`s `FileBackup.cpp`, `VJoySetup.cpp`, `MouseInput.cpp`, and `LayoutComputations.cpp` to pull them into the chain. |
| `Globals_ConfigInput.h` _(also listed above in Core/Globals)_ | _(See above)_ |
| `Globals_Misc.h` | Miscellaneous helper types and small structs used across multiple domains (tooltip structs, scroll state). |
| `Globals_Physics.h` | Physics simulation constants and state for the XInput knob: dead zones, acceleration curves, lerp factors. |

---

### `Core/System/`

| File | Description |
|---|---|
| `Updater.h` | Header declaring the auto-updater API: `currentVersion`, `updateAvailable`, `latestVersion`, `CheckForUpdates()`, `PerformUpdate()`, `DownloadFile()`, `ExtractZipNew()`, `IsInternetAvailable()`. |
| `Updater.cpp` | Implements the auto-update system. `CheckForUpdates()` uses `URLDownloadToFile` (urlmon) to fetch a version manifest from a remote URL, parses it, and sets `updateAvailable`. `PerformUpdate()` downloads the release ZIP, extracts it to the app directory, and triggers a restart. Runs on a background thread to avoid blocking the UI. |
| `FileBackup.cpp` | Implements `BackupFile()`. Before MouseShifter edits any game config file (ATS/ETS2 INI), it copies the original to a `.bak` file. Also implements `RestoreBackup()` called on shutdown to revert the edits. |

---

## Input Domain

### `Input/`

| File | Description |
|---|---|
| `Input/VJoySetup.cpp` | **Router** for the vJoy subsystem. `#include`s all five `Input/VJoy/` fragments. Exposes `SetGearKey()`, `ReleaseGearKey()`, `GetVJoyButtonCount()`. |
| `Input/MouseInput.cpp` | Implements `InitRawInput()`. Calls `RegisterRawInputDevices()` to subscribe to `HID_USAGE_GENERIC_MOUSE` raw input events for all devices. This enables per-device delta tracking, bypassing mouse acceleration. |

---

### `Input/VJoy/`

| File | Description |
|---|---|
| `VJoy_Init.cpp` | Initializes the vJoy virtual device: calls `vJoyEnabled()`, `AcquireVJD()`, gets axis ranges (`GetVJDAxisMax/Min`), resets the vJoy state to center, and stores `axisMin/Max` for later normalization. |
| `VJoy_State.cpp` | Defines the `gearInputMap` (maps gear name strings like `"1"`, `"R"`, `"N"` to `GearInput` structs containing type and code) and the `inputMap` (secondary key→vJoy button mappings). Also defines `GearInput`, `InputType` enum (KEYBOARD / MOUSE / VJOY_BUTTON). |
| `VJoy_GearControl.cpp` | Implements `SetGearKey(gear)` and `ReleaseGearKey()`. Looks up the gear in `gearInputMap` and fires the appropriate `SendInput()` (keyboard), `SetBtn()` (vJoy), or mouse button event. Also performs glow animation triggering for the activated gear row in the keybind panel. |
| `VJoy_RawInput.cpp` | Implements `UpdateVJoyFromMouse()`. Maps raw mouse delta values to vJoy X/Y axis values for mouse-steering mode (`bindingModeForRAxis`). |
| `VJoy_ThreadState.cpp` | Manages thread-safe toggle state for the vJoy mouse output thread. Defines `StartVJoyThread()`, `StopVJoyThread()`, and the mutex-guarded flag `vJoyMouseEnabled`. Ensures the vJoy axis update loop can be started and stopped cleanly. |

---

### `Input/Submodules/`

| File | Description |
|---|---|
| `Input_Pedals.cpp` | Implements `PollPedalBinding()`. On each timer tick, checks if the user is in "pedal binding mode" (`togglePedalBeingSet`) and reads `g_diState` to detect which pedal axis spiked, then binds it. Also calls `UpdateAutoInjection()` state check for pedal-driven toggle. |
| `Input_RawMouse.cpp` | Implements `ProcessRawInput()`. Handles `WM_INPUT` messages: reads `RAWINPUT` struct, identifies the source device, applies sensitivity scaling, diagonal assist, and reverse lock clamping, then updates `knobPos`. Calls `UpdateVJoyFromMouse()` for steering axis output. |
| `Input_ReverseLock.cpp` | The "fix" wrapper for reverse lock. Wraps `IsReverseUnlockActive()` with additional logic that tracks whether the knob is currently on the reverse rail (`g_reverseAuthorizedOnRail`) and prevents entry unless the unlock is actively held. |
| `Input_XInputEngine.cpp` | Implements `UpdateKnobFromXInput()`. Reads XInput gamepad state (`XInputGetState`), selects left or right stick based on `useRightStick`, applies `xInputSensitivity` and optional `axisSmoothingFactor`, and moves the knob. Handles the Ghost Knob (`useAssistPointer`) by routing stick input to `ghostKnobPos` instead. Also implements `UpdateVJoyFromXInput()` for mouse steering axis output from the controller. |
| `XInput_Knob_Physics.cpp` | **Core knob physics fragment.** Handles snap detection (Euclidean distance for H-Shifter, vertical-only for PRNDL), rail transitions (`HORIZONTAL` ↔ `VERTICAL`), lerp-based ghost→real knob "commit" animation, and hysteresis snap-in/snap-out with `realKnobSnapped` / `ghostKnobSnapped` guards. Calls `SafeRumble()` on snap events. |
| `XInput_Knob_Prefix.cpp` | Entry-wrapper fragment that sets up the `dx`/`dy` variables and `targetKnob` pointer (either `&knobPos` or `&ghostKnobPos`) before `XInput_Knob_Physics.cpp` executes its logic. |
| `XInput_Knob_Transparency.cpp` | In XInput mode, monitors knob movement and updates `SetLayeredWindowAttributes` alpha — the XInput equivalent of the mouse-driven dynamic transparency logic. |
| `XInput_Rumble.cpp` | Implements `SafeRumble(leftMotor, rightMotor, durationMs)`. Fires SDL haptic rumble on the currently selected gamepad (`g_gamepads[g_selectedGamepadIndex]`). Used by physics fragment on gear snap. |
| `XInput_vJoy.cpp` | Maps XInput axis values from the controller's steering axis (right stick X, or left stick X) to the vJoy X axis in real time for mouse-steering output, independent of knob gear logic. |

---

## Windows Domain

### `Windows/`

| File | Description |
|---|---|
| `Windows/WindowProc.cpp` | **Master `WndProc` function.** Handles `WM_CREATE` (window initialization: Direct2D factory, SDL gamepad init, titlebar close/max buttons), `WM_CLOSE` (shutdown), `WM_CHAR` / `WM_KEYDOWN` / `WM_SYSKEYDOWN` (text entry for profile naming, keybind capture), `WM_CAPTURECHANGED` (drag state reset), `WM_SIZE` (layout recompute), `WM_PAINT` (calls `DrawShifterGDIPlus()`), and `WM_NCHITTEST` (custom drag area for the borderless popup window). Sub-routers for `WM_INPUT`, `WM_LBUTTONDOWN/UP/MOVE`, and `WM_TIMER` are `#include`d from `Windows/Handlers/`. |

---

### `Windows/Handlers/`

| File | Description |
|---|---|
| `WindowProc_Input.cpp` | Sub-router for `WM_INPUT`. `#include`s the five `Windows/Mechanics/` fragments in order: Prefix → Keyboard → MouseCore → MouseBinds → MousePhysics. |
| `WindowProc_MouseEvents.cpp` | Sub-router for `WM_LBUTTONDOWN`, `WM_MOUSEMOVE`, and `WM_LBUTTONUP`. `#include`s the three `Windows/MouseEvents/` fragments. |
| `WindowProc_Timer.cpp` | Sub-router for `WM_TIMER`. `#include`s the five `Windows/Timer/` fragments in execution order: InputPolling → GlowAnimations → StateTracking → SmartRedraw → DynamicTransparency. |

---

### `Windows/Mechanics/`

| File | Description |
|---|---|
| `Input_Prefix.cpp` | Prefix invoked at the start of every `WM_INPUT` message. Allocates the `RAWINPUT` buffer via `GetRawInputData`, extracts `raw->data.mouse`. Sets up shared variables (`dx`, `dy`, device handle) consumed by subsequent fragments. |
| `Input_Keyboard.cpp` | Handles keyboard input within `WM_INPUT`. Specifically manages the `WM_KEYDOWN` cases for cursor navigation in the new-profile text field (arrow keys, Home, End, Escape). |
| `Input_MouseCore.cpp` | Core raw mouse dispatcher. Checks if the incoming device matches the selected steering device. For vJoy mouse-steering mode, calls `UpdateVJoyFromMouse()`. For regular shifter mode, calls `ProcessRawInput()` to update `knobPos`. |
| `Input_MouseBinds.cpp` | Handles raw mouse button events (`RI_MOUSE_LEFT_BUTTON_DOWN`, etc.) to detect when the user clicks a pedal binding row or the assist-button binding row in the toggle panel. |
| `Input_MousePhysics.cpp` | The main H-Shifter physics fragment for mouse input. Applies `knobSensitivity`, diagonal assist, rail tracking, gear snap-in/snap-out, and neutral handling. `#include`s `Input_Mouse_PRNDL.cpp` and `Input_Mouse_HShifter.cpp` conditionally based on `layoutType`. |
| `Input_Mouse_PRNDL.cpp` | PRNDL-specific mouse movement: vertical-only with strong horizontal centering pull. Snaps purely by Y-axis distance. |
| `Input_Mouse_HShifter.cpp` | H-Shifter diamond intersection logic. Checks if the knob is within a rail intersection region and enables switching between horizontal and vertical rail modes. |

---

### `Windows/MouseEvents/`

| File | Description |
|---|---|
| `MouseEvents_LButtonDown.cpp` | **Router** for `WM_LBUTTONDOWN`. `#include`s four sub-fragments that handle clicks on different UI areas. |
| `LButtonDown_Devices.cpp` | Handles left-click events on device-selection UI: the gamepad combo dropdown, vJoy device picker, mouse/steering device binder, and injection process picker. |
| `LButtonDown_Dropdowns.cpp` | Handles left-click on gear-layout and H-Shifter layout dropdown menus. Opens/closes dropdowns, applies the selected layout to `layoutType` and calls `ComputeLayout()`. |
| `LButtonDown_Panels.cpp` | Handles left-click on panel toggle buttons (Settings, Keybind, Input, Toggle panels), the close/minimize buttons, and the title bar drag area. |
| `LButtonDown_Profiles.cpp` | Handles left-click on profile management UI: create, rename, delete, and switch active profile buttons. |
| `LButtonDown_Settings.cpp` | Handles left-click on all slider drag initiation and toggle checkbox clicks in the Settings panel (knob radius, sensitivity, snap speed, layout scale, diagonal, all toggles). |
| `MouseEvents_MouseMove.cpp` | Handles `WM_MOUSEMOVE` for dragging sliders (knob radius, sensitivity, snap speed, diagonal, layout scale, transparency, scroll clutch speed, etc.) and hovering over profile rows. |
| `MouseEvents_LButtonUp.cpp` | Handles `WM_LBUTTONUP` to terminate all drag operations and reset all `dragging*` flags. Saves config on interactive drag end. |

---

### `Windows/Timer/`

| File | Description |
|---|---|
| `Timer_InputPolling.cpp` | The **primary per-tick update fragment** called every `WM_TIMER`. Calls `UpdateKnobMovement()`, `UpdateAutoInjection()`, `UpdateSmoothScroll()`, `UpdateKnobFromXInput()`, `UpdateVJoyFromXInput()`, `PollDirectInput()`, `PollPedalBinding()`. Also handles F9 knob disable state and tooltip timer. |
| `Timer_GlowAnimations.cpp` | Advances all active `KeybindAnimation` instances each tick. Fades glow alpha in (held = immediate max, released = fade-in within `GLOW_FADE_IN_TIME`) then fades out over `GLOW_FADE_OUT_TIME`. Cleans up completed animations from `keybindAnimations`. Triggers `InvalidateRect` only if any animation actually changed. |
| `Timer_StateTracking.cpp` | Tracks state changes that require a redraw: knob enable/disable flicker, knob position change, active gear change, clutch indicator change. Triggers targeted `InvalidateRect` only on the specific changed region (knob rect, gear indicator rect) to minimize CPU usage. |
| `Timer_SmartRedraw.cpp` | **Router** for smart redraw logic. `#include`s `Timer_Redraw_Knob.cpp` and `Timer_Redraw_Bars.cpp`. Decides whether a full, knob-only, or bar-only redraw is needed based on what changed. If `disableSmartRedraws` is true, always does a full invalidate. |
| `Timer_Redraw_Knob.cpp` | Per-tick redraw logic for the knob and ghost knob areas. Computes RECT bounds around the current and previous knob positions, inflates by knob radius, and calls `InvalidateRect` only for that region. Handles the ghost knob separately. |
| `Timer_Redraw_Bars.cpp` | Per-tick redraw logic for the indicator bars (X-axis steering bar and Y-axis clutch/brake bar). Tracks previous bar values and only invalidates the bar region if the telemetry value changed beyond a threshold. |
| `Timer_DynamicTransparency.cpp` | Dynamic transparency system (borderless mode only). Every ~1500ms, samples the background brightness under the app window (`GetHShifterBackgroundBrightnessDebug`), computes a `brightnessFactor`, and smoothly adjusts `minAlpha` using a 3-sample history with hysteresis. When the knob moves or the toggle is held, snaps to `maxAlpha`. When idle, fades down to `dynamicMinAlpha` using exponential decay via `SetLayeredWindowAttributes`. Also drives smooth scrolling for settings and right panels. |

---

## UI Domain

### `UI/`

| File | Description |
|---|---|
| `UI/ShifterUI.cpp` | **Master paint function `DrawShifterGDIPlus(hwnd, hdc)`**. Creates a fully off-screen 32-bit DIBSection, initializes a GDI+ `Graphics` object on it, clears to dark background with optional Win11 Acrylic blur, then `#include`s `ShifterUI_Drawing`, `ShifterUI_SidePanels`, and `ShifterUI_Settings`. Finally, `BitBlt`s the completed memory DC to the real HDC for flicker-free rendering. |

---

### `UI/Handlers/`

| File | Description |
|---|---|
| `ShifterUI_Drawing.cpp` | **Router** for all canvas drawing. `#include`s the seven `UI/Drawing/` fragments: HotkeyOverlay → HShifterRails → Gears → ReverseGear → GhostKnob → MainKnob → IndicatorBars. Executed in back-to-front order (painter's algorithm). |
| `ShifterUI_SidePanels.cpp` | Draws the three right-side panels: **Keybind Panel** (scrollable list of gear→key rows with glow overlays), **Input Panel** (key→vJoy button mappings), and **Toggle Panel** (knob toggle key, assist button, and reverse unlock button rows with live binding capture UI). Uses `Segoe UI` font, green `#00FF88` accent. |
| `ShifterUI_Settings.cpp` | **Router** for the Settings Panel. `#include`s six `UI/Settings/` fragments: VariablesAndTitles → Sliders → Toggles → LayoutsAndProfiles → Transparencies → GameControl → VJoyAndKeybinds. Ends with `graphics.ResetClip()`. |
| `ShifterUI_Overlays.cpp` | Draws floating overlays: the borderless mode "grab handle" indicator, the update-available button, and any modal dialogs (e.g., the vJoy picker modal). |
| `Graphics_StaticElements.cpp` | Draws non-interactive decorative elements: window border/shadow, panel dividers, and the title bar area (close button glyph, maximize button glyph). |
| `Graphics_DrawBorderless.cpp` | Renders the full borderless overlay mode. Calls `UpdateLayeredWindow` with the composed DIBSection bitmap to produce a per-pixel alpha transparent window. |
| `Graphics_ScreenCapture.cpp` | Implements `GetHShifterBackgroundBrightnessDebug()`. Captures a small screen region under the H-Shifter canvas via `BitBlt` from the desktop DC, then computes the luminance average of the captured pixels. Used by dynamic transparency. |
| `Graphics_RedrawMath.cpp` | Implements `ComputeLayout(hwnd)` and `ComputeIntersections()`. Calculates all gear positions, rail X-coordinates, top/bottom Y, knob bounds, and intersection radii based on `layoutType`, `layoutScale`, `gearRadius`, `is16GearSet`, and the current window size. Called on `WM_SIZE` and after settings changes. |
| `Transparency.cpp` | Implements `EnableWin11Blur(hwnd)` (sets `DWMWA_SYSTEMBACKDROP_TYPE` for Win 11 Acrylic), `IsTransparencyEnabled()` (checks system transparency setting), and the `SetLayeredWindowAttributes` wrapper for windowed transparency mode. |
| `Transparency.h` | Header declaring the transparency API: `EnableWin11Blur()`, `IsTransparencyEnabled()`, `currentAlpha`, `minAlpha`, `maxAlpha`, `isTransparent`, `dynamicTransparencyEnabled`, `alphaDecay`, `transparencyFadeDelay`. |

---

### `UI/Borderless/`

| File | Description |
|---|---|
| `Borderless_Math.cpp` | Computes the geometry for borderless overlay mode: calculates the overlay window position, scale factor, and the `RECT` that maps the H-Shifter canvas onto the target game window. |

---

### `UI/Drawing/`

All fragments here are `#include`d by `ShifterUI_Drawing.cpp` and execute within the GDI+ `Graphics` context.

| File | Description |
|---|---|
| `Drawing_HotkeyOverlay.cpp` | Draws a small overlay showing the currently active hotkeys (F9 state, toggle key, reverse unlock) as floating text hints at the top of the canvas. |
| `Drawing_HShifterRails.cpp` | Draws the H-Shifter rail grid: horizontal and vertical lines connecting gear positions, including the 4-rail or 5-rail layout for 12-gear or 16-gear sets. Applies `layoutScale` and `railOffsets`. |
| `Drawing_Gears.cpp` | Draws the gear position circles for all entries in `lowerGearPositions`. Colors each circle based on snap state: active gear = green accent `#00FF88`, snapped = bright, others = dark grey. Also triggers glow animation pulses. |
| `Drawing_ReverseGear.cpp` | Draws the Reverse (`R`) gear circle with a separate visual treatment (typically red or distinct styling) and applies the reverse lock overlay (grey-out or lock icon) when `reverseLockEnabled` is true and the unlock input is not held. |
| `Drawing_GhostKnob.cpp` | Draws the ghost / assist pointer knob as a semi-transparent smaller circle at `ghostKnobPos`. Only rendered when `useAssistPointer` is true. |
| `Drawing_MainKnob.cpp` | Draws the main shifter knob circle at `knobPos` with configurable `knobRadius`. Fills with a gradient or solid color, applies a flash highlight (`knobFlash`) when knob movement is toggled, and draws a red outline when knob movement is disabled. |
| `Drawing_IndicatorBars.cpp` | Draws the X-axis (steering) bar and Y-axis (clutch/brake) telemetry bars when `showXBar` / `showYBar` are enabled. Normalizes axis values from vJoy range to pixel width and draws filled bar rectangles with labels. |

---

### `UI/Math/`

| File | Description |
|---|---|
| `LayoutComputations.cpp` | Implements `ComputeLayout(hwnd)` and `ComputeIntersections()`, plus helper `IsInsideIntersection(x, y)`. Calculates all gear slot `POINT` positions in `lowerGearPositions` and `highGearPositions`, sets `railX[]`, `centerX/Y`, `topY/bottomY`, `knobMinX/MaxX/Y`, and `gearSnapInThreshold` based on the current `layoutType` and `layoutScale`. Also computes intersection zones where horizontal/vertical rail switching is allowed. |

---

### `UI/Settings/`

All fragments here are `#include`d by `ShifterUI_Settings.cpp` and draw into the Settings Panel GDI+ context.

| File | Description |
|---|---|
| `Settings_VariablesAndTitles.cpp` | Draws the Settings panel background, title ("Settings"), and defines the shared drawing variables (fonts, brushes, `y` cursor, clip rect) used by all subsequent settings fragments. |
| `Settings_Sliders.cpp` | Draws all interactive sliders in the Settings panel: knob radius, mouse sensitivity, snap speed, snap-in threshold, diagonal assist, layout scale, scroll clutch sensitivity, axis smoothing factor, controller sensitivity, smooth scroll speed, brake resistance, acceleration resistance. Computes and stores each slider's RECT for hit-testing. |
| `Settings_Toggles.cpp` | Draws all toggle checkboxes: mouse steering enable, neutral enable, realistic knob, clutch lock, reverse lock, XInput enable, use right stick, disable real knob, knob acceleration, show clutch indicator, scroll clutch, half-scroll clutch, invert scroll, use LT as clutch, PlayStation mode, invert PS Y-axis, use axis smoothing, show Y-bar, show X-bar, hide high gears, smart redraw optimization. |
| `Settings_LayoutsAndProfiles.cpp` | Draws the gear layout dropdown (H-Shifter layout type picker), gear layout scale, 16-gear toggle, profile list panel (create, rename, delete, switch), and the INI custom gear layouts dropdown. |
| `Settings_Transparencies.cpp` | Draws the transparency section: windowed alpha slider (`minAlpha`), dynamic transparency toggle and configuration sliders (`alphaDecay`, `transparencyFadeDelay`), win11 blur toggle, and the borderless mode toggle. |
| `Settings_GameControl.cpp` | Draws the game integration section: process list with the process picker combo, inject/uninject buttons, auto-inject toggle, mouse block toggle, XInput block toggle, and the "Invert Assist Axes" toggle. |
| `Settings_VJoyAndKeybinds.cpp` | Draws the vJoy device picker (dropdown of available vJoy IDs), the vJoy button picker modal trigger, and the "binding mode for R axis" toggle for steering axis output. |

---

## Resources

| File | Description |
|---|---|
| `Resources/MouseShifter.rc` | Windows resource script. Embeds the application icon into the executable. |
| `Resources/Resource.h` | Auto-generated resource ID constants (e.g., `IDI_MOUSESHIFTER`). |
| `Resources/MouseShifter.ico` | Main application icon (full-size). |
| `Resources/small.ico` | Small taskbar icon (16×16). |

---

## Root Data Files

| File | Description |
|---|---|
| `gearlayouts.ini` | User-editable file containing named gear layout definitions. Loaded at startup by `Config_INI.cpp`. Format: `count=N` header followed by `[LayoutN]` sections with gear position `key=value` pairs. |
| `profiles/profile_1.ini` | Default user profile INI. Stores all user settings (keybinds, sensitivities, toggles, layout type, etc.) in INI format. Additional profiles are created alongside this file by `Config_Profiles_Mgr.cpp`. |
| `.gitignore` | Standard Git ignore rules. Should exclude `*.user`, `Debug/`, `Release/`, `*.ncb`, `*.suo`, build logs. |
| `Docs/MODULARIZATION_GUIDE.md` | Developer reference guide explaining the Router-Fragment Unity Build architecture, the rules for adding new files, and the slicing methodology used during the refactor. |
