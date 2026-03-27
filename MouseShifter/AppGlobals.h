#pragma once
#include <windows.h>
#include <d2d1.h>
#include <map>
#include <math.h>
#include <string>
#include <tchar.h>
#include <tlhelp32.h>  // Add this for process/module snapshot functions
#pragma comment(lib, "kernel32.lib")        // Add this line
#include "UI/Transparency.h"
#include <string>
#include <cmath>
#include <windowsx.h>
#include "public.h"
#include "vjoyinterface.h"
#pragma comment(lib, "vJoyInterface.lib")
enum ToggleType
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
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")
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
int knobMinX, knobMaxX, knobMinY, knobMaxY;
WORD heldGearKey = 0;            // VK code of currently held key
bool knobMovementEnabled = true; // default = enabled
// Settings panel
bool mouseSteeringEnabled = false; // checkbox state
RECT mouseSteeringToggleRect;      // global rect for checkbox

#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;
#include <fstream>
#include <sstream>

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
#include <map>

struct KeybindAnimation {
    float glowAlpha = 0.0f;
    bool isActive = false;
    DWORD activationTime = 0;
    bool isHeld = false;  // New: track if key is currently held down
};

std::map<std::string, KeybindAnimation> keybindAnimations;
const float GLOW_FADE_IN_TIME = 200.0f;  // milliseconds - faster fade in
const float GLOW_FADE_OUT_TIME = 400.0f; // milliseconds - fade out when released
const float MAX_GLOW_ALPHA = 0.8f;
// keybinding panel

bool showKeybindPanel = true;                // toggleable with a hotkey later
std::string keybindBeingSet = "";            // which gear we’re rebinding right now
RECT keybindPanelRect = { 720, 0, 900, 6500 }; // panel on right side
#include <vector>                            // for std::vector
#include <algorithm>                         // for std::sort
#include <string>                            // for std::string
#pragma comment(lib, "gdiplus.lib")

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

float knobFlash = 0.0f;           // current flash intensity (0 = none, 1 = max)
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

#include <iostream>
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
enum RailType
{
    NONE = -1,
    HORIZONTAL,
    VERTICAL
};
RailType currentRail = HORIZONTAL;

int currentVerticalIndex = -1; // which vertical rail if on vertical
#include <map>
#include <Windows.h>
bool showVJoyPicker = false;
std::string vJoyPickerGear;
RECT vJoyPickerRect = { 0 };
int vJoyButtonCount = 0; // detected at startup
int GetVJoyButtonCount(UINT deviceId)
{
    if (!vJoyEnabled())
        return 0;
    return GetVJDButtonNumber(deviceId);
}
LONG joyX = 0;
LONG joyY = 0;
LONG joyRX = 0;                 // Right stick X (vJoy RX)
LONG joyRY = 0;                 // Right stick Y (vJoy RY)
extern float xInputSensitivity; // new sensitivity multiplier
// Declare and define global sensitivity
float xInputSensitivity = 1.0f; // adjust as needed
bool useAxisSmoothing = false; // Toggle for interdependent smoothing
float axisSmoothingFactor = 0.9f; // Single variable to control smoothing (0.0 = no effect, 1.0 = max effect)
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
float settingsScrollOffsetF = 0.0f; // smooth scroll offset (float)
int settingsScrollOffset = 0;       // current scroll offset (int)
bool knobDisabledByF9 = false; // true if F9 disabled knob
int hoveredProfileIndex = -1;  // Track which profile is hovered
float settingsScrollTarget = 0.0f; // target scroll
float settingsScrollSpeed = 0.2f;  // smoothing factor (0.0..1.0)
RECT GetScrolledRect(const RECT& r)
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

#include <thread>
#include <SDL.h>
#include <SDL_gamecontroller.h>
#include <SDL_haptic.h>
#include <vector>
#include <string>
#include <thread>
#include <cmath>
std::wstring ToWString(const char* cstr) {
    if (!cstr) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, cstr, -1, NULL, 0);
    std::wstring ws(size_needed - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, cstr, -1, &ws[0], size_needed);
    return ws;
}
struct Gamepad {
    SDL_GameController* controller = nullptr;
    SDL_Haptic* haptic = nullptr;
    int id = -1;
};

std::vector<Gamepad> g_gamepads;  // store all connected controllers
bool PlayStationMode = true;      // your flag
bool invertPlayStationYAxis = false;
void InitGamepads() {
    SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);

    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");

    int n = SDL_NumJoysticks();
    for (int i = 0; i < n; i++) {
        if (!SDL_IsGameController(i)) continue;

        SDL_GameController* gc = SDL_GameControllerOpen(i);
        if (!gc) continue;

        SDL_Joystick* joy = SDL_GameControllerGetJoystick(gc);
        SDL_Haptic* haptic = SDL_HapticOpenFromJoystick(joy);
        if (haptic && SDL_HapticRumbleInit(haptic) != 0) {
            SDL_HapticClose(haptic);
            haptic = nullptr;
        }

        g_gamepads.push_back({ gc, haptic, i });
    }
}
void RefreshGamepads()
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

#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
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

#include "AppGlobals.cpp"
