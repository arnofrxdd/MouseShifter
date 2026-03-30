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
    //{KEYBOARD, 'A', 1},       // Press 'A' â†’ vJoy button 1
    //{KEYBOARD, 'S', 2},       // Press 'S' â†’ vJoy button 2
    {MOUSE, 1, 11}, // Left mouse button â†’ vJoy button 3
    {MOUSE, 2, 12}  // Right mouse button â†’ vJoy button 4
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

// --- Modern UI State ---
RECT g_modalActionRect;
RECT g_modalCancelRect;
int hoveredCloneIndex = -1;
int hoveredDeleteIndex = -1;
std::string hoveredKeybindGear = "";
int hoveredInputIndex = -1;
int hoveredToggleIndex = -1;

// Add with other global variables
extern HWND g_yBarHwnd;
extern int yBarAlpha;

bool mouseDeviceDropdownOpen = false;
bool steeringDeviceDropdownOpen = false;
bool gamepadDropdownOpen = false;
bool processPickerModalOpen = false;
float processPickerScrollOffset = 0.0f;
float processPickerScrollTarget = 0.0f;
int hoveredProcessIndex = -1;
RECT processPickerBoxRect = { 0 };
