#include "UI/Handlers/Transparency.h"
static std::string lastSetGear = "";

void SetGearKey(std::string gear)
{
    if (gear == lastSetGear) return; // Prevent re-triggering if already in this gear
    lastSetGear = gear;

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
        keybindAnimations[gear].glowAlpha = 0.0f; // Start at 0 for fade in
    }
    
    keybindAnimations[gear].isActive = true;  // Trigger fade-in
    keybindAnimations[gear].isHeld = true;    // Ensure it stays at max alpha
    keybindAnimations[gear].activationTime = GetTickCount();
}

// Call this on every WM_INPUT update
void ReleaseGearKey()
{
    lastSetGear = ""; // Reset change detection
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