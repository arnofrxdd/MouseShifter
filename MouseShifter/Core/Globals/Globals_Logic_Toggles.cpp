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


#include "Core/System/Updater.h"