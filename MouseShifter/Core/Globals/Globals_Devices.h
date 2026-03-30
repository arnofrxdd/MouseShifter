#include <map>
#include <Windows.h>
bool showVJoyPicker = false;
bool showLayoutGallery = false;
std::vector<RECT> layoutGalleryItemRects;
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
ToggleType g_knobToggleType = TOGGLE_KEYBOARD; // tracks if itâ€™s a key or a pedal
bool togglePedalBeingSet = false; // true = waiting for pedal press

// NEW:

// DirectInput globals:
LPDIRECTINPUT8 g_pDI = nullptr;
LPDIRECTINPUTDEVICE8 g_pJoystick = nullptr;
DIJOYSTATE2 g_diState;
bool g_hasPedals = false;
