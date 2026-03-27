bool InitVJoy()
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

enum InputType
{
    KEYBOARD,
    MOUSE,
    VJOY_BUTTON
};

struct GearInput
{
    InputType type;
    WORD code; // VK code for keyboard or RAW mouse button code
};
InputType heldGearType = KEYBOARD;
struct MouseDevice
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

std::string inputBeingSet = ""; // currently being bound (similar to keybindBeingSet)
int rowHeight = 28;             // row height in panel

// For drawing and picking vJoy
std::string vJoyPickerInput; // currently selected input for vJoy
struct InputToVJoy
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
std::string GetExeFolder()
{
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string exePath(path);
    size_t pos = exePath.find_last_of("\\/");
    return exePath.substr(0, pos); // folder path
}

std::string configFile = GetExeFolder() + "\\config.ini";
const char* configSection = "Config";

extern int yBarAlpha;
extern bool useYbarFixedTransparency;
extern HWND g_yBarHwnd;

RECT accBrakeSensSliderRect;
RECT steeringSensSliderRect;
RECT steeringDegreesSliderRect;
float maxSteeringDegrees = 900.0f;
float fullWheelDegrees = 900.0f;
bool assistButtonHeld = false;
bool draggingSteeringDegreesSlider = false;
bool draggingTransparencySlider = false;
bool draggingYBarAlphaSlider = false;
const float alphaDecay = 0.66f;
// Add with other global variables
extern HWND g_yBarHwnd;
extern int yBarAlpha;

#include "UI/Transparency.h"
void SetGearKey(std::string gear)
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
void ReleaseGearKey()
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
void InitRawInput(HWND hwnd)
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

