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