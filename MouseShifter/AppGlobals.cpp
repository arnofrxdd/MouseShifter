#include "AppGlobals.h"
// --- Enum callback: pick first valid gamepad ---
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
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
bool InitDirectInput(HWND hwnd)
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
bool SetupJoystick(HWND hwnd)
{
    if (!g_pJoystick)
        return false;

    g_pJoystick->SetDataFormat(&c_dfDIJoystick2);
    g_pJoystick->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

    g_hasPedals = true;
    return true;
}

// --- Poll device ---
void PollDirectInput()
{
    if (!g_pJoystick)
        return;

    if (FAILED(g_pJoystick->Poll()))
        g_pJoystick->Acquire();

    g_pJoystick->GetDeviceState(sizeof(DIJOYSTATE2), &g_diState);
}

bool IsReverseUnlockActive();

void ClampMovementForReverseLock(int& newY, int currentX)
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

void ClampHorizontalMovementForReverseLock(LONG& newX, int currentY)
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

bool IsKnobToggleActive()
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

bool IsReverseUnlockActive()
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


void DebugToggleState()
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


#include "Core/Updater.h"



// ETS2 AND ATS EDITOR
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h> // for FOLDERID_Documents

namespace fs = std::filesystem;

// Get Documents folder path
fs::path getDocumentsFolder()
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
#include "Core/FileBackup.cpp"

#include "Input/VJoySetup.cpp"
#include "Input/MouseInput.cpp"
bool noReverseLayout = false;
// Layout types - use one of these
// Define gear layout types
struct GearLayout {
    int id;
    std::wstring name;
};

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
#include "UI/LayoutComputations.cpp"

#include "Core/ConfigManager.cpp"
