#include "AppGlobals.h"﻿#include <windows.h>
{
    TOGGLE_KEYBOARD,
    TOGGLE_PEDAL_CLUTCH,
    TOGGLE_PEDAL_BRAKE,
    TOGGLE_PEDAL_ACCEL,
    TOGGLE_MOUSE_LEFT,
    TOGGLE_MOUSE_RIGHT,
    TOGGLE_MOUSE_MIDDLE,
    TOGGLE_MOUSE_BUTTON4,
    TOGGLE_MOUSE_BUTTON5
};
int vjoyDeviceId = 1; // ID of your vJoy device
bool useXInput = false;               // whether to use XInput for knob movement
bool disableRealKnobMovement = false; // <- define here
bool useRightStick = true; // true = RS, false = LS
RECT knobAccelToggleRect;  // global rect for the Precision Knob Movement toggle
bool knobAccelerationEnabled = false; // false = actually enabled
bool useThrottleBrakeAxes = false;
RECT useThrottleBrakeToggleRect;  // Fixed typo in variable name

std::map<std::string, POINT> lowerGearPositions;
std::map<std::string, POINT> highGearPositions;
std::string activeGear;
bool lockedInGear = false;
bool prevLButtonDown = false; // track previous left button state
bool isBorderless = false;
bool isTransparent = false;
int knobMinX, knobMaxX, knobMinY, knobMaxY;
WORD heldGearKey = 0;            // VK code of currently held key
bool knobMovementEnabled = true; // default = enabled
// Settings panel
bool mouseSteeringEnabled = false; // checkbox state
RECT mouseSteeringToggleRect;      // global rect for checkbox
using namespace Gdiplus;

bool showSettingsPanel = true;
RECT settingsPanelRect;
bool isNeutralEnabled = true; // default enabled
bool prevShowSettingsPanel = false;
bool prevShowKeybindPanel = false;
bool prevShowInputPanel = false;
bool prevShowTogglePanel = false;
// dragging
int knobRadiusMin = 20;
int knobRadiusMax = 150;
bool draggingKnobSlider = false;
float knobSensitivityMin = 0.05f;
float knobSensitivityMax = 2.0f;
bool draggingSensSlider = false;
bool draggingDiagSlider = false;
bool draggingSnapSpeedSlider = false;
float snapSpeedMin = 0.0f;
float snapSpeedMax = 1.0f;
bool draggingLayoutSlider = false;
bool isDraggingGearRadius = false;
float steeringSensitivity = 1.0f; // X-axis sensitivity
float accBrakeSensitivity = 1.0f; // Y-axis sensitivity
bool draggingSteeringSlider = false;
bool draggingAccBrakeSlider = false;
// --- Global variables at the top of your .cpp file ---
RECT knobSliderRect;
RECT gearSliderRect;
RECT sensSliderRect;
RECT diagSliderRect;
RECT snapInSliderRect;
RECT snapSpeedSliderRect;
RECT layoutScaleSliderRect;
RECT neutralToggleRect;
RECT useRightStickToggleRect;
RECT realisticKnobToggleRect;
bool realisticKnob = false; // Set this based on your configuration
bool clutchLockGear = false;
RECT clutchLockGearToggleRect;

// --- Reverse Gear Lock ---
bool reverseLockEnabled = true;
ToggleType g_reverseUnlockType = TOGGLE_KEYBOARD;
int g_reverseUnlockKey = VK_LSHIFT; // Default to Left Shift
bool reverseUnlockBeingSet = false;
RECT reverseLockToggleRect;
RECT reverseUnlockKeyRect;

// Snap-in threshold slider
bool draggingSnapInSlider = false;
float snapInMin = 0.5f;
float snapInMax = 4.0f;
RECT noReverseToggleRect; // global or at the same scope as other toggle RECTs
bool hideHighGears = false; // toggle this as needed
RECT hideHighGearsToggleRect;

std::map<std::string, KeybindAnimation> keybindAnimations;
const float GLOW_FADE_IN_TIME = 200.0f;  // milliseconds - faster fade in
const float GLOW_FADE_OUT_TIME = 400.0f; // milliseconds - fade out when released
const float MAX_GLOW_ALPHA = 0.8f;
// keybinding panel

bool showKeybindPanel = true;                // toggleable with a hotkey later
std::string keybindBeingSet = "";            // which gear we’re rebinding right now
RECT keybindPanelRect = { 720, 0, 900, 6500 }; // panel on right side

bool isSnapped = false;
bool realKnobSnapped = false;
bool ghostKnobSnapped = false;
bool g_reverseAuthorizedOnRail = false;
std::map<int, bool> buttonPressedState;

// Forward declarations for Reverse Lock Fix
bool IsReverseUnlockActive_Fix();
void ClampMovementForReverseLock_Fix(int& newY, int currentX);
void ClampHorizontalMovementForReverseLock_Fix(long& newX, int currentX, int currentY);
void EnforceReverseLockBoundary(POINT& pt);
void UpdateKnobMovement(HWND hwnd);
void ProcessRawInput(RAWINPUT* raw);
void UpdateVJoyFromMouse(RAWMOUSE& rm, HANDLE device);
const float knobFlashFade = 0.02f; // how fast it fades per frame
bool lastKnobMovementEnabled = true; // Track previous state
bool lastGreyOutState = false;       // Track previous grey-out state
// Gear constants
int gearRadius = 30;
const int gearRadiusMin = 15;
const int gearRadiusMax = 50; // or any max you decide

int neutralHalf = 40;               // half-height of horizontal neutral zone
int slotOffset = 60;                // vertical distance from center to gear slots
float gearSnapInMultiplier = 1.70f; // default = 2.0, multiplier for gearRadius
// ---- Globals ----
LONG joyRx = 0;                     // clutch / Rx axis
float scrollClutchSens = 5.0f;      // small multiplier, tweak 0–10
RECT scrollSensSliderRect;
bool draggingScrollSensSlider = false;
bool useScrollClutch = false; // set to false to disable scroll → Rx
RECT useScrollClutchToggleRect;   // toggle rectangle for scroll→Rx
bool showClutchIndicator = false;
RECT showClutchToggleRect;
bool invertScrollClutchAxis = false; // set true to invert axis
RECT invertScrollToggleRect;       // UI rect for the toggle
bool useHalfClutch = false; // toggle this as needed
RECT halfScrollClutchToggleRect;
bool disableSmartRedraws = false;  // Set to false to re-enable smart redraws
RECT optimizationToggleRect;

float knobSensitivity = 0.45f;                     // 1.0 = normal, 2.0 = twice as fast, 0.5 = half speed
const float gearSnapThreshold = gearRadius * 1.0f; // smaller = easier to release
float gearSnapInThreshold = gearRadius * 2.0f;
const float gearSnapOutThreshold = gearRadius * 0.2f; // easy to release
std::map<std::string, bool> gearInsideRadius;         // true = knob is currently within grab radius
float snapSpeed = 0.75f;                              // 0.0 = slow, 1.0 = instant
const WORD neutralKey = 'N';                          // VK code for "N"
bool neutralHeld = false;                             // track if we are currently holding it
// ---------------- Globals ----------------
ID2D1Factory* pFactory = nullptr;
HWND hwndMain;
POINT knobPos;
int knobRadius = 85;
bool is16GearSet = false;     // false = 12 gears, true = 16 gears
float diagonalAssist = 1.75f; // default value
const float diagMin = 1.5f;
const float diagMax = 3.0f;
const int baseIntersectionRadius = 30; // default radius
bool isRightButtonHeld = false;
bool bindingModeForRAxis = false;
RECT bindingModeForAxisToggle;
const int baseEnterVerticalThreshold = 20; // default
int enterVerticalThreshold = int(baseEnterVerticalThreshold * diagonalAssist);

RECT useXInputToggleRect;
RECT disableRealKnobMovementToggleRect;

// Rail layout
// ---------------- H-Shifter size ----------------
float layoutScale = 1.75f; // previously const
int railOffsets12[4] = { -120, -40, 40, 120 };
int railOffsets16[5] = { -140, -70, 0, 70, 140 }; // 5 rails for 16-gear layout
int railCount = is16GearSet ? 5 : 4;            // more layout

POINT railX[5]; // max rails needed = 5

int centerX, centerY;
int topY, bottomY;
bool rawInputInitialized = false;
{
    NONE = -1,
    HORIZONTAL,
    VERTICAL
};
RailType currentRail = HORIZONTAL;

int currentVerticalIndex = -1; // which vertical rail if on vertical
bool showVJoyPicker = false;
std::string vJoyPickerGear;
RECT vJoyPickerRect = { 0 };
int vJoyButtonCount = 0; // detected at startup
{
    if (!vJoyEnabled())
        return 0;
    return GetVJDButtonNumber(deviceId);
}
LONG joyX = 0;
LONG joyY = 0;
extern float xInputSensitivity; // new sensitivity multiplier
// Declare and define global sensitivity
float xInputSensitivity = 1.0f; // adjust as needed
bool useAxisSmoothing = false; // Toggle for interdependent smoothing
RECT useAxisSmoothingToggle;
bool draggingAxisSmoothingFactorSlider = false;
bool draggingSmoothScrollSlider = false;
bool draggingBrakeResistanceSlider = false;
bool draggingAccelerationResistanceSlider = false;
RECT axisSmoothingFactorSlider;
static float smoothScrollSpeed = 8.0f;  // Adjust for smoothness
RECT smoothScrollSlider;
float brakeresistanceFactor = 20.0f;
float accelerationResistanceFactor = 20.0f;  // Add this new variable
RECT brakeresistanceSlider;
RECT accelerationresistanceSlider;
LONG axisMin = 0;
LONG axisMax = 0;
LONG axisMinX = 0;      // or the minimum value your vJoy X provides
LONG axisMaxX = 32767;
const float mouseSensitivity = 10.0f; // tweak as needed
bool mouseTrackingEnabled = false;    // toggle with F12 or a button
// gdiplus
Gdiplus::Rect panelRect;
RECT g_vJoyPickerRect;               // full picker
std::vector<RECT> g_vJoyButtonRects; // individual buttons
RECT g_CloseButtonRect;
RECT g_MaxButtonRect;
bool knobDisabledByF9 = false; // true if F9 disabled knob
int hoveredProfileIndex = -1;  // Track which profile is hovered
float settingsScrollTarget = 0.0f; // target scroll
{
    RECT scrolled = r;
    scrolled.top += settingsScrollOffset;
    scrolled.bottom += settingsScrollOffset;
    return scrolled;
}
POINT ghostKnobPos = { 0, 0 };
std::string ghostSnappedGear = "";
bool useAssistPointer = false;
int lastDx = 0;
int lastDy = 0;
bool invertAssistAxes = false; // new flag, set true to invert behavior
bool useLTAsClutch = false; // enable LT as clutch
RECT invertAssistToggleRect;
RECT useLTAsClutchToggleRect;

std::vector<Gamepad> g_gamepads;  // store all connected controllers
bool PlayStationMode = true;      // your flag
bool invertPlayStationYAxis = false;
{
    // Close and clear existing gamepads
    for (auto& g : g_gamepads) {
        if (g.haptic) SDL_HapticClose(g.haptic);
        if (g.controller) SDL_GameControllerClose(g.controller);
    }
    g_gamepads.clear();

    // Re-init them
    InitGamepads();

    // Optional: Debug output
    char dbg[128];
    sprintf_s(dbg, "[SDL] Refreshed gamepads, total now: %zu\n", g_gamepads.size());
    OutputDebugStringA(dbg);
}

int g_selectedGamepadIndex = 0; // -1 = "All Gamepads" or no selection
RECT gamepadComboRect;           // like your mouse/steering combo rect
Gamepad* g = nullptr;  // pointer to currently selected gamepad

// Controller sensitivity slider globals
bool controllerDraggingSensSlider = false;
RECT controllerSensSliderRect = { 0 };
float controllerSensSliderValue = 0.5f; // normalized 0.0 - 1.0
float controllerSensMultiplier = 1.0f;  // actual sensitivity applied
static std::vector<BYTE> minAlphaHistory;
static const int HISTORY_SIZE = 3; // Check last 3 values
static const BYTE CHANGE_THRESHOLD = 8; // Minimum change required to trigger update
//DLL INJECTION
// DLL Injection variables
HWND g_hProcessCombo = NULL;
HWND g_hAutoInjectButton = NULL;
HWND g_hRefreshButton = NULL;
std::vector<DWORD> g_processList;
DWORD g_selectedProcessId = 0;
bool g_autoInjectEnabled = false;
DWORD g_lastInjectedProcessId = 0;

// DLL Injection rects
RECT processComboRect;
RECT refreshButtonRect;
RECT injectButtonRect;
RECT uninjectButtonRect;
RECT autoInjectButtonRect;
RECT xinputBlockCheckboxRect;
RECT mouseBlockCheckboxRect;
// toggle bindings
//  Define the configurable key for toggling or enabling knob movement


int g_knobToggleKey = VK_SHIFT;                // still used when keyboard binding
ToggleType g_knobToggleType = TOGGLE_KEYBOARD; // tracks if it’s a key or a pedal
bool togglePedalBeingSet = false; // true = waiting for pedal press

// NEW:

// DirectInput globals:
LPDIRECTINPUT8 g_pDI = nullptr;
LPDIRECTINPUTDEVICE8 g_pJoystick = nullptr;
DIJOYSTATE2 g_diState;
bool g_hasPedals = false;

// --- Enum callback: pick first valid gamepad ---
{
    // Skip vJoy devices
#ifdef UNICODE
    if (wcsstr(pdidInstance->tszProductName, L"vJoy") != nullptr)
#else
    if (strstr(pdidInstance->tszProductName, "vJoy") != nullptr)
#endif
        return DIENUM_CONTINUE;

    if (FAILED(g_pDI->CreateDevice(pdidInstance->guidInstance, &g_pJoystick, NULL)))
        return DIENUM_CONTINUE;

    return DIENUM_STOP; // use first valid joystick/gamepad
}

// --- Initialize DirectInput ---
{
    if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
        IID_IDirectInput8, (void**)&g_pDI, NULL)))
        return false;

    g_pJoystick = nullptr;
    g_hasPedals = false;

    g_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, hwnd, DIEDFL_ATTACHEDONLY);

    return g_pJoystick != nullptr;
}

// --- Setup the joystick/gamepad ---
{
    if (!g_pJoystick)
        return false;

    g_pJoystick->SetDataFormat(&c_dfDIJoystick2);
    g_pJoystick->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

    g_hasPedals = true;
    return true;
}

// --- Poll device ---
{
    if (!g_pJoystick)
        return;

    if (FAILED(g_pJoystick->Poll()))
        g_pJoystick->Acquire();

    g_pJoystick->GetDeviceState(sizeof(DIJOYSTATE2), &g_diState);
}

bool IsReverseUnlockActive();
{
    if (reverseLockEnabled && !IsReverseUnlockActive())
    {
        POINT rPos = { 0,0 };
        bool rFound = false;

        // Check both maps to handle range switching
        auto findR = [&](const std::map<std::string, POINT>& map) {
            auto it = map.find("R");
            if (it != map.end()) { rPos = it->second; rFound = true; }
        };
        findR(lowerGearPositions);
        if (!rFound) findR(highGearPositions);

        if (rFound)
        {
            // Anti-Teleport Latch logic (Directional):
            // Only allow bypass if we are already "deep" in the Reverse side.
            // If Reverse is Top, we must be UP (y < centerY). If Reverse is Bottom, we must be DOWN (y > centerY).
            // This prevents bypassing from the OPPOSITE gear (which is also "far from center" but on the wrong side).
            bool safeSide = false;
            if (activeGear == "R")
            {
                safeSide = true;
            }
            else
            {
                int buffer = 15;
                if (rPos.y == topY && knobPos.y < centerY - buffer) safeSide = true;       // Already up in Reverse zone
                else if (rPos.y == bottomY && knobPos.y > centerY + buffer) safeSide = true; // Already down in Reverse zone
            }

            if (safeSide)
                return;

            // Check if any other gear has same X (same rail)
            bool hasOpposite = false;
            auto checkOpposite = [&](const std::map<std::string, POINT>& map) {
                for (auto& kv : map) {
                    if (kv.first != "R" && abs(kv.second.x - rPos.x) < 5) {
                        hasOpposite = true;
                        break;
                    }
                }
            };
            checkOpposite(lowerGearPositions);
            if (!hasOpposite) checkOpposite(highGearPositions);

            // If Reverse is on an ISOLATED rail (no opposite gear), and we are physically IN that rail
            // (meaning we passed the horizontal lock), then ALLOW vertical movement.
            // The Horizontal Lock is the guard; once you're in, you're in.
            if (!hasOpposite)
            {
                if (abs(currentX - rPos.x) < enterVerticalThreshold)
                    return; // Bypass vertical clamp
            }

            // Otherwise (Shared Rail, or not aligned), enforce strict vertical locking
             // Use WIDER threshold to catch diagonal/side entries (prevent sneaking in from the side)
             if (abs(currentX - rPos.x) < (enterVerticalThreshold * 3))
             {
                 if (rPos.y == topY) { // Reverse is at top
                     if (newY < centerY) newY = centerY;
                 }
                 else if (rPos.y == bottomY) { // Reverse is at bottom
                     if (newY > centerY) newY = centerY;
                 }
             }
        }
    }
}
{
    if (reverseLockEnabled && !IsReverseUnlockActive())
    {
        POINT rPos = { 0,0 };
        bool rFound = false;

        auto findR = [&](const std::map<std::string, POINT>& map) {
            auto it = map.find("R");
            if (it != map.end()) { rPos = it->second; rFound = true; }
        };
        findR(lowerGearPositions);
        if (!rFound) findR(highGearPositions);

        if (rFound)
        {
            // Check if any other gear has same X (same rail)
            bool hasOpposite = false;
            auto checkOpposite = [&](const std::map<std::string, POINT>& map) {
                for (auto& kv : map) {
                    if (kv.first != "R" && abs(kv.second.x - rPos.x) < 5) {
                        hasOpposite = true;
                        break;
                    }
                }
            };
            checkOpposite(lowerGearPositions);
            if (!hasOpposite) checkOpposite(highGearPositions);

            // If Reverse is on an ISOLATED rail (no opposite gear), ALWAYS clamp.
            // If Reverse SHARES a rail (hasOpposite), we normally allow entry (for the opposite gear).
            // BUT: If key is NOT held, and we are entering the "Reverse Zone" (vertical height), we must CLAMP.
            // This prevents "Flanking": moving diagonally into the Reverse side of a shared rail.
            bool shouldClamp = !hasOpposite;
            if (hasOpposite)
            {
                 // Check if we are in the "Danger Zone" vertically
                 if (rPos.y == topY && currentY < centerY) shouldClamp = true;       // Top Reverse: We are in upper half
                 else if (rPos.y == bottomY && currentY > centerY) shouldClamp = true; // Bottom Reverse: We are in lower half
            }

            if (shouldClamp)
            {
                // Find index of R rail
                int rRailIndex = -1;
                for (int i = 0; i < railCount; ++i) {
                    if (abs(railX[i].x - rPos.x) < 5) {
                        rRailIndex = i;
                        break;
                    }
                }

                if (rRailIndex != -1)
                {
                    // Find closest rail towards center (adjacent)
                    int adjX = centerX;
                    int minDiff = 99999;
                    bool foundAdj = false;

                    for (int i = 0; i < railCount; ++i)
                    {
                        if (i == rRailIndex) continue;

                        int diff = abs(railX[i].x - rPos.x);
                        if (diff < minDiff)
                        {
                            minDiff = diff;
                            adjX = railX[i].x;
                            foundAdj = true;
                        }
                    }

                    if (foundAdj)
                    {
                        // Check if we are already "inside" or closer to the Reverse rail than the Adjacent rail
                        // If so, we are already past the gate, so DO NOT clamp (latch logic)
                        // This prevents teleporting back if unlock is released while in/near Reverse
                        if (activeGear == "R" || abs(knobPos.x - rPos.x) < abs(knobPos.x - adjX))
                        {
                            return;
                        }

                        // Strict clamp to adjacent rail (with small buffer to allow entering the adjacent rail comfortably)
                        int railDist = abs(rPos.x - adjX);
                        int buffer = 25; 
                        
                        // Dynamic buffer safety: ensure buffer is strictly less than half distance to prevent crossing midpoint
                        // If buffer pushes us past midpoint, the "Are we inside?" check (dist < dist) flips to TRUE next frame
                        int maxSafeBuffer = (railDist / 2) - 2; 
                        if (maxSafeBuffer < 0) maxSafeBuffer = 0;
                        
                        if (buffer > maxSafeBuffer)
                            buffer = maxSafeBuffer;

                        if (rPos.x < adjX) // R is to the Left (e.g. -120), Adj is Right (e.g. -40)
                        {
                             // Prevent going left past Adj
                             if (newX < adjX - buffer)
                                 newX = adjX - buffer;
                        }
                        else // R is to the Right (e.g. 120), Adj is Left (e.g. 40)
                        {
                             // Prevent going right past Adj
                             if (newX > adjX + buffer)
                                 newX = adjX + buffer;
                        }                    }
                }
            }
        }
    }
}
{
    char buf[256]; // buffer for debugging
    switch (g_knobToggleType)
    {
    case TOGGLE_KEYBOARD:
    {
        bool pressed = (GetKeyState(g_knobToggleKey) & 0x8000) != 0;

        return pressed;
    }

    case TOGGLE_PEDAL_CLUTCH:
    {
        bool pressed = g_hasPedals && (g_diState.lZ > 32767);
        sprintf_s(buf, "TOGGLE_PEDAL_CLUTCH: g_hasPedals = %s, lZ = %ld, pressed = %s\n",
            g_hasPedals ? "true" : "false", g_diState.lZ, pressed ? "true" : "false");
        OutputDebugStringA(buf);
        return pressed;
    }

    case TOGGLE_PEDAL_BRAKE:
    {
        bool pressed = g_hasPedals && (g_diState.lY > 32767);
        sprintf_s(buf, "TOGGLE_PEDAL_BRAKE: g_hasPedals = %s, lY = %ld, pressed = %s\n",
            g_hasPedals ? "true" : "false", g_diState.lY, pressed ? "true" : "false");
        OutputDebugStringA(buf);
        return pressed;
    }

    case TOGGLE_PEDAL_ACCEL:
    {
        bool pressed = g_hasPedals && (g_diState.lRz > 32767);
        sprintf_s(buf, "TOGGLE_PEDAL_ACCEL: g_hasPedals = %s, lRz = %ld, pressed = %s\n",
            g_hasPedals ? "true" : "false", g_diState.lRz, pressed ? "true" : "false");
        OutputDebugStringA(buf);
        return pressed;
    }
    case TOGGLE_MOUSE_LEFT:
    {
        bool pressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        sprintf_s(buf, "TOGGLE_MOUSE_LEFT: pressed = %s\n", pressed ? "true" : "false");
        OutputDebugStringA(buf);
        return pressed;
    }
    case TOGGLE_MOUSE_RIGHT:
    {
        bool pressed = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        sprintf_s(buf, "TOGGLE_MOUSE_RIGHT: pressed = %s\n", pressed ? "true" : "false");
        OutputDebugStringA(buf);
        return pressed;
    }
    case TOGGLE_MOUSE_MIDDLE:
    {
        bool pressed = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        sprintf_s(buf, "TOGGLE_MOUSE_MIDDLE: pressed = %s\n", pressed ? "true" : "false");
        OutputDebugStringA(buf);
        return pressed;
    }
    case TOGGLE_MOUSE_BUTTON4:
    {
        bool pressed = (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
        sprintf_s(buf, "TOGGLE_MOUSE_BUTTON4: pressed = %s\n", pressed ? "true" : "false");
        OutputDebugStringA(buf);
        return pressed;
    }
    case TOGGLE_MOUSE_BUTTON5:
    {
        bool pressed = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
        sprintf_s(buf, "TOGGLE_MOUSE_BUTTON5: pressed = %s\n", pressed ? "true" : "false");
        OutputDebugStringA(buf);
        return pressed;
    }

    default:
        sprintf_s(buf, "DEFAULT case: returning false\n");
        OutputDebugStringA(buf);
        return false;
    }
}
{
    char buf[256];
    switch (g_reverseUnlockType)
    {
    case TOGGLE_KEYBOARD:
    {
        return (GetKeyState(g_reverseUnlockKey) & 0x8000) != 0;
    }
    case TOGGLE_PEDAL_CLUTCH:
    {
        return g_hasPedals && (g_diState.lZ > 32767);
    }
    case TOGGLE_PEDAL_BRAKE:
    {
        return g_hasPedals && (g_diState.lY > 32767);
    }
    case TOGGLE_PEDAL_ACCEL:
    {
        return g_hasPedals && (g_diState.lRz > 32767);
    }
    case TOGGLE_MOUSE_LEFT:
    {
        return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    }
    case TOGGLE_MOUSE_RIGHT:
    {
        return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    }
    case TOGGLE_MOUSE_MIDDLE:
    {
        return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
    }
    case TOGGLE_MOUSE_BUTTON4:
    {
        return (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
    }
    case TOGGLE_MOUSE_BUTTON5:
    {
        return (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
    }
    default:
        return false;
    }
}
{
    TCHAR buf[256];

    // Get keyboard key state
    int keyState = (GetKeyState(g_knobToggleKey) & 0x8000) ? 1 : 0;

    // Get DirectInput axes
    LONG clutch = g_diState.lRx;
    LONG brake = g_diState.lZ;
    LONG accel = g_diState.lY;

    // Format everything into a string
    _stprintf_s(buf, _T(
        "g_knobToggleKey: %d, KeyActive: %d, g_knobToggleType: %d\n"
        "DI Axes -> Clutch: %ld, Brake: %ld, Accel: %ld\n"),
        g_knobToggleKey,
        keyState,
        (int)g_knobToggleType,
        clutch,
        brake,
        accel
    );

    // Output to Visual Studio debug window
    OutputDebugString(buf);
}

// RECT for the toggle panel
// Toggle Panel RECT and state
RECT togglePanelRect;
bool toggleInputBeingSet = false;
RectF inputPanelRectUnified; // global or class member
Rect togglePanelRectUnified; // global or class member
RECT g_toggleKeyRect;
RECT g_assistButtonRect;
bool vJoyMouseEnabled = true;
bool showYBar = false; // toggle for acceleration/brake bar
bool showXBar = false; // toggle for mouse steering bar
RECT showYBarToggleRect;
RECT showXBarToggleRect;

// Control whether the toggle panel is visible
bool showTogglePanel = true; // set to true to always show, or manage visibility as needed
// Global assist button, default is Right Bumper (RB)
WORD assistButton = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;

// Flag to indicate whether the user is binding the assist button
bool assistButtonBeingSet = false;

// Temporary to store new binding during input
WORD newAssistButton = 0;


//update system
{
    int majA = 0, minA = 0, majB = 0, minB = 0;
    sscanf_s(a.c_str(), "%d.%d", &majA, &minA);
    sscanf_s(b.c_str(), "%d.%d", &majB, &minB);


    if (majA != majB) return majA - majB;
    return minA - minB;
}
{
    DWORD flags;
    return InternetGetConnectedState(&flags, 0) != 0;
}

std::string currentVersion = "5.0";
bool updateAvailable = false;
std::string latestVersion = "";
RECT updateButtonRect;
bool checkingForUpdates = false;

#define DBGPRINT(fmt, ...) { char buf[512]; sprintf_s(buf, fmt, __VA_ARGS__); OutputDebugStringA(buf); }
{
    HINTERNET hInternet = InternetOpenA("MouseShifter Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return "";

    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) { InternetCloseHandle(hInternet); return ""; }

    std::string result;
    char buffer[1024];
    DWORD bytesRead;

    while (InternetReadFile(hUrl, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        result += buffer;
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    return result;
}
{
    HINTERNET hInternet = InternetOpenA("MouseShifter Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) { InternetCloseHandle(hInternet); return false; }

    HANDLE hFile = CreateFileW(localPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) { InternetCloseHandle(hUrl); InternetCloseHandle(hInternet); return false; }

    char buffer[8192];
    DWORD bytesRead, bytesWritten;
    bool success = true;

    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
    {
        if (!WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL) || bytesWritten != bytesRead)
        {
            success = false;
            break;
        }
    }

    CloseHandle(hFile);
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    return success;
}
namespace fs = std::filesystem;
{
    // Create update_temp folder
    std::wstring tempFolder = extractPath + L"\\update_temp";
    fs::create_directories(tempFolder);

    // Use PowerShell Expand-Archive
    std::wstring command = L"powershell -NoProfile -ExecutionPolicy Bypass -Command \"& {Try {Expand-Archive -Force -LiteralPath '"
        + zipPath + L"' -DestinationPath '" + tempFolder + L"' -ErrorAction Stop} Catch {exit 1}}\"";

    DBGPRINT("[Update] Extract command: %S\n", command.c_str());

    STARTUPINFOW si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    wchar_t* cmd = _wcsdup(command.c_str());
    BOOL success = CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    free(cmd);

    if (!success)
    {
        DBGPRINT("[Update] CreateProcessW failed, error: %lu\n", GetLastError());
        return false;
    }

    WaitForSingleObject(pi.hProcess, 60000);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0)
    {
        DBGPRINT("[Update] Extract process exited with code: %lu\n", exitCode);
        return false;
    }

    DBGPRINT("[Update] Extraction successful to: %S\n", tempFolder.c_str());
    return true;
}
{
    if (!IsInternetAvailable())
    {
        DBGPRINT("[Update] No internet connection, skipping update check.\n");
        return;
    }

    checkingForUpdates = true;

    std::string apiUrl = "https://api.github.com/repos/arnofrxdd/MouseShifter/releases/latest";
    std::string jsonResponse = DownloadString(apiUrl);

    if (!jsonResponse.empty())
    {
        Json::Value root;
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false; // optional
        std::string errs;
        std::istringstream s(jsonResponse);

        if (Json::parseFromStream(builder, s, &root, &errs))
        {
            if (root.isMember("tag_name"))
            {
                latestTag = root["tag_name"].asString(); // e.g. "v2.0-update"

                // Convert "v2.0-update" -> "2.0" for numeric comparison
                size_t pos = latestTag.find_first_of("0123456789");
                size_t endPos = latestTag.find_first_of("-", pos);
                std::string numericVer = latestTag.substr(pos, endPos - pos);
                latestVersion = numericVer;

                if (CompareVersions(latestVersion, currentVersion) > 0)
                {
                    updateAvailable = true;
                    DBGPRINT("[Update] New version available: %s (current: %s)\n",
                        latestVersion.c_str(), currentVersion.c_str());
                }
                else
                {
                    updateAvailable = false;
                    DBGPRINT("[Update] No new version (latest: %s, current: %s)\n",
                        latestVersion.c_str(), currentVersion.c_str());
                }
            }
        }
        else
        {
            DBGPRINT("[Update] Failed to parse GitHub API JSON: %s\n", errs.c_str());
        }
    }

    checkingForUpdates = false;
}
{
    if (latestTag.empty())
    {
        DBGPRINT("[Update] No latest tag info available.\n");
        return;
    }

    // Generate dynamic download URL from tag
    std::string zipUrl = "https://github.com/arnofrxdd/MouseShifter/releases/download/" + latestTag + "/MouseShifter.zip";
    std::wstring tempZip = L"MouseShifter_update.zip";
    std::wstring exePath, updaterPath;

    wchar_t path[MAX_PATH];
    if (GetModuleFileNameW(NULL, path, MAX_PATH) == 0)
    {
        DBGPRINT("[Update] GetModuleFileNameW failed, error: %lu\n", GetLastError());
        return;
    }

    exePath = path;
    std::wstring exeDir = exePath.substr(0, exePath.find_last_of(L"\\/"));
    updaterPath = exeDir + L"\\updater.bat";
    std::wstring tempZipPath = exeDir + L"\\" + tempZip;

    DBGPRINT("[Update] Downloading zip from: %s\n", zipUrl.c_str());
    DBGPRINT("[Update] Temp zip path: %S\n", tempZipPath.c_str());

    if (!DownloadFile(zipUrl, tempZipPath))
    {
        DBGPRINT("[Update] DownloadFile failed, error: %lu\n", GetLastError());
        MessageBoxA(NULL, "Failed to download the update. Check your internet connection.", "Update Error", MB_OK | MB_ICONERROR);
        return;
    }

    DBGPRINT("[Update] Download successful: %S\n", tempZipPath.c_str());

    if (!ExtractZipNew(tempZipPath, exeDir))
    {
        DBGPRINT("[Update] ExtractZipNew failed\n");
        return;
    }
    // Create updater batch that force-closes old instance, replaces files, and restarts
    HANDLE hBat = CreateFileW(updaterPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hBat == INVALID_HANDLE_VALUE)
    {
        DBGPRINT("[Update] Failed to create updater batch: %S, error: %lu\n", updaterPath.c_str(), GetLastError());
        return;
    }

    std::string batContent =
        "@echo off\n"
        "echo Updating MouseShifter...\n"
        "timeout /t 1 /nobreak >nul\n"
        "setlocal enabledelayedexpansion\n"
        "REM Force close old MouseShifter.exe if running\n"
        "taskkill /IM MouseShifter.exe /F >nul 2>&1\n"
        "REM Move updated files except configs\n"
        "for %%f in (\"" + std::string(exeDir.begin(), exeDir.end()) + "\\update_temp\\*\") do (\n"
        "  set \"fname=%%~nxf\"\n"
        "  if /i not \"!fname!\"==\"config.ini\" if /i not \"!fname!\"==\"gearlayouts.ini\" (\n"
        "    move /Y \"%%f\" \"" + std::string(exeDir.begin(), exeDir.end()) + "\\\" >nul\n"
        "  )\n"
        ")\n"
        "rd /s /q \"" + std::string(exeDir.begin(), exeDir.end()) + "\\update_temp\" >nul\n"
        "del \"" + std::string(tempZipPath.begin(), tempZipPath.end()) + "\" >nul 2>&1\n"
        "REM Start new MouseShifter.exe\n"
        "start \"\" \"" + std::string(exePath.begin(), exePath.end()) + "\"\n"
        "del \"%~f0\" >nul 2>&1\n";

    DWORD written;
    WriteFile(hBat, batContent.c_str(), batContent.length(), &written, NULL);
    CloseHandle(hBat);

    DBGPRINT("[Update] Updater batch created: %S\n", updaterPath.c_str());

    // Launch updater batch
    SHELLEXECUTEINFOW sei{ sizeof(sei) };
    sei.lpFile = updaterPath.c_str();
    sei.nShow = SW_HIDE;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    if (!ShellExecuteExW(&sei))
    {
        DBGPRINT("[Update] Failed to launch updater batch, error: %lu\n", GetLastError());
        return;
    }

    DBGPRINT("[Update] Updater launched, quitting app.\n");
    Sleep(500);
    PostQuitMessage(0);
}






// ETS2 AND ATS EDITOR

namespace fs = std::filesystem;

// Get Documents folder path
{
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path)))
    {
        fs::path docPath(path);
        CoTaskMemFree(path);
        return docPath;
    }
    return ""; // fallback
}

// Universal game paths: ATS + ETS2
std::vector<fs::path> gamePaths = {
    getDocumentsFolder() / "American Truck Simulator",
    getDocumentsFolder() / "Euro Truck Simulator 2" };

// Lines to find & replace
std::vector<std::wstring> targetLines = {
    L"mix camuilr", L"mix camuiud", L"mix camlr", L"mix camud" };
std::vector<std::wstring> replacementLines = {
    L"mix camuilr `-mouse.rel_position.x?0 * c_msens* mouse.button_right?0`",
    L"mix camuiud `-mouse.rel_position.y?0 * sel(c_minvert, -c_msens, c_msens)* mouse.button_right?0`",
    L"mix camlr `-mouse.rel_position.x?0 * c_msens * mouse.button_right?0`",
    L"mix camud `-mouse.rel_position.y?0 * sel(c_minvert, -c_msens, c_msens) * mouse.button_right?0`" };

// Backup & edit function
{
    fs::path backupPath = filePath;
    backupPath += L"_backup_controls.sii";

    // --- Only create backup if it doesn't exist ---
    if (!fs::exists(backupPath))
    {
        std::wifstream inFile(filePath);
        if (!inFile.is_open())
            return; // Skip if file can't be opened

        std::wofstream backup(backupPath);
        if (!backup.is_open())
            return; // Skip if backup file can't be created

        std::wstring line;
        while (std::getline(inFile, line))
        {
            backup << line << L"\n";
        }
        inFile.close();
        backup.close();
    }

    // --- Read original file into memory for editing ---
    std::wifstream inFile(filePath);
    if (!inFile.is_open())
        return;

    std::vector<std::wstring> lines;
    std::wstring line;
    while (std::getline(inFile, line))
    {
        lines.push_back(line);
    }
    inFile.close();

    // --- Edit target lines ---
    for (size_t i = 0; i < lines.size(); i++)
    {
        for (size_t j = 0; j < targetLines.size(); j++)
        {
            if (lines[i].find(targetLines[j]) != std::wstring::npos)
            {
                size_t quotePos = lines[i].find(L'"');
                if (quotePos != std::wstring::npos)
                {
                    lines[i] = lines[i].substr(0, quotePos + 1) + replacementLines[j] + L'"';
                }
            }
        }
    }

    // --- Write edited file ---
    std::wofstream outFile(filePath);
    if (!outFile.is_open())
        return;
    for (auto& l : lines)
        outFile << l << L"\n";
}

// Revert function
{
    std::wifstream backup(filePath.wstring() + L"_backup_controls.sii");
    if (!backup.is_open())
        return; // Skip if backup doesn't exist

    std::wifstream inFile(filePath);
    if (!inFile.is_open())
        return; // Skip if original file doesn't exist

    std::vector<std::wstring> backupLines, lines;
    std::wstring line;

    while (std::getline(backup, line))
        backupLines.push_back(line);
    while (std::getline(inFile, line))
        lines.push_back(line);
    backup.close();
    inFile.close();

    for (size_t i = 0; i < lines.size(); i++)
    {
        for (size_t j = 0; j < targetLines.size(); j++)
        {
            if (lines[i].find(targetLines[j]) != std::wstring::npos)
            {
                size_t quotePos = lines[i].find(L'"');
                if (quotePos != std::wstring::npos)
                {
                    size_t endQuotePos = backupLines[i].rfind(L'"');
                    if (endQuotePos != std::wstring::npos && endQuotePos > quotePos)
                    {
                        lines[i] = lines[i].substr(0, quotePos + 1) +
                            backupLines[i].substr(quotePos + 1, endQuotePos - quotePos - 1) +
                            L'"';
                    }
                }
            }
        }
    }

    std::wofstream outFile(filePath);
    if (!outFile.is_open())
        return; // Skip if can't write
    for (auto& l : lines)
        outFile << l << L"\n";
}

// Process all files for both ATS and ETS2
{
    std::vector<fs::path> folders = { L"profiles", L"steam_profiles" };

    for (auto& gamePath : gamePaths)
    {
        if (!fs::exists(gamePath))
            continue; // skip if game folder doesn't exist

        for (auto& folder : folders)
        {
            fs::path folderPath = gamePath / folder;
            if (!fs::exists(folderPath))
                continue; // skip if subfolder doesn't exist

            for (auto& p : fs::recursive_directory_iterator(folderPath))
            {
                if (p.path().filename() == L"controls.sii")
                {
                    if (edit)
                        backupAndEdit(p.path());
                    else
                        revertFromBackup(p.path());
                }
            }
        }
    }
}
{
    if (!vJoyEnabled())
    {
        OutputDebugString(L"[vJoy] Driver not enabled!\n");
        MessageBox(nullptr, L"vJoy is not installed! Downloading now...", L"Error", MB_OK);

        // --- Download vJoy installer ---
        const wchar_t* url = L"https://cyfuture.dl.sourceforge.net/project/vjoystick/Beta%202.x/2.1.9.1-160719/vJoySetup.exe?viasf=1";
        const wchar_t* savePath = L"vJoy_Setup.exe";

        HRESULT hr = URLDownloadToFile(nullptr, url, savePath, 0, nullptr);
        if (SUCCEEDED(hr))
        {
            MessageBox(nullptr, L"vJoy downloaded! Please run the installer manually.", L"Download Complete", MB_OK);
            ShellExecute(nullptr, L"open", savePath, nullptr, nullptr, SW_SHOWNORMAL);
        }
        else
        {
            MessageBox(nullptr, L"Failed to download vJoy! Please download it from the official site.", L"Error", MB_OK | MB_ICONERROR);
        }

        return false;
    }

    VjdStat status = GetVJDStatus(vjoyDeviceId);

    switch (status)
    {
    case VJD_STAT_OWN:
        OutputDebugString(L"[vJoy] Device already owned.\n");
        break;
    case VJD_STAT_FREE:
        OutputDebugString(L"[vJoy] Device is free.\n");
        break;
    case VJD_STAT_BUSY:
        OutputDebugString(L"[vJoy] Device busy.\n");
        break;
    case VJD_STAT_MISS:
        OutputDebugString(L"[vJoy] Device missing.\n");
        break;
    case VJD_STAT_UNKN:
    default:
        OutputDebugString(L"[vJoy] Device status unknown.\n");
        break;
    }

    if (status != VJD_STAT_OWN && status != VJD_STAT_FREE)
    {
        MessageBox(nullptr, L"vJoy device not ready!", L"Error", MB_OK);
        return false;
    }

    // --- RELEASE ALL BUTTONS BEFORE STARTING ---
    vJoyButtonCount = GetVJDButtonNumber(vjoyDeviceId); // get number of buttons
    for (int btn = 1; btn <= vJoyButtonCount; ++btn)
    {
        SetBtn(FALSE, vjoyDeviceId, btn); // Release button
    }
    OutputDebugString(L"[vJoy] All buttons released before starting.\n");

    // Now acquire the device
    BOOL ok = AcquireVJD(vjoyDeviceId);

    // Get axis min/max
    GetVJDAxisMin(vjoyDeviceId, HID_USAGE_X, &axisMin);
    GetVJDAxisMax(vjoyDeviceId, HID_USAGE_X, &axisMax);

    // Center joystick axes
    joyX = joyY = (axisMax + axisMin) / 2;
    joyRx = (axisMax + axisMin) / 2;  // center Rx

    // Push initial positions to vJoy
    SetAxis(joyX, vjoyDeviceId, HID_USAGE_X);
    SetAxis(joyY, vjoyDeviceId, HID_USAGE_Y);
    SetAxis(joyRx, vjoyDeviceId, HID_USAGE_RX);


    if (ok)
    {
        OutputDebugString(L"[vJoy] Device acquired successfully.\n");

        // --- Store button count for later use ---
        char buf[128];
        sprintf_s(buf, "vJoyButtonCount=%d\n", vJoyButtonCount);
        OutputDebugStringA(buf);
    }
    else
    {
        OutputDebugString(L"[vJoy] Failed to acquire device!\n");
    }

    return ok != FALSE;
}
{
    KEYBOARD,
    MOUSE,
    VJOY_BUTTON
};
{
    InputType type;
    WORD code; // VK code for keyboard or RAW mouse button code
};
InputType heldGearType = KEYBOARD;
{
    HANDLE hDevice;
    std::wstring name;
    bool isTouchpad = false; // new
};

std::vector<MouseDevice> g_mouseDevices;
HANDLE g_selectedDevice = NULL; // NULL = all mice
RECT deviceComboRect;
HANDLE g_selectedSteeringDevice = NULL; // NULL means "All mice"
RECT steeringDeviceComboRect;           // Add this declaration near other RECT variables

std::map<std::string, GearInput> gearInputMap = {
    {"1", {VJOY_BUTTON, 1}},
    {"2", {VJOY_BUTTON, 2}},
    {"3", {VJOY_BUTTON, 3}},
    {"4", {VJOY_BUTTON, 4}},
    {"5", {VJOY_BUTTON, 5}},
    {"6", {VJOY_BUTTON, 6}},
    {"7", {VJOY_BUTTON, 7}},
    {"8", {VJOY_BUTTON, 8}},
    {"R", {VJOY_BUTTON, 9}},
    {"N", {VJOY_BUTTON, 10}} };

// For the new input panel
bool showInputPanel = true; // toggle panel visibility
RECT inputPanelRect;        // use RECT
int rowHeight = 28;             // row height in panel

// For drawing and picking vJoy
std::string vJoyPickerInput; // currently selected input for vJoy
{
    InputType type;
    WORD code;       // VK code for keyboard, RAW mouse code for mouse
    UINT vjoyButton; // vJoy button to activate
};
std::vector<InputToVJoy> inputMap = {
    //{KEYBOARD, 'A', 1},       // Press 'A' → vJoy button 1
    //{KEYBOARD, 'S', 2},       // Press 'S' → vJoy button 2
    {MOUSE, 1, 11}, // Left mouse button → vJoy button 3
    {MOUSE, 2, 12}  // Right mouse button → vJoy button 4
};
// Profile management
std::vector<std::string> profileNames;
int currentProfileIndex = 0;
bool profileDropdownOpen = false;
bool creatingNewProfile = false;
std::string newProfileName = "New Profile";
RECT profileButtonRect;
RECT createProfileButtonRect; // Add this with your other profile variables
// Profile text input
bool profileTextSelected = false;
int profileTextSelectionStart = 0;
int profileTextSelectionEnd = 0;
{
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string exePath(path);
    size_t pos = exePath.find_last_of("\\/");
    return exePath.substr(0, pos); // folder path
}

std::string configFile = GetExeFolder() + "\\config.ini";
const char* configSection = "Config";
{
    int nAccentState;
    int nFlags;
    int nColor;
    int nAnimationId;
};
{
    int nAttribute;
    PVOID pData;
    SIZE_T ulDataSize;
};
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
};
{
    ACCENT_POLICY policy = {};
    policy.nAccentState = acrylic ? ACCENT_ENABLE_ACRYLICBLURBEHIND : ACCENT_ENABLE_BLURBEHIND;
    policy.nFlags = 0;
    policy.nColor = 0xCCFFFFFF;
    policy.nAnimationId = 0;

    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.nAttribute = 19; // WCA_ACCENT_POLICY
    data.pData = &policy;
    data.ulDataSize = sizeof(policy);

    typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
    static pSetWindowCompositionAttribute SetWindowCompositionAttribute =
        (pSetWindowCompositionAttribute)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowCompositionAttribute");

    if (SetWindowCompositionAttribute)
        return SetWindowCompositionAttribute(hwnd, &data);

    return false;
}
{
    BOOL enabled = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(nullptr, &enabled)))
    {
        // This returns TRUE if colorization is active, but better:
    }

    DWORD value = 0;
    DWORD size = sizeof(DWORD);
    if (RegGetValue(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"EnableTransparency",
        RRF_RT_REG_DWORD, nullptr, &value, &size) == ERROR_SUCCESS)
    {
        return value != 0;
    }

    return false; // assume disabled if registry query fails
}
{
    const int steps = 20; // smoothness
    const int delay = durationMs / steps;

    for (int i = 0; i <= steps; ++i)
    {
        float t = i / (float)steps;
        BYTE alpha = (BYTE)(startAlpha + t * (endAlpha - startAlpha));
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), alpha, LWA_COLORKEY | LWA_ALPHA);
        Sleep(delay); // simple blocking; for non-blocking, use a timer
    }
}
// --- Transparency controls ---
bool dynamicTransparencyEnabled = true; // toggle for dynamic transparency
BYTE currentAlpha = 200;                // current alpha applied to window
BYTE minAlpha = 20;                     // idle transparency
const float alphaDecay = 0.66f; // approximately the same fade speed at 15 FPS
DWORD transparencyFadeDelay = 300; // milliseconds before transparency starts
RECT transparencyFadeDelaySliderRect;
bool draggingTransparencyFadeDelaySlider = false;
bool assistButtonHeld = false;
// Add with your other global variables
// --- UI state for settings panel ---
RECT dynamicTransparencyToggleRect;
RECT transparencySliderRect;
bool draggingTransparencySlider = false;
RECT accBrakeSensSliderRect;
RECT steeringSensSliderRect;
RECT minTransparencySliderRect;    // slider rect for idle transparency
bool draggingMinTransparencySlider = false; // dragging state
RECT steeringDegreesSliderRect;
bool draggingSteeringDegreesSlider = false; // for WM_MOUSEMOVE drag
bool draggingYBarAlphaSlider = false;

float maxSteeringDegrees = 900.0f; // user can set this in your UI
float fullWheelDegrees = 900.0f;   // full rotation your virtual wheel represents
// Add this with your other global variables
int yBarAlpha = 180; // Fixed alpha for Y-bar (0-255)
bool useYbarFixedTransparency = false; // Enable separate Y-bar window with fixed transparency
RECT yBarFixedTransToggle;
RECT yBarAlphaSlider;
HWND g_yBarHwnd = nullptr;
// Add with other global variables
extern HWND g_yBarHwnd;
extern int yBarAlpha;

// Add with other function declarations
void CreateYBarWindow(HWND parentHwnd);
void DestroyYBarWindow();
void UpdateYBarWindowPosition(HWND parentHwnd);
LRESULT CALLBACK YBarWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void DrawYBarOnly(HDC hdc, int width, int height);
{
    if (g_yBarHwnd) return;

    WNDCLASS wc = {};
    wc.lpfnWndProc = YBarWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"YBarWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);

    RECT parentRect;
    GetWindowRect(parentHwnd, &parentRect);

    g_yBarHwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"YBarWindow",
        L"YBar",
        WS_POPUP,
        parentRect.left, parentRect.top,
        parentRect.right - parentRect.left, parentRect.bottom - parentRect.top,
        parentHwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );

    SetLayeredWindowAttributes(g_yBarHwnd, RGB(0, 0, 0), yBarAlpha, LWA_COLORKEY | LWA_ALPHA);
    ShowWindow(g_yBarHwnd, SW_SHOW);
}
{
    if (g_yBarHwnd)
    {
        DestroyWindow(g_yBarHwnd);
        g_yBarHwnd = nullptr;
    }
}
{
    if (!g_yBarHwnd) return;

    RECT parentRect;
    GetWindowRect(parentHwnd, &parentRect);
    SetWindowPos(g_yBarHwnd, nullptr,
        parentRect.left, parentRect.top,
        parentRect.right - parentRect.left, parentRect.bottom - parentRect.top,
        SWP_NOZORDER | SWP_NOACTIVATE);
}
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int width = rc.right;
        int height = rc.bottom;

        // Double buffering for Y-bar window
        HDC memDC = CreateCompatibleDC(hdc);
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = nullptr;
        HBITMAP memBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        // Clear to transparent
        Gdiplus::Graphics graphics(memDC);
        graphics.Clear(Gdiplus::Color(0, 0, 0, 0));

        // Draw only Y-bar
        DrawYBarOnly(memDC, width, height);

        // Blit to screen
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        // Cleanup
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1; // Prevent background erasure

    case WM_DESTROY:
        g_yBarHwnd = nullptr;
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
{
    isBorderless = !isBorderless;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (isBorderless)
    {
        // Remove window borders
        style &= ~(WS_OVERLAPPEDWINDOW);
        SetWindowLong(hwnd, GWL_STYLE, style);

        // Make window layered for transparency
        exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

        // Set topmost only when borderless
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        // Disable blur/acrylic explicitly
        ACCENT_POLICY policy = {};
        policy.nAccentState = ACCENT_DISABLED;
        WINDOWCOMPOSITIONATTRIBDATA data = {};
        data.nAttribute = 19; // WCA_ACCENT_POLICY
        data.pData = &policy;
        data.ulDataSize = sizeof(policy);

        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
        static pSetWindowCompositionAttribute SetWindowCompositionAttribute =
            (pSetWindowCompositionAttribute)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowCompositionAttribute");

        if (SetWindowCompositionAttribute)
            SetWindowCompositionAttribute(hwnd, &data);

        currentAlpha = maxAlpha; // use slider-controlled max alpha
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_COLORKEY | LWA_ALPHA);

        isTransparent = true;

        // Create Y-bar window with separate alpha
        if (useYbarFixedTransparency)
        {
            CreateYBarWindow(hwnd);
        }


        // Hide cursor
        ShowCursor(FALSE);

        // Save and hide panels
        prevShowInputPanel = showInputPanel;
        prevShowTogglePanel = showTogglePanel;
        prevShowSettingsPanel = showSettingsPanel;
        prevShowKeybindPanel = showKeybindPanel;
        showSettingsPanel = false;
        showKeybindPanel = false;
        showInputPanel = false;
        showTogglePanel = false;
        // Lock cursor inside window
        ClipCursor(nullptr);
    }
    else
    {
        // Destroy Y-bar window
        if (useYbarFixedTransparency)
        {
            DestroyYBarWindow();
        }

        // Restore standard window style
        style &= ~WS_CAPTION;
        style &= ~WS_THICKFRAME;
        exStyle &= ~WS_EX_LAYERED;
        SetWindowLong(hwnd, GWL_STYLE, style);
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

        // Remove topmost when exiting borderless
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        // Show cursor
        ShowCursor(TRUE);
        ClipCursor(nullptr);

        // Restore panel visibility
        showSettingsPanel = prevShowSettingsPanel;
        showKeybindPanel = prevShowKeybindPanel;
        showInputPanel = prevShowInputPanel;
        showTogglePanel = prevShowTogglePanel;
        // Apply blur in windowed mode
        EnableWin11Blur(hwnd);
    }

    // Refresh window to apply style changes
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}
{
    if (gearInputMap.find(gear) == gearInputMap.end())
        return;

    GearInput gi = gearInputMap[gear];

    // Release previous key/button
    if (heldGearKey != 0 && heldGearKey != gi.code)
    {
        if (heldGearType == KEYBOARD)
        {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = heldGearKey;
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }
        else if (heldGearType == MOUSE)
        {
            INPUT input = {};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = (heldGearKey == RI_MOUSE_LEFT_BUTTON_DOWN ? MOUSEEVENTF_LEFTUP : heldGearKey == RI_MOUSE_RIGHT_BUTTON_DOWN ? MOUSEEVENTF_RIGHTUP
                : MOUSEEVENTF_MIDDLEUP);
            SendInput(1, &input, sizeof(INPUT));
        }
        else if (heldGearType == VJOY_BUTTON)
        {
            SetBtn(false, vjoyDeviceId, heldGearKey); // release button
        }
        heldGearKey = 0;
    }

    // Press new key/button
    if (gi.type == KEYBOARD)
    {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = gi.code;
        SendInput(1, &input, sizeof(INPUT));
    }
    else if (gi.type == MOUSE)
    {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = (gi.code == RI_MOUSE_LEFT_BUTTON_DOWN ? MOUSEEVENTF_LEFTDOWN : gi.code == RI_MOUSE_RIGHT_BUTTON_DOWN ? MOUSEEVENTF_RIGHTDOWN
            : MOUSEEVENTF_MIDDLEDOWN);
        SendInput(1, &input, sizeof(INPUT));
    }
    else if (gi.type == VJOY_BUTTON)
    {
        SetBtn(true, vjoyDeviceId, gi.code); // press vJoy button
    }

    heldGearKey = gi.code;
    heldGearType = gi.type;

    // === START GLOW ANIMATION FOR THIS GEAR ===
    if (keybindAnimations.find(gear) == keybindAnimations.end()) {
        keybindAnimations[gear] = KeybindAnimation();
    }
    // FIX: Only set isHeld = true, NOT isActive = true
    keybindAnimations[gear].isHeld = true;
    keybindAnimations[gear].isActive = false; // Make sure active is false
    keybindAnimations[gear].glowAlpha = MAX_GLOW_ALPHA; // Immediate full brightness
    keybindAnimations[gear].activationTime = GetTickCount();
}

// Call this on every WM_INPUT update
{
    if (heldGearKey != 0)
    {
        if (heldGearType == KEYBOARD)
        {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = heldGearKey;
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }
        else if (heldGearType == MOUSE)
        {
            INPUT input = {};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = (heldGearKey == RI_MOUSE_LEFT_BUTTON_DOWN ? MOUSEEVENTF_LEFTUP : heldGearKey == RI_MOUSE_RIGHT_BUTTON_DOWN ? MOUSEEVENTF_RIGHTUP
                : MOUSEEVENTF_MIDDLEUP);
            SendInput(1, &input, sizeof(INPUT));
        }
        else if (heldGearType == VJOY_BUTTON)
        {
            SetBtn(false, vjoyDeviceId, heldGearKey); // release button
        }

        // === STOP ALL HELD GLOWS ===
        for (auto& kv : keybindAnimations) {
            if (kv.second.isHeld) {
                kv.second.isHeld = false;
                kv.second.isActive = false; // Also clear active state
                kv.second.activationTime = GetTickCount(); // Start fade out
            }
        }

        heldGearKey = 0;
        heldGearType = KEYBOARD;
    }
}

// ---------------- Raw Input ----------------
{
    RAWINPUTDEVICE rid[2];

    // Mouse
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02;            // Mouse
    rid[0].dwFlags = RIDEV_INPUTSINK; // receive input even when unfocused
    rid[0].hwndTarget = hwnd;

    // Keyboard
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x06;            // Keyboard
    rid[1].dwFlags = RIDEV_INPUTSINK; // receive input even when unfocused
    rid[1].hwndTarget = hwnd;

    RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
    rawInputInitialized = true;

    g_mouseDevices.clear();
    UINT nDevices = 0;
    GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST));
    if (nDevices == 0)
        return;

    std::vector<RAWINPUTDEVICELIST> devs(nDevices);
    GetRawInputDeviceList(devs.data(), &nDevices, sizeof(RAWINPUTDEVICELIST));

    int usbMouseCounter = 1;

    for (auto& d : devs)
    {
        if (d.dwType != RIM_TYPEMOUSE)
            continue;

        UINT size = 0;
        GetRawInputDeviceInfo(d.hDevice, RIDI_DEVICENAME, NULL, &size);
        std::wstring rawName(size, 0);
        GetRawInputDeviceInfo(d.hDevice, RIDI_DEVICENAME, &rawName[0], &size);
        rawName.resize(size - 1);

        // --- Determine if this is a touchpad ---
        bool isTouchpad = (d.hDevice == 0 || rawName.find(L"SYNA") != std::wstring::npos || rawName.find(L"TouchPad") != std::wstring::npos);

        // --- Friendly name formatting ---
        std::wstring friendlyName;
        if (isTouchpad)
        {
            friendlyName = L"Touchpad";
        }
        else
        {
            // Give USB mice sequential numbers
            friendlyName = L"USB Mouse " + std::to_wstring(usbMouseCounter++);
        }

        g_mouseDevices.push_back({ d.hDevice, friendlyName, isTouchpad });

        // --- Debug print ---
        wchar_t buf[256];
        swprintf_s(buf, L"Mouse device handle: %p, formatted name: %s, raw name: %s\n",
            d.hDevice, friendlyName.c_str(), rawName.c_str());
        OutputDebugStringW(buf);
    }
}
// Throttle and brake variables
bool throttleBrakeThreadRunning = false;
HANDLE throttleBrakeThread = NULL;
DWORD throttleBrakeThreadId = 0;
LONG joyZ = 0;
LONG joyRz = 0;
float throttleSensitivity = 8.5f;
float brakeSensitivity = 8.5f;
float releaseSpeed = 8.5f;

// Thread synchronization
CRITICAL_SECTION throttleBrakeCS;
{
    while (throttleBrakeThreadRunning)
    {
        // SKIP if using XInput OR if throttle/brake axes are disabled
        if (useXInput || !useThrottleBrakeAxes)
        {
            Sleep(16);
            continue;
        }

        EnterCriticalSection(&throttleBrakeCS);

        // Throttle control (Z axis) - W key
        bool throttlePressed = (GetAsyncKeyState(0x57) & 0x8000) != 0;

        // Brake control (Rz axis) - S key  
        bool brakePressed = (GetAsyncKeyState(0x53) & 0x8000) != 0;

        // Update throttle (Z axis)
        if (throttlePressed)
        {
            joyZ += (LONG)((axisMax - axisMin) * throttleSensitivity / 100.0f);
            joyZ = min(axisMax, joyZ);
        }
        else
        {
            // Return to 0 when released
            if (joyZ > axisMin)
            {
                joyZ -= (LONG)((axisMax - axisMin) * releaseSpeed / 100.0f);
                joyZ = max(axisMin, joyZ);
            }
        }

        // Update brake (Rz axis)
        if (brakePressed)
        {
            joyRz += (LONG)((axisMax - axisMin) * brakeSensitivity / 100.0f);
            joyRz = min(axisMax, joyRz);
        }
        else
        {
            // Return to 0 when released
            if (joyRz > axisMin)
            {
                joyRz -= (LONG)((axisMax - axisMin) * releaseSpeed / 100.0f);
                joyRz = max(axisMin, joyRz);
            }
        }

        // ALWAYS push to vJoy (no matter what UI is doing)
        SetAxis(joyZ, vjoyDeviceId, HID_USAGE_Z);
        SetAxis(joyRz, vjoyDeviceId, HID_USAGE_RZ);

        LeaveCriticalSection(&throttleBrakeCS);

        // Run at ~60 FPS for smooth input
        Sleep(16);
    }
    return 0;
}
{
    if (throttleBrakeThreadRunning) return;

    InitializeCriticalSection(&throttleBrakeCS);
    throttleBrakeThreadRunning = true;
    throttleBrakeThread = CreateThread(
        NULL,
        0,
        ThrottleBrakeThread,
        NULL,
        0,
        &throttleBrakeThreadId
    );

    if (throttleBrakeThread)
    {
        // Set lower priority so it doesn't interfere with UI
        SetThreadPriority(throttleBrakeThread, THREAD_PRIORITY_BELOW_NORMAL);
    }
}
{
    throttleBrakeThreadRunning = false;

    if (throttleBrakeThread)
    {
        WaitForSingleObject(throttleBrakeThread, 1000);
        CloseHandle(throttleBrakeThread);
        throttleBrakeThread = NULL;
    }

    DeleteCriticalSection(&throttleBrakeCS);
}
{
    EnterCriticalSection(&throttleBrakeCS);
    joyZ = axisMin;
    joyRz = axisMin;
    SetAxis(joyZ, vjoyDeviceId, HID_USAGE_Z);
    SetAxis(joyRz, vjoyDeviceId, HID_USAGE_RZ);
    LeaveCriticalSection(&throttleBrakeCS);
}
{
    // Persistent toggle flags
    static DWORD f9PressedTime = 0; // timestamp when F9 was pressed

    // Track previous key states
    static bool wasF9Pressed = false;

    // Current key states
    bool isF9Pressed = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;

    // ----- F11 toggle -----

    // ----- F9 hold detection -----
    if (isF9Pressed)
    {
        if (!wasF9Pressed)
            f9PressedTime = GetTickCount(); // record when F9 was first pressed

        // check if held ≥ 3 seconds (3000 ms)
        if (GetTickCount() - f9PressedTime >= 3000)
        {
            // reset axes to center
            LONG defaultX = (axisMin + axisMax) / 2;
            LONG defaultY = (axisMin + axisMax) / 2;

            joyX = defaultX;
            joyY = defaultY;

            SetAxis(joyX, vjoyDeviceId, HID_USAGE_X);
            SetAxis(joyY, vjoyDeviceId, HID_USAGE_Y);
        }
    }
    wasF9Pressed = isF9Pressed;
    // ----- Scroll → Rx axis test -----



    // ----- Mouse steering disabled -----
// Respect both global steering AND knob override
    if (!mouseSteeringEnabled)
    {
        LONG defaultX = (axisMin + axisMax) / 2;
        LONG defaultY = (axisMin + axisMax) / 2;

        joyX = defaultX;
        joyY = defaultY;

        SetAxis(joyX, vjoyDeviceId, HID_USAGE_X);
        SetAxis(joyY, vjoyDeviceId, HID_USAGE_Y);
        return;
    }

    // ----- Mouse tracking temporarily disabled (knob toggle) -----
    if (!mouseTrackingEnabled)
    {
        // Do nothing (keep last joyX/joyY)
        return;
    }


    // ----- vJoy mouse disabled (F11) -----
    if (!vJoyMouseEnabled)
        return; // stop processing but do not reset axes

    // ----- Process selected device -----
    bool process = (g_selectedSteeringDevice == NULL || g_selectedSteeringDevice == hDevice);
    if (!process)
        return;

    float initialBoost = 8.0f; // 8× boost

    // Define center and max steering offset
    LONG centerX = (axisMax + axisMin) / 2;
    LONG axisRange = axisMax - axisMin;
    LONG steeringMaxOffset = (LONG)((maxSteeringDegrees / fullWheelDegrees) * (axisRange / 2));

    // ----- INTERDEPENDENT AXIS SMOOTHING -----


    float finalSteeringSensitivity = steeringSensitivity * initialBoost;
    float finalAccBrakeSensitivity = accBrakeSensitivity * initialBoost;

    // Apply interdependent smoothing if enabled
    if (useAxisSmoothing && axisSmoothingFactor > 0.0f)
    {
        // Calculate movement magnitude for both axes
        float xMovement = fabs((float)rm.lLastX * steeringSensitivity * initialBoost);
        float yMovement = fabs((float)rm.lLastY * accBrakeSensitivity * initialBoost);

        // Normalize movement to 0-1 range
        float maxPossibleMovement = 50.0f; // Adjust based on your typical max delta
        float xMoveFactor = min(xMovement / maxPossibleMovement, 1.0f);
        float yMoveFactor = min(yMovement / maxPossibleMovement, 1.0f);

        // Apply interdependent sensitivity reduction using single factor
        finalSteeringSensitivity *= (1.0f - (yMoveFactor * axisSmoothingFactor));
        finalAccBrakeSensitivity *= (1.0f - (xMoveFactor * axisSmoothingFactor));
    }

    // Compute new positions with adjusted sensitivities
    LONG newJoyX = joyX + (LONG)(rm.lLastX * finalSteeringSensitivity);
    LONG newJoyY = joyY + (LONG)(rm.lLastY * finalAccBrakeSensitivity);

    // ----- BRAKE RESISTANCE FACTOR -----
    if (brakeresistanceFactor > 0.0f && rm.lLastY > 0) // Only apply resistance when moving downward (-Y direction, brakes)
    {
        // Calculate how far we are into the brake range (0.0 to 1.0)
        // Center is where brakes start, axisMax is full brakes
        LONG centerY = (axisMin + axisMax) / 2;
        float brakePosition = (float)(newJoyY - centerY) / (float)(axisMax - centerY);

        // Clamp brake position between 0 and 1
        brakePosition = max(0.0f, min(1.0f, brakePosition));

        // Apply resistance: higher resistance as brake position increases
        float resistance = brakeresistanceFactor * brakePosition;
        float resistanceMultiplier = 1.0f / (1.0f + resistance);

        // Only apply resistance to downward movement
        LONG actualMovement = newJoyY - joyY;
        if (actualMovement > 0) // Moving downward
        {
            LONG resistedMovement = (LONG)(actualMovement * resistanceMultiplier);
            newJoyY = joyY + resistedMovement;
        }
    }

    // ----- ACCELERATION RESISTANCE FACTOR -----
    if (accelerationResistanceFactor > 0.0f && rm.lLastY < 0) // Only apply resistance when moving upward (+Y direction, acceleration)
    {
        // Calculate how far we are into the acceleration range (0.0 to 1.0)
        // Center is where acceleration starts, axisMin is full acceleration
        LONG centerY = (axisMin + axisMax) / 2;
        float accelerationPosition = (float)(centerY - newJoyY) / (float)(centerY - axisMin);

        // Clamp acceleration position between 0 and 1
        accelerationPosition = max(0.0f, min(1.0f, accelerationPosition));

        // Apply resistance: higher resistance as acceleration position increases
        float resistance = accelerationResistanceFactor * accelerationPosition;
        float resistanceMultiplier = 1.0f / (1.0f + resistance);

        // Only apply resistance to upward movement
        LONG actualMovement = joyY - newJoyY; // Positive when moving upward
        if (actualMovement > 0) // Moving upward
        {
            LONG resistedMovement = (LONG)(actualMovement * resistanceMultiplier);
            newJoyY = joyY - resistedMovement;
        }
    }

    // Clamp X to steering limit (hard lock)
    joyX = max(centerX - steeringMaxOffset, min(centerX + steeringMaxOffset, newJoyX));

    // Clamp Y to axis range
    joyY = max(axisMin, min(axisMax, newJoyY));

    // Push to vJoy
    SetAxis(joyX, vjoyDeviceId, HID_USAGE_X);
    SetAxis(joyY, vjoyDeviceId, HID_USAGE_Y);
}
// Add these global variables at the top of your file
static LONG targetJoyRx = (axisMin + axisMax) / 2;  // Start at center

// New function to handle scroll input
{
    if (useScrollClutch && (rm.usButtonFlags & RI_MOUSE_WHEEL))
    {
        SHORT wheelDelta = (SHORT)rm.usButtonData; // +120 / -120

        float scrollBoost = 2.0f;
        float step = scrollClutchSens * ((float)(axisMax - axisMin) / 100.0f) * scrollBoost;

        // Update target position
        if (invertScrollClutchAxis)
            targetJoyRx -= (LONG)((wheelDelta / 120.0f) * step);
        else
            targetJoyRx += (LONG)((wheelDelta / 120.0f) * step);

        // ---- HALF-AXIS MODE ----
        if (useHalfClutch)
        {
            LONG mid = (axisMin + axisMax) / 2;
            targetJoyRx = max(mid, min(axisMax, targetJoyRx));
        }

        // Clamp target
        targetJoyRx = max(axisMin, min(axisMax, targetJoyRx));
    }
}

// New function to update smooth scroll (call from WM_TIMER)
{
    if (joyRx != targetJoyRx)
    {
        // Smooth interpolation
        joyRx = joyRx + (LONG)((targetJoyRx - joyRx) / smoothScrollSpeed);

        // Snap to target when very close
        if (abs(targetJoyRx - joyRx) < 5)
            joyRx = targetJoyRx;

        // Clamp final
        joyRx = max(axisMin, min(axisMax, joyRx));
        SetAxis(joyRx, vjoyDeviceId, HID_USAGE_RX);
    }
}
bool noReverseLayout = false;
// Layout types - use one of these
// Define gear layout types

std::vector<GearLayout> hShifterLayouts = {
    {1, L"6/8-Gear + High R Top-Left"},
    {2, L"6/8-Gear + High No Reverse"},
    {3, L"6/8-Gear + High R Bottom-Left"},
    {4, L"6/8-Gear + High R Bottom-Right"},
    {5, L"5-Gear R Bottom-Right"},
    {6, L"5-Gear R Top-Left"},
    {7, L"4-Gear R Top-Left Only"},
    {8, L"4-Gear R Bottom-Left Only"},
    {9, L"4-Gear Mixed (R Top-Left)"},
    {10, L"6/8-Gear + High R Top-Right"},
    {11, L"PRNDL (Automatic Transmission)"}
};
int currentHShifterLayout = 1; // Default to Normal Layout
int layoutType = 1; // Default to Normal Layout
bool hShifterLayoutDropdownOpen = false;
RECT hShifterLayoutButtonRect;
int hoveredGearLayoutIndex = -1;
int hoveredHShifterLayoutIndex = -1;
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    centerX = (rc.right - rc.left) / 2;
    centerY = (rc.bottom - rc.top) / 2;

    int* offsets;
    if (layoutType == 5 || layoutType == 6 || layoutType == 7 || layoutType == 8 || layoutType == 9) {
        // Force 12-gear offsets for layouts with less than 6 gears
        offsets = railOffsets12;
    }
    else {
        // Use normal selection for other layouts
        offsets = is16GearSet ? railOffsets16 : railOffsets12;
    }
    int railCount = 0;

    // Determine rail count based on layout type
    switch (layoutType)
    {
    case 1: // Normal Layout
    case 3: // Reverse Bottom Layout (First rail)
    case 4: // Reverse Bottom Layout (Last rail)
    case 10: // Reverse Top Last Layout
        railCount = is16GearSet ? 5 : 4;
        break;
    case 2: // No Reverse Layout
        railCount = is16GearSet ? 4 : 3;
        break;
    case 5: // 5-Gear Only Layout
    case 6: // 5-Gear Reverse First Layout
    case 7: // 4-Gear Reverse Top Layout
    case 8: // 4-Gear Reverse Bottom Layout  
    case 9: // 4-Gear Reverse Mixed Layout
        railCount = 3; // Always 3 rails for these layouts
        break;
    case 11: // PRNDL Layout
        railCount = 1; // Only one vertical rail for PRNDL
        break;
    }

    // --- Compute center offset if No Reverse Layout to keep rails centered ---

    int centerOffset = 0;
    if (layoutType == 2 || layoutType == 5 || layoutType == 6 || layoutType == 7 || layoutType == 8 || layoutType == 9 || layoutType == 11)
    {
        // Use appropriate full rail count based on layout type
        int fullRailCount;
        if (layoutType == 5 || layoutType == 6 || layoutType == 7 || layoutType == 8 || layoutType == 9) {
            fullRailCount = 4; // 12-gear full layout for <6 gear layouts
        }
        else {
            fullRailCount = is16GearSet ? 5 : 4; // Normal logic for other layouts
        }

        int usedRailCount = railCount;                                 // rails we actually use
        int fullWidth = offsets[fullRailCount - 1] - offsets[0];       // width of full layout
        int usedWidth = offsets[usedRailCount - 1] - offsets[0];       // width of used rails
        centerOffset = int((fullWidth - usedWidth) / 2 * layoutScale); // shift rails to center
    }

    // --- Assign rail positions ---
    for (int i = 0; i < railCount; ++i)
    {
        railX[i].x = centerX + int(offsets[i] * layoutScale) + centerOffset;
        railX[i].y = centerY;
    }

    // --- Adjust vertical range based on layout type ---
    if (layoutType == 11) // PRNDL Layout - larger vertical range
    {
        topY = centerY - int(140 * layoutScale);    // Increased from 60 to 80
        bottomY = centerY + int(140 * layoutScale); // Increased from 60 to 80
    }
    else // All other layouts
    {
        topY = centerY - int(60 * layoutScale);
        bottomY = centerY + int(60 * layoutScale);
    }

    knobMinX = railX[0].x;
    knobMaxX = railX[railCount - 1].x;
    knobMinY = topY;
    knobMaxY = bottomY;

    lowerGearPositions.clear();
    highGearPositions.clear();

    // --- Layout 1: Normal Layout ---
    if (layoutType == 1)
    {
        if (!is16GearSet)
        {
            // 12-gear normal layout
            lowerGearPositions["R"] = { railX[0].x, topY };
            lowerGearPositions["1"] = { railX[1].x, topY };
            lowerGearPositions["2"] = { railX[1].x, bottomY };
            lowerGearPositions["3"] = { railX[2].x, topY };
            lowerGearPositions["4"] = { railX[2].x, bottomY };
            lowerGearPositions["5"] = { railX[3].x, topY };
            lowerGearPositions["6"] = { railX[3].x, bottomY };

            highGearPositions["R"] = { railX[0].x, topY };
            highGearPositions["7"] = { railX[1].x, topY };
            highGearPositions["8"] = { railX[1].x, bottomY };
            highGearPositions["9"] = { railX[2].x, topY };
            highGearPositions["10"] = { railX[2].x, bottomY };
            highGearPositions["11"] = { railX[3].x, topY };
            highGearPositions["12"] = { railX[3].x, bottomY };
        }
        else
        {
            // 16-gear normal layout
            lowerGearPositions["R"] = { railX[0].x, topY };
            lowerGearPositions["1"] = { railX[1].x, topY };
            lowerGearPositions["2"] = { railX[1].x, bottomY };
            lowerGearPositions["3"] = { railX[2].x, topY };
            lowerGearPositions["4"] = { railX[2].x, bottomY };
            lowerGearPositions["5"] = { railX[3].x, topY };
            lowerGearPositions["6"] = { railX[3].x, bottomY };
            lowerGearPositions["7"] = { railX[4].x, topY };
            lowerGearPositions["8"] = { railX[4].x, bottomY };

            highGearPositions["R"] = { railX[0].x, topY };
            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
            highGearPositions["13"] = { railX[3].x, topY };
            highGearPositions["14"] = { railX[3].x, bottomY };
            highGearPositions["15"] = { railX[4].x, topY };
            highGearPositions["16"] = { railX[4].x, bottomY };
        }
    }
    // --- Layout 2: No Reverse Layout ---
    else if (layoutType == 2)
    {
        if (!is16GearSet)
        {
            // 12-gear, 3 vertical rails (no reverse)
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };

            highGearPositions["7"] = { railX[0].x, topY };
            highGearPositions["8"] = { railX[0].x, bottomY };
            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
        }
        else
        {
            // 16-gear, 4 vertical rails (no reverse)
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["7"] = { railX[3].x, topY };
            lowerGearPositions["8"] = { railX[3].x, bottomY };

            highGearPositions["9"] = { railX[0].x, topY };
            highGearPositions["10"] = { railX[0].x, bottomY };
            highGearPositions["11"] = { railX[1].x, topY };
            highGearPositions["12"] = { railX[1].x, bottomY };
            highGearPositions["13"] = { railX[2].x, topY };
            highGearPositions["14"] = { railX[2].x, bottomY };
            highGearPositions["15"] = { railX[3].x, topY };
            highGearPositions["16"] = { railX[3].x, bottomY };
        }
    }
    // --- Layout 3: Reverse Bottom Layout ---
    else if (layoutType == 3)
    {
        if (!is16GearSet)
        {
            // 12-gear with reverse at bottom
            lowerGearPositions["1"] = { railX[1].x, topY };
            lowerGearPositions["2"] = { railX[1].x, bottomY };
            lowerGearPositions["3"] = { railX[2].x, topY };
            lowerGearPositions["4"] = { railX[2].x, bottomY };
            lowerGearPositions["5"] = { railX[3].x, topY };
            lowerGearPositions["6"] = { railX[3].x, bottomY };
            lowerGearPositions["R"] = { railX[0].x, bottomY }; // Reverse at bottom

            highGearPositions["7"] = { railX[1].x, topY };
            highGearPositions["8"] = { railX[1].x, bottomY };
            highGearPositions["9"] = { railX[2].x, topY };
            highGearPositions["10"] = { railX[2].x, bottomY };
            highGearPositions["11"] = { railX[3].x, topY };
            highGearPositions["12"] = { railX[3].x, bottomY };
            highGearPositions["R"] = { railX[0].x, bottomY }; // Reverse at bottom
        }
        else
        {
            // 16-gear with reverse at bottom
            lowerGearPositions["1"] = { railX[1].x, topY };
            lowerGearPositions["2"] = { railX[1].x, bottomY };
            lowerGearPositions["3"] = { railX[2].x, topY };
            lowerGearPositions["4"] = { railX[2].x, bottomY };
            lowerGearPositions["5"] = { railX[3].x, topY };
            lowerGearPositions["6"] = { railX[3].x, bottomY };
            lowerGearPositions["7"] = { railX[4].x, topY };
            lowerGearPositions["8"] = { railX[4].x, bottomY };
            lowerGearPositions["R"] = { railX[0].x, bottomY }; // Reverse at bottom

            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
            highGearPositions["13"] = { railX[3].x, topY };
            highGearPositions["14"] = { railX[3].x, bottomY };
            highGearPositions["15"] = { railX[4].x, topY };
            highGearPositions["16"] = { railX[4].x, bottomY };
            highGearPositions["R"] = { railX[0].x, bottomY }; // Reverse at bottom
        }
    }

    // --- Layout 4: Reverse Bottom Last Layout ---
    else if (layoutType == 4)
    {
        if (!is16GearSet)
        {
            // 12-gear with reverse at bottom on last rail
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["R"] = { railX[3].x, bottomY }; // Reverse at bottom on last rail

            highGearPositions["7"] = { railX[0].x, topY };
            highGearPositions["8"] = { railX[0].x, bottomY };
            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
            highGearPositions["R"] = { railX[3].x, bottomY }; // Reverse at bottom on last rail
        }
        else
        {
            // 16-gear with reverse at bottom on last rail
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["7"] = { railX[3].x, topY };
            lowerGearPositions["8"] = { railX[3].x, bottomY };
            lowerGearPositions["R"] = { railX[4].x, bottomY }; // Reverse at bottom on last rail

            highGearPositions["9"] = { railX[0].x, topY };
            highGearPositions["10"] = { railX[0].x, bottomY };
            highGearPositions["11"] = { railX[1].x, topY };
            highGearPositions["12"] = { railX[1].x, bottomY };
            highGearPositions["13"] = { railX[2].x, topY };
            highGearPositions["14"] = { railX[2].x, bottomY };
            highGearPositions["15"] = { railX[3].x, topY };
            highGearPositions["16"] = { railX[3].x, bottomY };
            highGearPositions["R"] = { railX[4].x, bottomY }; // Reverse at bottom on last rail
        }
    }
    // --- Layout 5: 5-Gear Only Layout ---
    else if (layoutType == 5)
    {
        // 5-gear layout only (no 16-gear support)
        // First rail: 1 top, 2 bottom
        lowerGearPositions["1"] = { railX[0].x, topY };
        lowerGearPositions["2"] = { railX[0].x, bottomY };

        // Second rail: 3 top, 4 bottom  
        lowerGearPositions["3"] = { railX[1].x, topY };
        lowerGearPositions["4"] = { railX[1].x, bottomY };

        // Third rail: 5 top, R bottom
        lowerGearPositions["5"] = { railX[2].x, topY };
        lowerGearPositions["R"] = { railX[2].x, bottomY };

        // High range uses same positions (no separate high range for 5-gear)
        highGearPositions["1"] = { railX[0].x, topY };
        highGearPositions["2"] = { railX[0].x, bottomY };
        highGearPositions["3"] = { railX[1].x, topY };
        highGearPositions["4"] = { railX[1].x, bottomY };
        highGearPositions["5"] = { railX[2].x, topY };
        highGearPositions["R"] = { railX[2].x, bottomY };
    }
    // --- Layout 6: 5-Gear Reverse First Layout ---
    else if (layoutType == 6)
    {
        // 5-gear with reverse on first rail top
        // First rail: R top, 1 bottom
        lowerGearPositions["R"] = { railX[0].x, topY };
        lowerGearPositions["1"] = { railX[0].x, bottomY };

        // Second rail: 2 top, 3 bottom  
        lowerGearPositions["2"] = { railX[1].x, topY };
        lowerGearPositions["3"] = { railX[1].x, bottomY };

        // Third rail: 4 top, 5 bottom
        lowerGearPositions["4"] = { railX[2].x, topY };
        lowerGearPositions["5"] = { railX[2].x, bottomY };

        // High range uses same positions
        highGearPositions["R"] = { railX[0].x, topY };
        highGearPositions["1"] = { railX[0].x, bottomY };
        highGearPositions["2"] = { railX[1].x, topY };
        highGearPositions["3"] = { railX[1].x, bottomY };
        highGearPositions["4"] = { railX[2].x, topY };
        highGearPositions["5"] = { railX[2].x, bottomY };
    }

    // --- Layout 7: 4-Gear Reverse Top Layout ---
    else if (layoutType == 7)
    {
        // 4-gear with reverse on first rail top
        // First rail: R top, nothing bottom
        lowerGearPositions["R"] = { railX[0].x, topY };

        // Second rail: 1 top, 2 bottom  
        lowerGearPositions["1"] = { railX[1].x, topY };
        lowerGearPositions["2"] = { railX[1].x, bottomY };

        // Third rail: 3 top, 4 bottom
        lowerGearPositions["3"] = { railX[2].x, topY };
        lowerGearPositions["4"] = { railX[2].x, bottomY };

        // High range uses same positions
        highGearPositions["R"] = { railX[0].x, topY };
        highGearPositions["1"] = { railX[1].x, topY };
        highGearPositions["2"] = { railX[1].x, bottomY };
        highGearPositions["3"] = { railX[2].x, topY };
        highGearPositions["4"] = { railX[2].x, bottomY };
    }

    // --- Layout 8: 4-Gear Reverse Bottom Layout ---
    else if (layoutType == 8)
    {
        // 4-gear with reverse on first rail bottom
        // First rail: nothing top, R bottom
        lowerGearPositions["R"] = { railX[0].x, bottomY };

        // Second rail: 1 top, 2 bottom  
        lowerGearPositions["1"] = { railX[1].x, topY };
        lowerGearPositions["2"] = { railX[1].x, bottomY };

        // Third rail: 3 top, 4 bottom
        lowerGearPositions["3"] = { railX[2].x, topY };
        lowerGearPositions["4"] = { railX[2].x, bottomY };

        // High range uses same positions
        highGearPositions["R"] = { railX[0].x, bottomY };
        highGearPositions["1"] = { railX[1].x, topY };
        highGearPositions["2"] = { railX[1].x, bottomY };
        highGearPositions["3"] = { railX[2].x, topY };
        highGearPositions["4"] = { railX[2].x, bottomY };
    }

    // --- Layout 9: 4-Gear Reverse Mixed Layout ---
    else if (layoutType == 9)
    {
        // 4-gear with reverse on first rail top, 4th gear on last rail top
        // First rail: R top, 1 bottom
        lowerGearPositions["R"] = { railX[0].x, topY };
        lowerGearPositions["1"] = { railX[0].x, bottomY };

        // Second rail: 2 top, 3 bottom  
        lowerGearPositions["2"] = { railX[1].x, topY };
        lowerGearPositions["3"] = { railX[1].x, bottomY };

        // Third rail: 4 top, nothing bottom
        lowerGearPositions["4"] = { railX[2].x, topY };

        // High range uses same positions
        highGearPositions["R"] = { railX[0].x, topY };
        highGearPositions["1"] = { railX[0].x, bottomY };
        highGearPositions["2"] = { railX[1].x, topY };
        highGearPositions["3"] = { railX[1].x, bottomY };
        highGearPositions["4"] = { railX[2].x, topY };
    }
    // --- Layout 10: Reverse Top Last Layout ---
    else if (layoutType == 10)
    {
        if (!is16GearSet)
        {
            // 12-gear with reverse at top on last rail
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["R"] = { railX[3].x, topY }; // Reverse at top on last rail

            highGearPositions["7"] = { railX[0].x, topY };
            highGearPositions["8"] = { railX[0].x, bottomY };
            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
            highGearPositions["R"] = { railX[3].x, topY }; // Reverse at top on last rail
        }
        else
        {
            // 16-gear with reverse at top on last rail
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["7"] = { railX[3].x, topY };
            lowerGearPositions["8"] = { railX[3].x, bottomY };
            lowerGearPositions["R"] = { railX[4].x, topY }; // Reverse at top on last rail

            highGearPositions["9"] = { railX[0].x, topY };
            highGearPositions["10"] = { railX[0].x, bottomY };
            highGearPositions["11"] = { railX[1].x, topY };
            highGearPositions["12"] = { railX[1].x, bottomY };
            highGearPositions["13"] = { railX[2].x, topY };
            highGearPositions["14"] = { railX[2].x, bottomY };
            highGearPositions["15"] = { railX[3].x, topY };
            highGearPositions["16"] = { railX[3].x, bottomY };
            highGearPositions["R"] = { railX[4].x, topY }; // Reverse at top on last rail
        }
    }
    // --- Layout 11: PRNDL Layout ---
    else if (layoutType == 11)
    {
        // PRNDL layout with single vertical rail
        // Using numbers 1-5 instead of PRNDL labels
        int railXPos = railX[0].x; // Only one rail

        // Evenly distribute positions along the vertical rail
        int totalPositions = 5;
        int segmentHeight = (bottomY - topY) / (totalPositions - 1);

        lowerGearPositions["1"] = { railXPos, topY };                    // P position
        lowerGearPositions["2"] = { railXPos, topY + segmentHeight };    // R position  
        lowerGearPositions["3"] = { railXPos, topY + segmentHeight * 2 };// N position
        lowerGearPositions["4"] = { railXPos, topY + segmentHeight * 3 };// D position
        lowerGearPositions["5"] = { railXPos, bottomY };                 // L position

        // High range uses same positions (no separate high range for PRNDL)
        highGearPositions["1"] = { railXPos, topY };
        highGearPositions["2"] = { railXPos, topY + segmentHeight };
        highGearPositions["3"] = { railXPos, topY + segmentHeight * 2 };
        highGearPositions["4"] = { railXPos, topY + segmentHeight * 3 };
        highGearPositions["5"] = { railXPos, bottomY };
    }
    // Set default knob position
    knobPos.x = railX[railCount / 2].x; // Center rail
    knobPos.y = centerY;
}
{
    int x, y;   // center of diamond
    int radius; // distance from center to diamond corners
};
std::vector<Intersection> intersections;
{
    intersections.clear();
    int railCount = is16GearSet ? 5 : 4;

    for (int i = 0; i < railCount; ++i)
    {
        Intersection inter;
        inter.x = railX[i].x;
        inter.y = centerY;
        inter.radius = int(baseIntersectionRadius * diagonalAssist); // scaled by diagonalAssist
        intersections.push_back(inter);
    }

    // Update vertical threshold too
    enterVerticalThreshold = int(baseEnterVerticalThreshold * diagonalAssist);
}

// --- Determine if inside diamond intersection with diagonal assist ---
{
    for (auto& inter : intersections)
    {
        int dx = x - inter.x;
        int dy = y - inter.y;

        // --- Detect if vertical rail (top/bottom gear exists) ---
        bool hasTop = false;
        bool hasBottom = false;
        for (auto& kv : lowerGearPositions)
        {
            if (kv.second.x == inter.x)
            {
                if (kv.second.y == topY) hasTop = true;
                if (kv.second.y == bottomY) hasBottom = true;
            }
        }

        double radiusX = inter.radius * diagonalAssist;

        // Asymmetric vertical radii with hard cutoff
        double radiusY_top = 0.0;
        double radiusY_bottom = 0.0;

        if (hasTop && hasBottom)
        {
            radiusY_top = radiusY_bottom = inter.radius * diagonalAssist * 1.5;
        }
        else if (hasTop)
        {
            radiusY_top = inter.radius * diagonalAssist * 1.5;
            radiusY_bottom = inter.radius * diagonalAssist * 0.4; // small buffer zone
        }
        else if (hasBottom)
        {
            radiusY_top = inter.radius * diagonalAssist * 0.4; // small buffer zone
            radiusY_bottom = inter.radius * diagonalAssist * 1.5;
        }

        else
        {
            radiusY_top = radiusY_bottom = 0.0; // no gear, no intersection
        }

        // If dy is in the “empty” direction, skip this intersection
        if ((y < inter.y && radiusY_top == 0.0) || (y > inter.y && radiusY_bottom == 0.0))
            continue;

        // Normalize dy asymmetrically
        double normalizedDY = (dy < 0) ? -dy / radiusY_top : dy / radiusY_bottom;
        double normalizedDX = dx / radiusX;

        double distance = sqrt(normalizedDX * normalizedDX + normalizedDY * normalizedDY);

        if (distance <= 1.0)
            return true;
    }

    return false;
}


RECT gearLayoutButtonRect;
bool gearLayoutDropdownOpen = false;

// Define the gear label map (can be updated later to override labels)
std::map<std::string, std::string> gearLabelOverride = {
    {"1", "1"}, {"2", "2"}, {"3", "3"}, {"4", "4"}, {"5", "5"}, {"6", "6"}, {"7", "7"}, {"8", "8"}, {"9", "9"}, {"10", "10"}, {"11", "11"}, {"12", "12"}, {"13", "13"}, {"14", "14"}, {"15", "15"}, {"16", "16"}, {"R", "R"} };

// Add at global scope or before showSettingsPanel block
std::vector<std::map<std::string, std::string>> gearLayouts = {
    {// default
     {"1", "1"},
     {"2", "2"},
     {"3", "3"},
     {"4", "4"},
     {"5", "5"},
     {"6", "6"},
     {"7", "7"},
     {"8", "8"},
     {"9", "9"},
     {"10", "10"},
     {"11", "11"},
     {"12", "12"},
     {"13", "13"},
     {"14", "14"},
     {"15", "15"},
     {"16", "16"},
     {"R", "R"}},
    {// Alphabet
     {"1", "A"},
     {"2", "B"},
     {"3", "C"},
     {"4", "D"},
     {"5", "E"},
     {"6", "F"},
     {"7", "G"},
     {"8", "H"},
     {"9", "I"},
     {"10", "J"},
     {"11", "K"},
     {"12", "L"},
     {"13", "M"},
     {"14", "N"},
     {"15", "O"},
     {"16", "P"},
     {"R", "R"}},
    {// Roman Numerals
     {"1", "I"},
     {"2", "II"},
     {"3", "III"},
     {"4", "IV"},
     {"5", "V"},
     {"6", "VI"},
     {"7", "VII"},
     {"8", "VIII"},
     {"9", "IX"},
     {"10", "X"},
     {"11", "XI"},
     {"12", "XII"},
     {"13", "XIII"},
     {"14", "XIV"},
     {"15", "XV"},
     {"16", "XVI"},
     {"R", "R"}} };

std::vector<std::wstring> gearLayoutNames = {
    L"Default",       // gearLayouts[0]
    L"Alphabet",      // gearLayouts[1]
    L"Roman Numerals" // gearLayouts[2]
};

int currentGearLayout = 0; // index of active layout
{
    gearLayouts.clear();
    gearLayoutNames.clear();

    std::wifstream file(filename);
    if (!file.is_open())
        return;

    std::wstring line;
    int layoutCount = 0;

    while (std::getline(file, line))
    {
        // Remove BOM if present
        if (!line.empty() && line[0] == 0xFEFF)
            line.erase(0, 1);

        // Trim whitespace
        line.erase(0, line.find_first_not_of(L" \t\r\n"));
        line.erase(line.find_last_not_of(L" \t\r\n") + 1);

        if (line.empty() || line[0] == L';' || line[0] == L'#')
            continue;

        if (line.find(L"count=") == 0)
        {
            layoutCount = std::stoi(line.substr(6));
        }
    }

    file.clear();
    file.seekg(0, std::ios::beg);

    for (int i = 0; i < layoutCount; ++i)
    {
        std::map<std::string, std::string> layout;
        std::wstring layoutName;
        std::wstring sectionHeader = L"[Layout" + std::to_wstring(i) + L"]";

        bool inSection = false;
        while (std::getline(file, line))
        {
            line.erase(0, line.find_first_not_of(L" \t\r\n"));
            line.erase(line.find_last_not_of(L" \t\r\n") + 1);

            if (line.empty() || line[0] == L';' || line[0] == L'#')
                continue;

            if (line == sectionHeader)
            {
                inSection = true;
                continue;
            }

            if (inSection)
            {
                if (line[0] == L'[')
                    break; // next section

                size_t eqPos = line.find(L'=');
                if (eqPos == std::wstring::npos)
                    continue;

                std::wstring key = line.substr(0, eqPos);
                std::wstring value = line.substr(eqPos + 1);

                key.erase(0, key.find_first_not_of(L" \t\r\n"));
                key.erase(key.find_last_not_of(L" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(L" \t\r\n"));
                value.erase(value.find_last_not_of(L" \t\r\n") + 1);

                if (key == L"name")
                    layoutName = value;
                else
                {
                    std::string keyStr(key.begin(), key.end());
                    std::string valueStr(value.begin(), value.end());
                    layout[keyStr] = valueStr;
                }
            }
        }

        if (!layout.empty())
        {
            gearLayouts.push_back(layout);
            gearLayoutNames.push_back(layoutName);
        }

        file.clear();
        file.seekg(0, std::ios::beg); // reset for next layout
    }
}
{
    char buf[64];

    // Booleans
    WritePrivateProfileStringA(configSection, "NeutralEnabled", isNeutralEnabled ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "DynamicTransparency", dynamicTransparencyEnabled ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "ShowYBar", showYBar ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "ShowXBar", showXBar ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "KnobAccelerationEnabled", knobAccelerationEnabled ? "1" : "0", configFile.c_str());

    // Floats/ints
    sprintf_s(buf, "%.2f", layoutScale);
    WritePrivateProfileStringA(configSection, "LayoutScale", buf, configFile.c_str());

    sprintf_s(buf, "%d", knobRadius);
    WritePrivateProfileStringA(configSection, "KnobRadius", buf, configFile.c_str());

    sprintf_s(buf, "%d", knobRadiusMin);
    WritePrivateProfileStringA(configSection, "KnobRadiusMin", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", knobSensitivity);
    WritePrivateProfileStringA(configSection, "KnobSensitivity", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", snapSpeed);
    WritePrivateProfileStringA(configSection, "SnapSpeed", buf, configFile.c_str());
    // Diagonal & snap-in multipliers
    sprintf_s(buf, "%.2f", diagonalAssist);
    WritePrivateProfileStringA(configSection, "DiagonalAssist", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", gearSnapInMultiplier);
    WritePrivateProfileStringA(configSection, "GearSnapInMultiplier", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", controllerSensMultiplier);
    WritePrivateProfileStringA(configSection, "ControllerSensMultiplier", buf, configFile.c_str());

    sprintf_s(buf, "%d", gearRadius);
    WritePrivateProfileStringA(configSection, "GearRadius", buf, configFile.c_str());
    sprintf_s(buf, "%u", maxAlpha);
    WritePrivateProfileStringA(configSection, "MaxAlpha", buf, configFile.c_str());
    sprintf_s(buf, "%u", minAlpha);
    WritePrivateProfileStringA(configSection, "MinAlpha", buf, configFile.c_str());
    sprintf_s(buf, "%u", transparencyFadeDelay);
    WritePrivateProfileStringA(configSection, "TransparencyFadeDelay", buf, configFile.c_str());
    sprintf_s(buf, "%d", g_knobToggleKey);
    WritePrivateProfileStringA(configSection, "KnobToggleKey", buf, configFile.c_str());
    sprintf_s(buf, "%d", (int)g_knobToggleType);
    WritePrivateProfileStringA(configSection, "KnobToggleType", buf, configFile.c_str());

    WritePrivateProfileStringA(configSection, "ReverseLockEnabled", reverseLockEnabled ? "1" : "0", configFile.c_str());
    sprintf_s(buf, "%d", g_reverseUnlockKey);
    WritePrivateProfileStringA(configSection, "ReverseUnlockKey", buf, configFile.c_str());
    sprintf_s(buf, "%d", (int)g_reverseUnlockType);
    WritePrivateProfileStringA(configSection, "ReverseUnlockType", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", steeringSensitivity);
    WritePrivateProfileStringA(configSection, "SteeringSensitivity", buf, configFile.c_str());
    WritePrivateProfileStringA(configSection, "RealisticKnob", realisticKnob ? "1" : "0", configFile.c_str());
    sprintf_s(buf, "%.2f", accBrakeSensitivity);
    WritePrivateProfileStringA(configSection, "AccBrakeSensitivity", buf, configFile.c_str());
    // XInput assist button
    // Save
    sprintf_s(buf, "%u", assistButton);
    WritePrivateProfileStringA(configSection, "AssistButton", buf, configFile.c_str());
    // Scroll Clutch Sensitivity
    sprintf_s(buf, "%.2f", scrollClutchSens);
    WritePrivateProfileStringA(configSection, "ScrollClutchSens", buf, configFile.c_str());

    // Invert Scroll → Clutch toggle
    WritePrivateProfileStringA(configSection, "InvertScrollClutch", invertScrollClutchAxis ? "1" : "0", configFile.c_str());
    // Smooth Scroll Speed
    sprintf_s(buf, "%.1f", smoothScrollSpeed);
    WritePrivateProfileStringA(configSection, "SmoothScrollSpeed", buf, configFile.c_str());

    // Brake Resistance Factor
    sprintf_s(buf, "%.1f", brakeresistanceFactor);
    WritePrivateProfileStringA(configSection, "BrakeResistanceFactor", buf, configFile.c_str());

    // Acceleration Resistance Factor  
    sprintf_s(buf, "%.1f", accelerationResistanceFactor);
    WritePrivateProfileStringA(configSection, "AccelerationResistanceFactor", buf, configFile.c_str());
    // Show Clutch Indicator toggle
    WritePrivateProfileStringA(configSection, "ShowClutchIndicator", showClutchIndicator ? "1" : "0", configFile.c_str());
    sprintf_s(buf, "%.0f", maxSteeringDegrees);
    WritePrivateProfileStringA(configSection, "MaxSteeringDegrees", buf, configFile.c_str());
    // Add these with the other boolean saves:
    WritePrivateProfileStringA(configSection, "UseAxisSmoothing", useAxisSmoothing ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "UseYbarFixedTransparency", useYbarFixedTransparency ? "1" : "0", configFile.c_str());

    // Add these with the other float/int saves:
    sprintf_s(buf, "%.2f", axisSmoothingFactor);
    WritePrivateProfileStringA(configSection, "AxisSmoothingFactor", buf, configFile.c_str());

    sprintf_s(buf, "%d", yBarAlpha); // This will save 0-255
    WritePrivateProfileStringA(configSection, "YBarAlpha", buf, configFile.c_str());
    // Keybindings (save both type and code)
    for (auto& kv : gearInputMap)
    {
        std::string typeKey = kv.first + "_Type";
        std::string codeKey = kv.first + "_Code";

        const char* typeStr = nullptr;
        switch (kv.second.type)
        {
        case KEYBOARD:
            typeStr = "KEYBOARD";
            break;
        case MOUSE:
            typeStr = "MOUSE";
            break;
        case VJOY_BUTTON:
            typeStr = "VJOY";
            break;
        }
        WritePrivateProfileStringA(configSection, typeKey.c_str(), typeStr, configFile.c_str());

        sprintf_s(buf, "%u", kv.second.code);
        WritePrivateProfileStringA(configSection, codeKey.c_str(), buf, configFile.c_str());
    }
    sprintf_s(buf, "%zu", inputMap.size());
    WritePrivateProfileStringA(configSection, "InputMapCount", buf, configFile.c_str());

    // Then, store each entry
    for (size_t i = 0; i < inputMap.size(); ++i)
    {
        std::string index = std::to_string(i);

        std::string typeKey = "InputMap_" + index + "_Type";
        std::string codeKey = "InputMap_" + index + "_Code";
        std::string vjoyKey = "InputMap_" + index + "_VJoyButton";

        const char* typeStr = nullptr;
        switch (inputMap[i].type)
        {
        case KEYBOARD:
            typeStr = "KEYBOARD";
            break;
        case MOUSE:
            typeStr = "MOUSE";
            break;
        case VJOY_BUTTON:
            typeStr = "VJOY";
            break;
        default:
            typeStr = "KEYBOARD";
            break;
        }
        WritePrivateProfileStringA(configSection, typeKey.c_str(), typeStr, configFile.c_str());

        sprintf_s(buf, "%u", inputMap[i].code);
        WritePrivateProfileStringA(configSection, codeKey.c_str(), buf, configFile.c_str());

        sprintf_s(buf, "%u", inputMap[i].vjoyButton);
        WritePrivateProfileStringA(configSection, vjoyKey.c_str(), buf, configFile.c_str());
    }
}
{
    char buf[64];

    // Booleans
    GetPrivateProfileStringA(configSection, "NeutralEnabled", "0", buf, sizeof(buf), configFile.c_str());
    isNeutralEnabled = (strcmp(buf, "1") == 0);
    GetPrivateProfileStringA(
        configSection,
        "KnobAccelerationEnabled",
        "0", // Default: acceleration disabled (Precision Movement enabled)
        buf,
        sizeof(buf),
        configFile.c_str());
    knobAccelerationEnabled = (strcmp(buf, "1") == 0);
    // Booleans
    GetPrivateProfileStringA(configSection, "ShowYBar", "0", buf, sizeof(buf), configFile.c_str());
    showYBar = (strcmp(buf, "1") == 0);

    GetPrivateProfileStringA(configSection, "ShowXBar", "0", buf, sizeof(buf), configFile.c_str());
    showXBar = (strcmp(buf, "1") == 0);
    // Dynamic Transparency
    GetPrivateProfileStringA(
        configSection,
        "DynamicTransparency",
        "0", // default disabled
        buf,
        sizeof(buf),
        configFile.c_str());
    dynamicTransparencyEnabled = (strcmp(buf, "1") == 0);

    // Floats/ints
    GetPrivateProfileStringA(configSection, "LayoutScale", "1.75", buf, sizeof(buf), configFile.c_str());
    layoutScale = (float)atof(buf);

    GetPrivateProfileStringA(configSection, "KnobRadius", "85", buf, sizeof(buf), configFile.c_str());
    knobRadius = atoi(buf);

    GetPrivateProfileStringA(configSection, "KnobRadiusMin", "20", buf, sizeof(buf), configFile.c_str());
    knobRadiusMin = atoi(buf);

    GetPrivateProfileStringA(configSection, "KnobSensitivity", "0.45", buf, sizeof(buf), configFile.c_str());
    knobSensitivity = (float)atof(buf);
    // Load multiplier from config
    GetPrivateProfileStringA(configSection, "ControllerSensMultiplier", "1.0", buf, sizeof(buf), configFile.c_str());
    controllerSensMultiplier = (float)atof(buf);
    controllerSensMultiplier = max(0.1f, min(2.0f, controllerSensMultiplier)); // clamp

    // Update slider position to match loaded multiplier
    controllerSensSliderValue = (controllerSensMultiplier - 0.1f) / 2.0f; // inverse of mapping: 0.1-2.1 → 0-1

    GetPrivateProfileStringA(configSection, "SnapSpeed", "0.75", buf, sizeof(buf), configFile.c_str());
    snapSpeed = (float)atof(buf);
    GetPrivateProfileStringA(configSection, "DiagonalAssist", "1.70", buf, sizeof(buf), configFile.c_str());
    diagonalAssist = (float)atof(buf);
    diagonalAssist = max(diagMin, min(diagMax, diagonalAssist)); // clamp to [0.5, 4.0]

    // Recompute dependent values
    enterVerticalThreshold = int(baseEnterVerticalThreshold * diagonalAssist);
    for (auto& inter : intersections)
    {
        inter.radius = int(baseIntersectionRadius * diagonalAssist);
    }

    GetPrivateProfileStringA(configSection, "GearSnapInMultiplier", "1.70", buf, sizeof(buf), configFile.c_str());
    gearSnapInMultiplier = (float)atof(buf);
    GetPrivateProfileStringA(configSection, "GearRadius", "30", buf, sizeof(buf), configFile.c_str());
    gearRadius = atoi(buf);
    gearSnapInThreshold = int(gearRadius * gearSnapInMultiplier);
    GetPrivateProfileStringA(configSection, "MinAlpha", "20", buf, sizeof(buf), configFile.c_str());
    minAlpha = (BYTE)atoi(buf);
    minAlpha = max(0, min(255, minAlpha)); // clamp to valid range
    GetPrivateProfileStringA(configSection, "MaxAlpha", "150", buf, sizeof(buf), configFile.c_str());
    maxAlpha = (BYTE)atoi(buf);
    maxAlpha = max(minAlpha, min(255, maxAlpha)); // clamp to valid range
    GetPrivateProfileStringA(configSection, "TransparencyFadeDelay", "300", buf, sizeof(buf), configFile.c_str());
    transparencyFadeDelay = (DWORD)atoi(buf);
    GetPrivateProfileStringA(configSection, "SteeringSensitivity", "1.0", buf, sizeof(buf), configFile.c_str());
    steeringSensitivity = (float)atof(buf);

    GetPrivateProfileStringA(configSection, "AccBrakeSensitivity", "1.0", buf, sizeof(buf), configFile.c_str());
    accBrakeSensitivity = (float)atof(buf);
    GetPrivateProfileStringA(configSection, "RealisticKnob", "0", buf, sizeof(buf), configFile.c_str());
    realisticKnob = (strcmp(buf, "1") == 0);
    // Inside LoadConfig()
    GetPrivateProfileStringA(configSection, "KnobToggleKey", "16", buf, sizeof(buf), configFile.c_str()); // 16 = VK_SHIFT
    g_knobToggleKey = atoi(buf);
    GetPrivateProfileStringA(configSection, "KnobToggleType", "0", buf, sizeof(buf), configFile.c_str());
    g_knobToggleType = (ToggleType)atoi(buf);

    GetPrivateProfileStringA(configSection, "ReverseLockEnabled", "1", buf, sizeof(buf), configFile.c_str());
    reverseLockEnabled = (strcmp(buf, "1") == 0);
    GetPrivateProfileStringA(configSection, "ReverseUnlockKey", "16", buf, sizeof(buf), configFile.c_str()); // 16 = VK_SHIFT
    g_reverseUnlockKey = atoi(buf);
    GetPrivateProfileStringA(configSection, "ReverseUnlockType", "0", buf, sizeof(buf), configFile.c_str());
    g_reverseUnlockType = (ToggleType)atoi(buf);

    // Scroll Clutch Sensitivity
    GetPrivateProfileStringA(configSection, "ScrollClutchSens", "5.0", buf, sizeof(buf), configFile.c_str());
    scrollClutchSens = (float)atof(buf);
    scrollClutchSens = max(0.0f, min(10.0f, scrollClutchSens)); // clamp 0–10
    // Smooth Scroll Speed (range 1.0-20.0)
    GetPrivateProfileStringA(configSection, "SmoothScrollSpeed", "3.0", buf, sizeof(buf), configFile.c_str());
    smoothScrollSpeed = (float)atof(buf);
    smoothScrollSpeed = max(1.0f, min(20.0f, smoothScrollSpeed)); // clamp to valid range

    // Brake Resistance Factor (range 0-50)
    GetPrivateProfileStringA(configSection, "BrakeResistanceFactor", "15.0", buf, sizeof(buf), configFile.c_str());
    brakeresistanceFactor = (float)atof(buf);
    brakeresistanceFactor = max(0.0f, min(50.0f, brakeresistanceFactor)); // clamp to valid range

    // Acceleration Resistance Factor (range 0-50)
    GetPrivateProfileStringA(configSection, "AccelerationResistanceFactor", "5.0", buf, sizeof(buf), configFile.c_str());
    accelerationResistanceFactor = (float)atof(buf);
    accelerationResistanceFactor = max(0.0f, min(50.0f, accelerationResistanceFactor)); // clamp to valid range
    // Invert Scroll → Clutch toggle
    GetPrivateProfileStringA(configSection, "InvertScrollClutch", "0", buf, sizeof(buf), configFile.c_str());
    invertScrollClutchAxis = (strcmp(buf, "1") == 0);

    GetPrivateProfileStringA(configSection, "MaxSteeringDegrees", "900", buf, sizeof(buf), configFile.c_str());
    maxSteeringDegrees = (float)atof(buf);
    maxSteeringDegrees = max(90.0f, min(900.0f, maxSteeringDegrees)); // clamp 90–900
    // Add these with the other boolean loads:
    GetPrivateProfileStringA(configSection, "UseAxisSmoothing", "0", buf, sizeof(buf), configFile.c_str());
    useAxisSmoothing = (strcmp(buf, "1") == 0);

    GetPrivateProfileStringA(configSection, "UseYbarFixedTransparency", "0", buf, sizeof(buf), configFile.c_str());
    useYbarFixedTransparency = (strcmp(buf, "1") == 0);

    // Add these with the other float/int loads:
    GetPrivateProfileStringA(configSection, "AxisSmoothingFactor", "1.0", buf, sizeof(buf), configFile.c_str());
    axisSmoothingFactor = (float)atof(buf);
    axisSmoothingFactor = max(0.0f, min(2.0f, axisSmoothingFactor)); // clamp if needed

    GetPrivateProfileStringA(configSection, "YBarAlpha", "180", buf, sizeof(buf), configFile.c_str()); // Default to 180 (about 70%)
    yBarAlpha = atoi(buf);
    yBarAlpha = max(0, min(255, yBarAlpha)); // clamp to 0-255 range
    // Show Clutch Indicator toggle
    GetPrivateProfileStringA(configSection, "ShowClutchIndicator", "1", buf, sizeof(buf), configFile.c_str());
    showClutchIndicator = (strcmp(buf, "1") == 0);
    // Load
    GetPrivateProfileStringA(
        configSection,
        "AssistButton",
        "10", // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER = 10
        buf,
        sizeof(buf),
        configFile.c_str());

    assistButton = static_cast<SDL_GameControllerButton>(atoi(buf));



    // --- Load keybindings ---
    for (auto& kv : gearInputMap)
    {
        std::string typeKey = kv.first + "_Type";
        std::string codeKey = kv.first + "_Code";

        // type
        // type
        const char* defaultTypeStr = nullptr;
        switch (kv.second.type)
        {
        case KEYBOARD:
            defaultTypeStr = "KEYBOARD";
            break;
        case MOUSE:
            defaultTypeStr = "MOUSE";
            break;
        case VJOY_BUTTON:
            defaultTypeStr = "VJOY";
            break;
        default:
            defaultTypeStr = "KEYBOARD";
            break;
        }

        DWORD charsRead = GetPrivateProfileStringA(
            configSection, typeKey.c_str(), defaultTypeStr, buf, sizeof(buf), configFile.c_str());

        // Parse type string
        if (strcmp(buf, "KEYBOARD") == 0)
            kv.second.type = KEYBOARD;
        else if (strcmp(buf, "MOUSE") == 0)
            kv.second.type = MOUSE;
        else if (strcmp(buf, "VJOY") == 0)
            kv.second.type = VJOY_BUTTON;
        else
            kv.second.type = KEYBOARD; // fallback

        // code
        char defaultCode[16];
        sprintf_s(defaultCode, "%u", kv.second.code); // use current value as default
        charsRead = GetPrivateProfileStringA(
            configSection, codeKey.c_str(), defaultCode, buf, sizeof(buf), configFile.c_str());
        kv.second.code = (WORD)atoi(buf);

        std::cout << kv.first << " loaded: type=" << (kv.second.type == KEYBOARD ? "KEYBOARD" : "MOUSE")
            << " code=" << kv.second.code
            << " (charsRead=" << charsRead << ")" << std::endl;
    }

    // Clear inputMap before loading
    inputMap.clear();

    // Read the count of entries
    GetPrivateProfileStringA(configSection, "InputMapCount", "0", buf, sizeof(buf), configFile.c_str());
    size_t count = (size_t)atoi(buf);

    for (size_t i = 0; i < count; ++i)
    {
        std::string index = std::to_string(i);

        std::string typeKey = "InputMap_" + index + "_Type";
        std::string codeKey = "InputMap_" + index + "_Code";
        std::string vjoyKey = "InputMap_" + index + "_VJoyButton";

        InputToVJoy entry;
        entry.type = KEYBOARD;
        entry.code = 0;
        entry.vjoyButton = 0;

        GetPrivateProfileStringA(configSection, typeKey.c_str(), "KEYBOARD", buf, sizeof(buf), configFile.c_str());
        if (strcmp(buf, "KEYBOARD") == 0)
            entry.type = KEYBOARD;
        else if (strcmp(buf, "MOUSE") == 0)
            entry.type = MOUSE;
        else if (strcmp(buf, "VJOY") == 0)
            entry.type = VJOY_BUTTON;

        GetPrivateProfileStringA(configSection, codeKey.c_str(), "0", buf, sizeof(buf), configFile.c_str());
        entry.code = (WORD)atoi(buf);

        GetPrivateProfileStringA(configSection, vjoyKey.c_str(), "0", buf, sizeof(buf), configFile.c_str());
        entry.vjoyButton = (UINT)atoi(buf);

        inputMap.push_back(entry);
    }

    // If inputMap is empty, or missing entries, fill in fallback defaults
    if (inputMap.empty())
    {
        inputMap.push_back({ MOUSE, 1, 11 });
        inputMap.push_back({ KEYBOARD, 191, 12 });
    }
}


void RefreshProfilesList();
// Forward declaration
bool HasVisibleWindow(DWORD processId);
bool IsGameProcess(DWORD processId, const std::wstring& exeName);

// DLL Injection functions
{
    g_processList.clear();

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe))
    {
        do {
            // Check if process has a visible window AND is a game (not browser/system app)
            if (HasVisibleWindow(pe.th32ProcessID) && IsGameProcess(pe.th32ProcessID, pe.szExeFile))
            {
                g_processList.push_back(pe.th32ProcessID);
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);

    // Reverse the list to invert the cycling order
}
{
    // Convert to lowercase for case-insensitive comparison
    std::wstring lowerExe = exeName;
    std::transform(lowerExe.begin(), lowerExe.end(), lowerExe.begin(), ::towlower);

    // Exclude browsers
    static const std::vector<std::wstring> excludedBrowsers = {
        L"msedge.exe", L"chrome.exe", L"firefox.exe", L"opera.exe",
        L"safari.exe", L"brave.exe", L"vivaldi.exe", L"browser.exe"
    };

    for (const auto& browser : excludedBrowsers) {
        if (lowerExe == browser) {
            return false;
        }
    }

    // Exclude development tools and IDEs
    static const std::vector<std::wstring> excludedDevTools = {
        L"devenv.exe",          // Visual Studio
        L"vcsexpress.exe",      // Visual Studio Express
        L"wdexpress.exe",       // Visual Studio WD Express
        L"code.exe",            // VS Code
        L"vscode.exe",          // VS Code (alternative)
        L"notepad++.exe",       // Notepad++
        L"sublime_text.exe",    // Sublime Text
        L"atom.exe",            // Atom
        L"pycharm.exe",         // PyCharm
        L"idea.exe",            // IntelliJ IDEA
        L"clion.exe",           // CLion
        L"rider.exe",           // Rider
        L"webstorm.exe",        // WebStorm
        L"eclipse.exe",         // Eclipse
        L"netbeans.exe",        // NetBeans
        L"androidstudio.exe",   // Android Studio
        L"codeblocks.exe",      // Code::Blocks
        L"dev-cpp.exe",         // Dev-C++
    };

    for (const auto& devTool : excludedDevTools) {
        if (lowerExe == devTool) {
            return false;
        }
    }

    // Exclude system applications and utilities
// Exclude system applications and utilities
    static const std::vector<std::wstring> excludedSystemApps = {
        L"explorer.exe", L"taskmgr.exe", L"cmd.exe", L"powershell.exe",
        L"notepad.exe", L"calc.exe", L"mspaint.exe", L"winword.exe",
        L"excel.exe", L"powerpnt.exe", L"outlook.exe", L"teams.exe",
        L"discord.exe", L"spotify.exe", L"vlc.exe", L"photoshop.exe",
        L"MouseShifter.exe",    // Exclude your own app
        L"obs64.exe", L"obs.exe", // OBS Studio
        L"fraps.exe",           // Fraps
        L"msiafterburner.exe",  // MSI Afterburner
        L"rtss.exe",            // RivaTuner Statistics Server
        L"nvidia container.exe", // NVIDIA services
        L"geforce experience.exe", // GeForce Experience
        L"epicgameslauncher.exe", // Epic Games Launcher
        L"steam.exe",           // Steam client
        L"battle.net.exe",      // Battle.net
        L"ubisoftconnect.exe",  // Ubisoft Connect
        L"eadesktop.exe",       // EA Desktop
        L"galaxyclient.exe",    // GOG Galaxy
        L"origin.exe",          // Origin
        L"bethesdanetlauncher.exe", // Bethesda Launcher
        L"amazon games.exe",    // Amazon Games
        L"xboxapp.exe",         // Xbox App
        L"xboxpcapp.exe",       // Xbox PC App
        L"nahimic3.exe",         // Nahimic audio service
        L"amtrucks.exe",
        L"eurotrucks2.exe",
        L"ds4windows.exe",
    };
    for (const auto& app : excludedSystemApps) {
        if (lowerExe == app) {
            return false;
        }
    }

    // Exclude Windows store/apps and common non-game applications
    static const std::vector<std::wstring> excludedWindowsApps = {
        L"applicationframehost.exe", L"runtimebroker.exe", L"searchui.exe",
        L"startmenuexperiencehost.exe", L"widgets.exe", L"textinputhost.exe",
        L"lockapp.exe", L"shellexperiencehost.exe",
        L"yourphone.exe", L"phoneexperiencehost.exe",
        L"gamebar.exe", L"gamebarftserver.exe", // Xbox Game Bar
        L"gamingoverlay.exe", L"gamingservices.exe",
    };

    for (const auto& winApp : excludedWindowsApps) {
        if (lowerExe == winApp) {
            return false;
        }
    }

    // Additional check: Look for common game indicators in window titles
    struct WindowData {
        DWORD targetProcessId;
        bool isGame;
    };

    WindowData data = { processId, true }; // Assume it's a game unless proven otherwise

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        WindowData* data = reinterpret_cast<WindowData*>(lParam);

        DWORD windowProcessId;
        GetWindowThreadProcessId(hwnd, &windowProcessId);

        if (windowProcessId == data->targetProcessId && IsWindowVisible(hwnd)) {
            wchar_t title[256];
            if (GetWindowTextW(hwnd, title, 256) > 0) {
                std::wstring windowTitle = title;
                std::transform(windowTitle.begin(), windowTitle.end(), windowTitle.begin(), ::towlower);

                // Exclude windows with development/browser-like titles
                static const std::vector<std::wstring> excludedTitles = {
                    L"microsoft edge", L"google chrome", L"firefox", L"opera",
                    L"browser", L"download", L"new tab", L"settings",
                    L"mouseshifter",   // Exclude your own app window
                    L"visual studio",  // Visual Studio
                    L"vscode",         // VS Code
                    L"notepad++",      // Notepad++
                    L"sublime text",   // Sublime Text
                    L"pycharm",        // PyCharm
                    L"intellij",       // IntelliJ
                    L"eclipse",        // Eclipse
                    L"android studio", // Android Studio
                    L"steam",          // Steam client
                    L"epic games",     // Epic Games Launcher
                    L"battle.net",     // Battle.net
                    L"discord",        // Discord
                    L"spotify",        // Spotify
                    L"obs studio",     // OBS
                    L"msi afterburner", // MSI Afterburner
                    L"xbox game bar",  // Xbox Game Bar
                };

                for (const auto& excluded : excludedTitles) {
                    if (windowTitle.find(excluded) != std::wstring::npos) {
                        data->isGame = false;
                        return FALSE; // Found excluded title, stop checking
                    }
                }
            }
        }
        return TRUE; // Continue enumeration
        }, reinterpret_cast<LPARAM>(&data));

    return data.isGame;
}
{
    struct WindowData {
        DWORD targetProcessId;
        bool hasVisibleWindow;
    };

    WindowData data = { processId, false };

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        WindowData* data = reinterpret_cast<WindowData*>(lParam);

        DWORD windowProcessId;
        GetWindowThreadProcessId(hwnd, &windowProcessId);

        if (windowProcessId == data->targetProcessId && IsWindowVisible(hwnd)) {
            // Less restrictive checks - focus on what makes a window "visible" to users
            LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

            // Skip tool windows and some system windows
            if (exStyle & WS_EX_TOOLWINDOW) {
                return TRUE; // Continue looking for other windows
            }

            // Get window title to filter out system windows
            wchar_t title[256];
            int titleLength = GetWindowTextW(hwnd, title, 256);

            // Skip windows with empty titles or known system windows
            if (titleLength > 0) {
                std::wstring windowTitle = title;

                // Common windows to exclude
                static const std::vector<std::wstring> excludedTitles = {
                    L"Default IME",
                    L"MSCTFIME UI",
                    L"",
                    L" "
                };

                bool shouldExclude = false;
                for (const auto& excluded : excludedTitles) {
                    if (windowTitle == excluded) {
                        shouldExclude = true;
                        break;
                    }
                }

                if (!shouldExclude) {
                    // Additional check: window should have some size
                    RECT rect;
                    if (GetWindowRect(hwnd, &rect)) {
                        int width = rect.right - rect.left;
                        int height = rect.bottom - rect.top;

                        // Consider windows with reasonable size as "visible"
                        if (width > 50 && height > 30) {
                            data->hasVisibleWindow = true;
                            // Don't return FALSE here - check all windows
                        }
                    }
                }
            }
        }
        return TRUE; // Always continue enumeration
        }, reinterpret_cast<LPARAM>(&data));

    return data.hasVisibleWindow;
}
// Global variables for both DLLs
bool g_mouseBlockEnabled = false;
bool g_xinputBlockEnabled = false;
DWORD g_lastInjectedMouseProcessId = 0;
DWORD g_lastInjectedXinputProcessId = 0;

// Track XInput blocking state based on controller input
bool g_xinputBlockTemporarilyDisabled = false;
bool g_lastAssistButtonState = false;
{
    if (processId == 0) return false;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) return false;

    // Allocate memory in target process
    size_t pathSize = (wcslen(dllPath) + 1) * sizeof(wchar_t);
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMemory) {
        CloseHandle(hProcess);
        return false;
    }

    // Write DLL path
    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath, pathSize, NULL)) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Create remote thread to load DLL
    LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(
        GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if (!pLoadLibrary) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibrary, pRemoteMemory, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return true;
}
{
    if (processId == 0) return false;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    MODULEENTRY32 me;
    me.dwSize = sizeof(MODULEENTRY32);

    bool found = false;
    HMODULE hModule = NULL;

    if (Module32First(hSnapshot, &me)) {
        do {
            if (wcscmp(me.szModule, dllName) == 0) {
                found = true;
                hModule = me.hModule;
                break;
            }
        } while (Module32Next(hSnapshot, &me));
    }

    CloseHandle(hSnapshot);

    if (!found) return false;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) return false;

    LPTHREAD_START_ROUTINE pFreeLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(
        GetModuleHandleW(L"kernel32.dll"), "FreeLibrary");
    if (!pFreeLibrary) {
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pFreeLibrary, hModule, 0, NULL);
    if (!hThread) {
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return true;
}

// Check controller state for assist button
{
    XINPUT_STATE state;
    if (XInputGetState(0, &state) == ERROR_SUCCESS) {
        bool assistButtonHeld = (state.Gamepad.wButtons & assistButton) != 0;
        return assistButtonHeld;
    }
    return false;
}

// Auto injection logic for both DLLs with controller-aware XInput blocking
// Auto injection logic for both DLLs with controller-aware XInput blocking
{
    static bool lastMouseState = false;
    static bool lastXinputState = false;
    static bool lastKnobState = false;
    static DWORD lastCheckedProcessId = 0;

    if (g_selectedProcessId != 0) {
        // Check if the selected process is still running
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, g_selectedProcessId);
        bool processRunning = (hProcess != NULL);
        if (hProcess) CloseHandle(hProcess);

        if (!processRunning) {
            // Process is no longer running, reset
            g_lastInjectedMouseProcessId = 0;
            g_lastInjectedXinputProcessId = 0;
            lastCheckedProcessId = 0;
            return;
        }

        // ============================================================================
        // MOUSE BLOCKING LOGIC (Original knobMovementEnabled logic)
        // ============================================================================
        if (g_mouseBlockEnabled && knobMovementEnabled && g_lastInjectedMouseProcessId != g_selectedProcessId) {
            // Mouse blocking enabled + knob enabled and not injected yet - inject DLL immediately
            wchar_t dllPath[MAX_PATH];
            GetModuleFileNameW(NULL, dllPath, MAX_PATH);
            wchar_t* lastBackslash = wcsrchr(dllPath, L'\\');
            if (lastBackslash) {
                wcscpy_s(lastBackslash + 1, MAX_PATH - (lastBackslash - dllPath + 1), L"RawMouseInput.dll");
                if (InjectDLL(g_selectedProcessId, dllPath)) {
                    g_lastInjectedMouseProcessId = g_selectedProcessId;
                    OutputDebugString(L"[AutoInject] RawMouseInput.dll injected successfully\n");
                }
            }
        }
        else if ((!g_mouseBlockEnabled || !knobMovementEnabled) && g_lastInjectedMouseProcessId == g_selectedProcessId) {
            // Mouse blocking disabled OR knob disabled and currently injected - uninject DLL immediately
            if (UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll")) {
                OutputDebugString(L"[AutoInject] RawMouseInput.dll uninjected successfully\n");
                g_lastInjectedMouseProcessId = 0;
            }
        }

        // ============================================================================
        // XINPUT BLOCKING LOGIC (New controller-aware logic)
        // ============================================================================
        bool assistButtonHeld = IsAssistButtonHeld();
        bool shouldBlockXInput = g_xinputBlockEnabled;

        // Apply disableRealKnobMovement logic
        if (disableRealKnobMovement) {
            // DEFAULT: UNINJECTED (right stick ALLOWED)
            if (assistButtonHeld) {
                // Assist button held - INJECT DLL (block right stick)
                shouldBlockXInput = true;
            }
            else {
                // Assist button NOT held - UNINJECT DLL (allow right stick)
                shouldBlockXInput = false;
            }
        }

        // Apply invertAssistAxes logic  
        if (invertAssistAxes) {
            // DEFAULT: INJECTED (right stick BLOCKED)
            if (assistButtonHeld) {
                // Assist button held - UNINJECT DLL (allow right stick)
                shouldBlockXInput = false;
            }
            else {
                // Assist button NOT held - INJECT DLL (block right stick)
                shouldBlockXInput = true;
            }
        }

        // Handle XInput DLL injection/uninjection based on calculated state
        if (shouldBlockXInput && g_lastInjectedXinputProcessId != g_selectedProcessId) {
            // Should block and not injected yet - inject DLL
            wchar_t dllPath[MAX_PATH];
            GetModuleFileNameW(NULL, dllPath, MAX_PATH);
            wchar_t* lastBackslash = wcsrchr(dllPath, L'\\');
            if (lastBackslash) {
                wcscpy_s(lastBackslash + 1, MAX_PATH - (lastBackslash - dllPath + 1), L"xInputBlocker.dll");
                if (InjectDLL(g_selectedProcessId, dllPath)) {
                    g_lastInjectedXinputProcessId = g_selectedProcessId;
                    OutputDebugString(L"[AutoInject] xInputBlocker.dll injected (blocking right stick)\n");
                }
            }
        }
        else if (!shouldBlockXInput && g_lastInjectedXinputProcessId == g_selectedProcessId) {
            // Should not block and currently injected - uninject DLL
            if (UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll")) {
                OutputDebugString(L"[AutoInject] xInputBlocker.dll uninjected (allowing right stick)\n");
                g_lastInjectedXinputProcessId = 0;
            }
        }

        // Log state changes for debugging
        if (assistButtonHeld != g_lastAssistButtonState) {
            g_lastAssistButtonState = assistButtonHeld;
            if (assistButtonHeld) {
                OutputDebugString(L"[AutoInject] Assist button pressed\n");
            }
            else {
                OutputDebugString(L"[AutoInject] Assist button released\n");
            }
        }

        lastKnobState = knobMovementEnabled;
    }
    else {
        // No process selected, uninject both DLLs if they're injected
        if (g_lastInjectedMouseProcessId != 0) {
            UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll");
            g_lastInjectedMouseProcessId = 0;
        }
        if (g_lastInjectedXinputProcessId != 0) {
            UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll");
            g_lastInjectedXinputProcessId = 0;
        }
    }

    lastMouseState = g_mouseBlockEnabled;
    lastXinputState = g_xinputBlockEnabled;
}
// Add with your other global variables
bool g_showTooltip = false;
RECT g_tooltipBounds = { 0, 0, 0, 0 };



int settingsScrollMax = 0; // Maximum scrollable offset

// --- Right Panel Scroll variables ---
float rightPanelScrollOffsetF = 0.0f;
int rightPanelScrollOffset = 0;
float rightPanelScrollTarget = 0.0f;
int rightPanelScrollMax = 0;
const float rightPanelScrollSpeed = 0.3f;
// X-bar tracking
static int lastJoyX = 0;
static bool lastShowXBar = false;
static bool lastMouseSteeringEnabled = false;
static POINT lastKnobPos = { 0, 0 };
static int lastDrawRailCount = 0;

std::vector<TooltipInfo> tooltips;
TooltipInfo* currentTooltip = nullptr;
{
    if (!tooltip.show) return;

    FontFamily fontFamily(L"Segoe UI");
    Font tooltipFont(&fontFamily, 12, FontStyleRegular, UnitPixel);
    SolidBrush tooltipBgBrush(Color(30, 30, 30));
    SolidBrush tooltipTextBrush(Color(255, 255, 255));
    Pen tooltipBorderPen(Color(0, 255, 136), 1);

    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);

    // Measure text with word wrapping to get proper height
    float maxWidth = 250.0f; // Maximum width for tooltip
    RectF textBounds;
    graphics.MeasureString(tooltip.text.c_str(), -1, &tooltipFont,
        RectF(0, 0, maxWidth - 20, 500), &format, &textBounds); // Account for padding

    // Auto-width: use text width + padding, but max 250px
    float tooltipWidth = min(textBounds.Width + 20, maxWidth);

    // Auto-height: use text height + padding
    float tooltipHeight = textBounds.Height + 16;

    // Center the tooltip horizontally within the setting panel
    float centerX = (settingsPanelRect.left + settingsPanelRect.right) / 2.0f;
    float tooltipLeft = centerX - (tooltipWidth / 2.0f);

    // Position tooltip below the setting - USE THE verticalOffset PARAMETER
    RectF tooltipRect(
        tooltipLeft,
        (REAL)(tooltip.bounds.bottom + verticalOffset + scrollOffset), // FIXED: Use verticalOffset instead of hardcoded 25
        tooltipWidth,
        tooltipHeight
    );

    // Draw tooltip
    graphics.FillRectangle(&tooltipBgBrush, tooltipRect);
    graphics.DrawRectangle(&tooltipBorderPen, tooltipRect);

    // Draw centered text with word wrapping
    graphics.DrawString(tooltip.text.c_str(), -1, &tooltipFont,
        RectF(tooltipRect.X + 8, tooltipRect.Y + 8,
            tooltipRect.Width - 16, tooltipRect.Height - 16),
        &format, &tooltipTextBrush);
}
// Add this with your other settings, probably near the top of your drawing function
// ---------------- Paint ----------------
