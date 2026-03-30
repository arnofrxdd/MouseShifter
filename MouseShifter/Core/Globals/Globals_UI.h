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
std::string keybindBeingSet = "";            // which gear weâ€™re rebinding right now
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
std::map<std::wstring, bool> g_collapsedSections; // Tracks expanded/collapsed state of headings

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
float scrollClutchSens = 5.0f;      // small multiplier, tweak 0â€“10
RECT scrollSensSliderRect;
bool draggingScrollSensSlider = false;
bool useScrollClutch = false; // set to false to disable scroll â†’ Rx
RECT useScrollClutchToggleRect;   // toggle rectangle for scrollâ†’Rx
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

// --- Visual Layout Gallery ---
// --- Visual Layout Gallery ---
extern bool showLayoutGallery;
extern std::vector<RECT> layoutGalleryItemRects;
extern RECT layoutGalleryModalRect;
extern float layoutGalleryScrollOffset;
extern float layoutGalleryScrollTarget;
// --- Top Navigation Bar (Pill) ---
extern RECT topNavBarRect;
extern RECT gearCountToggleRect;
extern RECT topProfileBtnRect;
extern RECT topNewProfileBtnRect;
extern RECT topGearLabelBtnRect;
extern RECT topLayoutBtnRect;

// --- Reset System State ---
extern bool showResetConfirmation;
extern RECT resetAllButtonRect;
extern std::string hoveredResetGear;
extern std::map<std::string, RECT> gearResetBtnRects; 
extern int hoveredInputResetIndex;
extern std::map<int, RECT> inputResetBtnRects;

RailType currentRail = HORIZONTAL;

int currentVerticalIndex = -1; // which vertical rail if on vertical